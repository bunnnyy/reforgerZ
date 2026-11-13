//! An opening's chain (docs/DESIGN.md, "Openings"): its steps on the errand engine
//! (Missions/Errands), handed straight to the player (RZ_Errands.Hand), so the tracker, the Quests
//! tab and the talk screen show them as they show any errand. With someone outside, each step
//! starts with a talk objective (RZ_OpeningTalkDef: "Talk to {giver}.", done only by talking to
//! them), and the step itself is handed in that conversation:
//! 1. close by, quietly: search a building near the contact (one of your background's kinds when
//!    one is near), done there;
//! 2. your background's own system (RZ_OpeningSteps.c), else water from the nearest well, or
//!    something medical for the contact's hand;
//! 3. walk to where your background belongs (it ends there; nobody to go back to).
//! Levels 2-3, and an opening with nobody outside, end at the nearest place with work instead (the
//! lead), with no Fame.
//! The steps pay nothing through the errand ledger: the chain pays its own Fame at the end
//! (RZ_Openings), and two step 2s their own small effect (RZ_OpeningTasks.Resolved). Server only.

//------------------------------------------------------------------------------------------------
//! A step of the chain: its definition is the errand's, its done tells the opening.
class RZ_OpeningStepDef : RZ_ErrandDef
{
	int m_iStep;
	int m_iTask;			// step 2's task (RZ_EOpeningTask), NONE for steps 1 and 3

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningStepDef(string key, string title, int step)
	{
		m_sKey = key;
		m_sTitle = title;
		m_iStep = step;
		m_sFamily = "opening";
		m_bHanded = true;
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		Over("Never mind. It'll keep.");
	}

	//------------------------------------------------------------------------------------------------
	//! Handed, not asked for: the one job tracked before you've asked anyone (RZ_ErrandDef.IsOpening).
	override bool IsOpening()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Step 2's own system says whether its clock runs (RZ_OpeningTasks.Counts).
	override bool StepCounts(RZ_Errands dealer, RZ_Errand e, IEntity me)
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return true;

		return openings.StepCounts(dealer, e, me);
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (openings)
			openings.OnStepDone(e, said);
	}
}

//------------------------------------------------------------------------------------------------
//! A talk objective: "Talk to {giver}." at the contact, tracked (the HUD and the map's diamond),
//! never done by being there: it ends when you open talk with them (RZ_Openings.TalkLines), and the
//! step it waits for is handed in that conversation. It lapses with the contact's 45 minutes.
class RZ_OpeningTalkDef : RZ_OpeningStepDef
{
	//------------------------------------------------------------------------------------------------
	void RZ_OpeningTalkDef(string key, string title, int step)
	{
		// Set here, whatever the base constructor did with the same arguments
		m_sKey = key;
		m_sTitle = title;
		m_iStep = step;
		m_sFamily = "opening";
		m_bHanded = true;
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		m_fLapseMinutes = 45;
		m_aWays.Clear();
		RZ_ErrandWay w = Way("talk");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 1;
		w.m_fRadius = 4;
		w.m_bEndsThere = true;
		w.m_sStep = "Talk to {giver}.";
	}

	//------------------------------------------------------------------------------------------------
	//! Being there never does it: only talking to them.
	override bool StepCounts(RZ_Errands dealer, RZ_Errand e, IEntity me)
	{
		return false;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningChain
{
	static const string KEY_SEARCH = "opening.search";
	static const string KEY_WALK = "opening.walk";
	static const string KEY_LEAD = "opening.lead";
	static const string KEY_TALK = "opening.talk";

	ref RZ_OpeningTalkDef m_TalkFirst;			// before the first step: whoever's outside
	ref RZ_OpeningTalkDef m_TalkNext;			// back to them: only when no next person could stand (hand-off)
	ref RZ_OpeningTalkDef m_TalkFind;			// handed on: the next person, at their place

	ref RZ_OpeningStepDef m_Search;
	ref RZ_OpeningStepDef m_SearchShort;		// a learning player's step 1 (level 1)
	ref RZ_OpeningStepDef m_Lead;				// levels 2-3: the nearest place with work, from whoever's outside
	ref RZ_OpeningStepDef m_LeadNote;			// the same with nobody outside: the scene's read says it
	ref array<ref RZ_OpeningStepDef> m_aWalk = {};	// by the ending faction (RZ_EFaction)
	protected ref map<int, ref RZ_OpeningStepDef> m_mTasks = new map<int, ref RZ_OpeningStepDef>();

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningChain()
	{
		m_TalkFirst = new RZ_OpeningTalkDef(KEY_TALK, "Outside the Door", 0);
		m_TalkNext = new RZ_OpeningTalkDef(KEY_TALK + ".next", "Back to {giver}", 0);
		m_TalkFind = new RZ_OpeningTalkDef(KEY_TALK + ".find", "Find {giver}", 0);
		m_TalkFind.m_aWays[0].m_sStep = "Find {giver} at {place}. Talk to them.";
		m_Search = Search(KEY_SEARCH, 20);
		m_SearchShort = Search(KEY_SEARCH + ".short", 10);

		RZ_ErrandWay w = Task(RZ_EOpeningTask.WATER, "opening.water", "Water", "do", RZ_EErrandPlace.SUBJECT, 15, 8,
			"Fill what you can carry at {place}.");
		w.Work(RZ_EWork.FILL, "Fill what you can carry", 8);
		w.Take("We're down to the last cup. There's {place}. Fill whatever you can carry.",
			"I haven't had a proper drink since Tuesday. It's not far to {place}. I'd go myself, but you're quicker.");
		w.Done("Clean enough. Boil it if you can. I can't, so I won't.",
			"Thank you. I mean it. I was starting to look at the rain barrel.");

		w = Task(RZ_EOpeningTask.CUT, "opening.hand", "The Cut", "give", RZ_EErrandPlace.HERE, 4, 6,
			"Dress the cut on {giver}'s hand: a bandage, or anything medical you carry.");
		Give(w, RZ_EErrandItem.MEDICAL);
		w.Take("I cut my hand on the shutter getting in. It's gone hot. You've got something in that bag, haven't you?",
			"Look at this. A nail, getting in. You were always better at this than me.");
		w.Done("That's better already. Or I'm telling myself it is. Either way.",
			"Neat. You've done that before. Thank you.");

		w = Task(RZ_EOpeningTask.FEVER, "opening.fever", "The Fever", "give", RZ_EErrandPlace.HERE, 6, 6,
			"Treat the fever of {giver}'s boy: a dressing, pills, anything medical.");
		Give(w, RZ_EErrandItem.MEDICAL);
		w.Take("My boy's burning. Since last night. Whatever's in your bag, he needs it more than the bag does.",
			"He's inside, on the kitchen floor where it's cool. Do what you used to do. I'll hold his hand.");
		w.Done("He's asleep. Properly asleep, not the other thing. You don't know what that sounds like.",
			"His forehead's wet. That's good, isn't it? Tell me that's good.");

		w = Task(RZ_EOpeningTask.BOILED, "opening.boiled", "Water for the Fever", "do", RZ_EErrandPlace.SUBJECT, 15, 8,
			"Fill what you can carry at {place}, to be boiled for the fever.");
		w.Work(RZ_EWork.FILL, "Fill what you can carry", 8);
		w.Take("No medicine left anywhere, then. Water's the next best thing for a fever. There's {place}. I'll get a fire going.",
			"If there's nothing in the bag, there's still water. Bring what you can from {place}. I'll boil it.");
		w.Done("It's on the fire. Cool cloths till then. You taught me that, once, in a waiting room.",
			"Enough for the night. One night at a time.");

		w = Task(RZ_EOpeningTask.PUMP, "opening.pump", "The Pump", "do", RZ_EErrandPlace.SUBJECT, 5, 15,
			"Fill a can at the pump at {place}.");
		w.Take("The van's dry and so's every tank on this street. There's still a pump at {place}, if nobody's had it. Fill a can there.",
			"Fuel. That's all I need, then I can stop walking everywhere. {place} had a pump, last I looked.");
		w.Done("Smell that. Proper fuel. I could cry, and I'm not going to.",
			"That'll turn her over, if anything will. You always said the old ones start for spite.");

		w = Task(RZ_EOpeningTask.DOWNWIND, "opening.downwind", "Downwind", "sneak", RZ_EErrandPlace.SUBJECT, 30, 25,
			"Watch {place} for half a minute without being seen.");
		w.m_bUnseen = true;
		w.m_bQuiet = true;
		w.Take("Something's been moving round {place} at dusk. Several of them, together. Go and see how many. Stay low, stay downwind.",
			"They go about together now, round {place}. Count them for me from somewhere they can't see you.");
		w.Done("Together, at dusk, the same way every time. Like they still have somewhere to be.",
			"You weren't seen. Good. Then we know where not to be tonight.");

		w = Task(RZ_EOpeningTask.KEYS, "opening.keys", "The Station Keys", "do", RZ_EErrandPlace.SUBJECT, 3, 25,
			"Let yourself into {place} with the key.");
		w.Take("Your lot left the station keys with me when they went. Here. There's {place}, locked tight. Nobody's been in.",
			"You'll know what these open. I never did. Try {place}: it's been locked since the second day.");
		w.Done("So that's what was in there. All this time, and the key on my nail.",
			"Lock it again behind you next time. It's the only door left on this street that does.");

		w = Task(RZ_EOpeningTask.TOLL, "opening.toll", "The Toll", "talk", RZ_EErrandPlace.SUBJECT, 10, 30,
			"Walk up to {place}, say your piece, and walk away again. No shooting.");
		w.Take("The red-hand lot have a toll up at {place}. Go and look at it for me. Talk, don't fight. You know how.",
			"I need to know what they're charging at {place}, and whether they shoot first. Walk up slow. Walk away slower.");
		w.Done("Both hands, and walking. That's how you know it went well.",
			"Good. Now I know what it costs to get anywhere. Everything, mostly.");

		w = Task(RZ_EOpeningTask.SAMPLE, "opening.sample", "A Sample", "do", RZ_EErrandPlace.SUBJECT, 2, 8,
			"Take a sample from {place}.");
		w.Take("There's {place}. The lab people pay for a piece of one. Gloves, if you've got them.",
			"One of the big ones went down by the road. The lab wants tissue. You'd know how to take it.");
		w.Done("Keep it cold if you can. Keep it away from me either way.",
			"That's a piece of the way out, they say. It doesn't look like much.");

		w = Task(RZ_EOpeningTask.CHANNEL, "opening.channel", "Open Channel", "do", RZ_EErrandPlace.SUBJECT, 1, 50,
			"Call on the open channel with your radio, and answer whoever's asking.");
		w.Take("Someone's been calling on the open channel since dawn. I can hear them, but I can't make the thing talk back. You can.",
			"There's a voice on the radio asking if anyone's there. Somebody should say yes.");
		w.Done("Somebody heard you. That's twice as many people as heard them yesterday.",
			"Good. I didn't like that voice on its own.");

		w = Task(RZ_EOpeningTask.SWAP, "opening.swap", "A Fair Swap", "give", RZ_EErrandPlace.SUBJECT, 5, 6,
			"Find {place} on the road and see what {place} will swap.");
		w.Take("{place} comes through about now. They swap. A tin for a word at the Haven, usually. Go and catch them.",
			"The people on the roads know things. {place} is about. Take them something, if you've got it.");
		w.Done("There. Now someone out there knows your face. That's worth more than the tin.",
			"See? Nobody gets anything for nothing, but nobody has to.");

		w = Task(RZ_EOpeningTask.GATE, "opening.gate", "Hold the Gate", "do", RZ_EErrandPlace.SUBJECT, 60, 15,
			"Hold {place} at {giver}'s against the infected for a minute.");
		w.Take("The gate's been banging all night. They'll come to it. Hold it a minute while I wedge the back.",
			"Stand at the gate. A minute, that's all. I'd do it myself, but my hands won't keep still.");
		w.Done("Still standing. You and the gate both.",
			"That's the back wedged. Thank you. I couldn't have done both.");

		w = Task(RZ_EOpeningTask.CLOCK, "opening.clock", "The Clock", "give", RZ_EErrandPlace.HERE, 6, 6,
			"Give {giver}'s friend the antidote before the bite takes.");
		Give(w, RZ_EErrandItem.ANTIDOTE);
		w.Take("He's bitten. Forearm, an hour ago. You found an antidote, didn't you? Then we've got minutes, not hours.",
			"It's a bite, and it's going grey round the edge. You know what that means better than I do.");
		w.Done("His colour's coming back. Is that it? Is that all it takes?",
			"You did it at a run and your hands didn't shake. I'll remember that.");

		w = Task(RZ_EOpeningTask.CLOCK_MED, "opening.clock_med", "The Clock", "give", RZ_EErrandPlace.HERE, 6, 6,
			"Keep {giver}'s bitten friend going: a dressing, anything medical you carry.");
		Give(w, RZ_EErrandItem.MEDICAL);
		w.Take("He's bitten. No antidote anywhere. Can you keep him going till someone has one?",
			"A bite on the forearm. I know there's no fixing it with what we've got. Buy him time.");
		w.Done("Clean and bound. He's asking for water. That's a good sign, isn't it?",
			"You bought him a day. Somebody might have the rest of it.");

		w = Task(RZ_EOpeningTask.DEBT, "opening.debt", "The Debt", "press", RZ_EErrandPlace.SUBJECT, 10, 10,
			"Collect what {place} owes. A few seconds at the door is usually enough.");
		w.m_eProp = RZ_EErrandProp.CRATE;
		w.Take("There's {place}, two doors from the corner. They borrowed and they never paid. You know how to ask.",
			"I'm owed, by {place}. I've asked nicely twice. Somebody should ask the other way.");
		w.Done("They paid. I won't ask how you asked.",
			"That's square. Word will get round how it got square. It always does.");

		w = Task(RZ_EOpeningTask.SETTLE, "opening.settle", "Two Families", "talk", RZ_EErrandPlace.HERE, 1, 10,
			"Settle the two families' argument: talk it through with {giver}.");
		w.Take("Two families, one well, and they've stopped speaking. They might listen to someone outside the quarrel.",
			"The Kerns and the Orlovs are at it again, over the last of the coal. Someone has to sit them down.");
		w.Done("Shaking hands. Not smiling, but shaking hands. That'll do for this week.",
			"They'll share the well. They won't share a word, but they'll share the well.");

		m_Lead = Lead(KEY_LEAD,
			"Go and see them at {place}. There's work there, and they feed the ones who do it.",
			"They need hands at {place}, last I heard. Go and be some. I'll manage.");
		m_LeadNote = Lead(KEY_LEAD + ".note",
			"Lower down, in your own handwriting: \"Work at {place}. Ask for it.\"",
			"Under it, in your own handwriting, a reminder you don't remember writing: \"{place}. They had work.\"");

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

		// The clock: a bite doesn't wait
		Def(RZ_EOpeningTask.CLOCK).m_fLapseMinutes = 10;
		Def(RZ_EOpeningTask.CLOCK_MED).m_fLapseMinutes = 10;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_OpeningStepDef Search(string key, float seconds)
	{
		RZ_OpeningStepDef d = new RZ_OpeningStepDef(key, "Down the Road", 1);
		RZ_ErrandWay w = d.Way("sneak");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = seconds;
		w.m_fRadius = 10;
		w.m_bQuiet = true;
		w.m_bEndsThere = true;
		w.m_sStep = "Search {place} down the road, quietly.";
		w.Work(RZ_EWork.SEARCH, "Search it, quietly");
		w.Take("I haven't dared go into {place}. Go and look. Walk, don't run: they hear running.",
			"I need eyes on {place}. Two days I've watched it from here. Nothing moves. That's what worries me.");
		w.Done("You went in and came out again. Most don't, from that end of the road.",
			"Anything? It doesn't matter. You went in. That's more than most manage.");
		return d;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest place with work: done within 40 m of it, nobody to go back to; an hour to get there.
	protected RZ_OpeningStepDef Lead(string key, string a, string b)
	{
		RZ_OpeningStepDef d = new RZ_OpeningStepDef(key, "Where the Work Is", 3);
		d.m_fLapseMinutes = 60;
		RZ_ErrandWay w = d.Way("do");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 3;
		w.m_fRadius = 40;
		w.m_bEndsThere = true;
		w.m_sStep = "Go to {place} and ask for work.";
		w.Take(a, b);
		return d;
	}

	//------------------------------------------------------------------------------------------------
	//! A step 2 by its task: one way, done at `place` (SUBJECT: where the task says) for `seconds`.
	protected RZ_ErrandWay Task(int task, string key, string title, string kind, RZ_EErrandPlace place, float seconds, float radius, string step)
	{
		RZ_OpeningStepDef d = new RZ_OpeningStepDef(key, title, 2);
		d.m_iTask = task;
		m_mTasks.Set(task, d);
		RZ_ErrandWay w = d.Way(kind);
		w.m_ePlace = place;
		w.m_fSeconds = seconds;
		w.m_fRadius = radius;
		w.m_sStep = step;
		// Done where it's done: nobody to go back to (the next step starts there)
		w.m_bEndsThere = true;
		return w;
	}

	//------------------------------------------------------------------------------------------------
	//! One item of this kind, handed over at the place.
	protected void Give(RZ_ErrandWay w, RZ_EErrandItem item)
	{
		w.m_eItem = item;
		w.m_iItems = 1;
		w.m_bGiveThere = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Step 2's definition by its task; null for NONE.
	RZ_OpeningStepDef Def(int task)
	{
		RZ_OpeningStepDef d;
		m_mTasks.Find(task, d);
		return d;
	}

	//------------------------------------------------------------------------------------------------
	RZ_OpeningStepDef Walk(int faction)
	{
		if (faction < 0 || faction >= m_aWalk.Count())
			return m_aWalk[RZ_EFaction.SURVIVORS];

		return m_aWalk[faction];
	}
}
