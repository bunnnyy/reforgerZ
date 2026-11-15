//! An opening's chain (docs/DESIGN.md, "Openings"): its steps on the errand engine
//! (Missions/Errands), handed straight to the player (RZ_Errands.Hand), so the tracker, the Quests
//! tab and the talk screen show them as they show any errand. With someone outside, each step
//! starts with a talk objective (RZ_OpeningTalkDef: "Talk to {giver}.", done only by talking to
//! them), and the step itself is handed in that conversation. The favour's steps are
//! RZ_OpeningFavours'; here: the line (RZ_OpeningLine), the runner and his case (RZ_OpeningCase),
//! the old ends (the walk to where your background belongs, the lead to the nearest work) when no
//! case can be found here, and the step defs of the old step 2s (RZ_OpeningSteps.c: only The Fever
//! still runs, as the Nurse's and the Doctor's way through their favour).
//! The steps pay nothing through the errand ledger: the chain pays its own Fame at the end
//! (RZ_Openings). Server only.

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
	static const string KEY_WALK = "opening.walk";
	static const string KEY_LEAD = "opening.lead";
	static const string KEY_TALK = "opening.talk";

	ref RZ_OpeningTalkDef m_TalkFirst;			// before the first step: whoever's outside
	ref RZ_OpeningTalkDef m_TalkNext;			// back to them, between steps

	ref RZ_OpeningStepDef m_Lead;				// levels 2-3: the nearest place with work, from whoever's outside
	ref RZ_OpeningStepDef m_LeadNote;			// the same with nobody outside: the scene's read says it
	ref RZ_OpeningStepDef m_Case;				// the chain's end: the runner and his case
	ref RZ_OpeningStepDef m_CaseSeen;			// the same, after the line: the man you saw shot
	ref RZ_OpeningStepDef m_LineWalk;			// beat 3, the line: walk with him to the road above it
	ref RZ_OpeningStepDef m_LineWatch;			// and watch it with him (RZ_OpeningLine)
	ref array<ref RZ_OpeningStepDef> m_aWalk = {};	// by the ending faction (RZ_EFaction)
	protected ref map<int, ref RZ_OpeningStepDef> m_mTasks = new map<int, ref RZ_OpeningStepDef>();

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningChain()
	{
		m_TalkFirst = new RZ_OpeningTalkDef(KEY_TALK, "Outside the Door", 0);
		m_TalkNext = new RZ_OpeningTalkDef(KEY_TALK + ".next", "Back to {giver}", 0);

		RZ_ErrandWay w = Task(RZ_EOpeningTask.WATER, "opening.water", "Water", "do", RZ_EErrandPlace.SUBJECT, 15, 8,
			"Fill what you can carry at {place}.");
		w.Work(RZ_EWork.FILL, "Fill what you can carry", 8);
		w.Take("We're down to our last cup of water. There's still water at {place}. Fill whatever you can carry and bring it back here, and we'll both drink tonight.",
			"I haven't had a proper drink since Tuesday. It's not far to {place}. Fill what you can carry there and bring it back. I'd go myself, but you're quicker.");
		w.Done("Clean enough. Boil it if you can. I can't, so I won't.",
			"Thank you. I mean it. I was starting to look at the rain barrel.");

		w = Task(RZ_EOpeningTask.CUT, "opening.hand", "The Cut", "give", RZ_EErrandPlace.HERE, 4, 6,
			"Dress the cut on {giver}'s hand: a bandage, or anything medical you carry.");
		Give(w, RZ_EErrandItem.MEDICAL);
		w.Take("I cut my hand on the shutter getting in, and it's gone hot. That's how an infection starts. If you've a bandage or anything medical in that bag, dress it for me.",
			"Look at this. A nail, getting in, and now it's red and swollen. Have you anything to dress it with? You were always better at this than me.");
		w.Done("That's better already. Or I'm telling myself it is. Either way.",
			"Neat. You've done that before. Thank you.");

		// Someone else is the one in need: they lie there (a mattress on a house's ground floor near the
		// giver, else beside them), the tracker points at them, and you see to them with your hands
		w = Task(RZ_EOpeningTask.FEVER, "opening.fever", "The Fever", "do", RZ_EErrandPlace.HERE, 8, 6,
			"Treat {giver}'s son, lying sick nearby: something medical from your bag.");
		Patient(w, "Treat his fever", RZ_EErrandItem.MEDICAL, "house");
		w.Take("My son's had a high fever since last night. He's lying down just there. Whatever medical you've got in your bag, use it on him. He needs it more than the bag does.",
			"My boy's lying down where it's cool, burning up with fever. You know what to do. Treat him with what's in your bag, and I'll hold his hand.");
		w.Done("He's asleep. Properly asleep, not the other thing. You don't know what that sounds like.",
			"His forehead's wet. That's good, isn't it? Tell me that's good.");

		w = Task(RZ_EOpeningTask.BOILED, "opening.boiled", "Water for the Fever", "do", RZ_EErrandPlace.SUBJECT, 15, 8,
			"Fill what you can carry at {place}, to be boiled for the fever.");
		w.Work(RZ_EWork.FILL, "Fill what you can carry", 8);
		w.Take("There's no medicine left anywhere, so water will have to do: a fever needs fluids. Fill what you can carry at {place} and bring it back. I'll get a fire going to boil it.",
			"If there's nothing in your bag for his fever, there's still water. Bring what you can carry from {place} and I'll boil it for him.");
		w.Done("It's on the fire. Cool cloths till then. You taught me that, once, in a waiting room.",
			"Enough for the night. One night at a time.");

		w = Task(RZ_EOpeningTask.PUMP, "opening.pump", "The Pump", "do", RZ_EErrandPlace.SUBJECT, 5, 15,
			"Fill a can at the pump at {place}.");
		w.Take("My van's out of fuel, and so is every tank on this street. There's still a pump at {place}, if nobody's emptied it. Fill a can there and bring it back, and the van can move again.",
			"Fuel. That's all I need, then I can stop walking everywhere. {place} had a working pump, last I looked. Fill me a can there.");
		w.Done("Smell that. Proper fuel. I could cry, and I'm not going to.",
			"That'll turn her over, if anything will. You always said the old ones start for spite.");

		w = Task(RZ_EOpeningTask.DOWNWIND, "opening.downwind", "Downwind", "sneak", RZ_EErrandPlace.SUBJECT, 30, 25,
			"Watch {place} for half a minute without being seen.");
		w.m_bUnseen = true;
		w.m_bQuiet = true;
		w.Take("A group of the infected moves round {place} at dusk, several together. I need to know how many before we go near. Watch them from downwind, where they can't see you, then come and tell me.",
			"The infected move about together round {place} now. Count them for me from somewhere they can't see or smell you. Then we'll know which way not to go.");
		w.Done("Together, at dusk, the same way every time. Like they still have somewhere to be.",
			"You weren't seen. Good. Then we know where not to be tonight.");

		w = Task(RZ_EOpeningTask.KEYS, "opening.keys", "The Station Keys", "do", RZ_EErrandPlace.SUBJECT, 3, 25,
			"Let yourself into {place} with the key.");
		w.Take("Your lot left me these keys before they went. Here. They open {place}, locked tight since the second day. Nobody's been in. Go and see what's inside: it could be what we need.",
			"You'll know what these keys open. I never did. Try {place}: it's been locked since the second day, so whatever's inside is still there.");
		w.Done("So that's what was in there. All this time, and the key on my nail.",
			"Lock it again behind you next time. It's the only door left on this street that does.");

		w = Task(RZ_EOpeningTask.TOLL, "opening.toll", "The Toll", "talk", RZ_EErrandPlace.SUBJECT, 10, 30,
			"Walk up to {place}, say your piece, and walk away again. No shooting.");
		w.Take("The Red Hand, the gang that runs the roads round here, have a toll at {place}. I need to know what they charge to pass. Walk up, talk to them, don't fight, and walk away again. You know how.",
			"I need to know what the Red Hand charge at {place}, and whether they shoot first. Walk up slowly, say your piece, and walk away slower. Shoot, and none of us use that road again.");
		w.Done("Both hands, and walking. That's how you know it went well.",
			"Good. Now I know what it costs to get anywhere. Everything, mostly.");

		w = Task(RZ_EOpeningTask.SAMPLE, "opening.sample", "A Sample", "do", RZ_EErrandPlace.SUBJECT, 2, 8,
			"Take a sample from {place}.");
		w.Take("There's {place}. The Halcyon lab pays for a piece of one: they study them for the cure. Take a sample from it for them. Gloves, if you've got them.",
			"One of the big infected went down by the road. The Halcyon lab wants tissue from it for their work on the cure. You'd know how to take a sample.");
		w.Done("Keep it cold if you can. Keep it away from me either way.",
			"That's a piece of the way out, they say. It doesn't look like much.");

		w = Task(RZ_EOpeningTask.CHANNEL, "opening.channel", "Open Channel", "do", RZ_EErrandPlace.SUBJECT, 1, 50,
			"Call on the open channel with your radio, and answer whoever's asking.");
		w.Take("Someone has been calling on the open radio channel since dawn, asking if anyone's there. I can hear them, but I can't make my set talk back. You can. Call them back on your radio.",
			"There's a voice on the open radio channel asking if anyone's alive. Somebody should answer. Call them on your radio and say yes.");
		w.Done("Somebody heard you. That's twice as many people as heard them yesterday.",
			"Good. I didn't like that voice on its own.");

		w = Task(RZ_EOpeningTask.SWAP, "opening.swap", "A Fair Swap", "give", RZ_EErrandPlace.SUBJECT, 5, 6,
			"Find {place} on the road and see what {place} will swap.");
		w.Take("{place} comes along this road about now. They trade: a tin of food usually buys you a good word at the Haven, the safehouse people. Go and catch them, and take something to swap.",
			"The people who walk the roads know things, and {place} is about. Take them something to swap, if you've got it. A friend on the road is worth more than a tin.");
		w.Done("There. Now someone out there knows your face. That's worth more than the tin.",
			"See? Nobody gets anything for nothing, but nobody has to.");

		w = Task(RZ_EOpeningTask.GATE, "opening.gate", "Hold the Gate", "do", RZ_EErrandPlace.SUBJECT, 60, 15,
			"Hold {place} at {giver}'s against the infected for a minute.");
		w.Take("The infected have been at the front gate all night, and they'll come at it again. Hold the gate for me, just a minute, while I wedge the back door shut.",
			"Stand at the gate and keep the infected off it for a minute, that's all. Then the back's wedged and we're safe. I'd do it myself, but my hands won't keep still.");
		w.Done("Still standing. You and the gate both.",
			"That's the back wedged. Thank you. I couldn't have done both.");

		w = Task(RZ_EOpeningTask.CLOCK, "opening.clock", "The Clock", "do", RZ_EErrandPlace.HERE, 8, 6,
			"Give the antidote to {giver}'s bitten friend, lying nearby, before the bite takes.");
		Patient(w, "Give him the antidote", RZ_EErrandItem.ANTIDOTE, "any");
		w.Take("My friend's been bitten on the forearm, an hour ago. He's lying just there. You've got an antidote, haven't you? Give it to him now, before the bite takes him. We've minutes, not hours.",
			"My friend's bite is going grey round the edge. That means it's turning. You've an antidote on you. Give it to him, quickly, and he lives.");
		w.Done("His colour's coming back. Is that it? Is that all it takes?",
			"You did it at a run and your hands didn't shake. I'll remember that.");

		w = Task(RZ_EOpeningTask.CLOCK_MED, "opening.clock_med", "The Clock", "do", RZ_EErrandPlace.HERE, 8, 6,
			"Clean and bind the bite of {giver}'s friend, lying nearby: a dressing, anything medical you carry.");
		Patient(w, "Clean and bind the bite", RZ_EErrandItem.MEDICAL, "any");
		w.Take("My friend's been bitten, and there's no antidote anywhere. He's lying just there. Clean and bind the bite with whatever medical you've got, and buy him time till someone has one.",
			"A bite on my friend's forearm. I know we can't cure it with what we've got. Clean it and bind it, so he lasts till someone finds an antidote.");
		w.Done("Clean and bound. He's asking for water. That's a good sign, isn't it?",
			"You bought him a day. Somebody might have the rest of it.");

		w = Task(RZ_EOpeningTask.DEBT, "opening.debt", "The Debt", "press", RZ_EErrandPlace.SUBJECT, 10, 10,
			"Collect what {place} owes. A few seconds at the door is usually enough.");
		w.m_eProp = RZ_EErrandProp.CRATE;
		w.Take("The people in {place}, two doors from the corner, borrowed food from me and never paid it back. I've asked nicely twice. Go and lean on them till they pay. You know how to ask.",
			"I'm owed food by the people in {place}. I've asked nicely twice. Somebody should ask the other way. Collect it for me.");
		w.Done("They paid. I won't ask how you asked.",
			"That's square. Word will get round how it got square. It always does.");

		w = Task(RZ_EOpeningTask.SETTLE, "opening.settle", "Two Families", "talk", RZ_EErrandPlace.HERE, 1, 10,
			"Settle the two families' argument: talk it through with {giver}.");
		w.Take("Two families here, the Kerns and the Orlovs, share one well and have stopped speaking over it. It'll come to a fight soon. They might listen to someone outside the quarrel. Sit down with me and help settle it.",
			"The Kerns and the Orlovs are at it again over the well: who draws water when. Someone has to sit them down before it comes to fists. Help me talk it through.");
		w.Done("Shaking hands. Not smiling, but shaking hands. That'll do for this week.",
			"They'll share the well. They won't share a word, but they'll share the well.");

		m_Lead = Lead(KEY_LEAD,
			"Go and see them at {place}. There's work there, and they feed the ones who do it.",
			"They need hands at {place}, last I heard. Go and be some. I'll manage.");
		m_LeadNote = Lead(KEY_LEAD + ".note",
			"Lower down, in your own handwriting: \"Work at {place}. Ask for it.\"",
			"Under it, in your own handwriting, a reminder you don't remember writing: \"{place}. They had work.\"");

		m_Case = new RZ_OpeningStepDef("opening.case", "The Runner", 3);
		m_Case.m_fLapseMinutes = 90;
		RZ_ErrandWay cw = m_Case.Way("do");
		cw.m_ePlace = RZ_EErrandPlace.SUBJECT;
		cw.m_fSeconds = 1;
		cw.m_fRadius = 12;
		cw.m_bEndsThere = true;
		cw.m_sStep = "Take the case from the runner, {place}.";
		cw.Take("A man tried to run for the Perimeter, and the snipers shot him. He fell {place}. He was carrying a case, and nobody's been out to look. Go and take it before somebody else does.",
			"Whoever he was, he was shot running for the line, and he's lying {place} now. He had a case with him. Get to it and see what it is before somebody else does.");
		cw.Done("That's a Halcyon case, from the lab people working on the cure. If what's in it is still alive, it's the most important thing in the Zone.",
			"That's a Halcyon tag. Don't open it. Whatever's in there belongs to the lab, and they'll want it kept cold.");

		// After the line: the man you saw shot, where he fell
		m_CaseSeen = new RZ_OpeningStepDef("opening.case.seen", "The Runner", 3);
		m_CaseSeen.m_fLapseMinutes = 90;
		RZ_ErrandWay sw = m_CaseSeen.Way("do");
		sw.m_ePlace = RZ_EErrandPlace.SUBJECT;
		sw.m_fSeconds = 1;
		sw.m_fRadius = 12;
		sw.m_bEndsThere = true;
		sw.m_sStep = "Take the case from the runner, {place}.";
		sw.Take("That man was shot running for the line, and he had a pack on him. Whatever was worth that run is still on him. Go down and take it. I can't.",
			"Go down to the dead man, quickly, before the drone comes back. Whatever he was carrying, he thought it was worth dying for. Bring it back.");
		sw.Done("That's a Halcyon case, from the lab people working on the cure. If what's in it is still alive, it's the most important thing in the Zone.",
			"That's a Halcyon tag. Don't open it. Whatever's in there belongs to the lab, and they'll want it kept cold.");

		// Beat 3, the line (RZ_OpeningLine): walk with him to the road above it, then watch it with him
		m_LineWalk = new RZ_OpeningStepDef("opening.line.walk", "The Line", 3);
		m_LineWalk.m_fLapseMinutes = 30;
		RZ_ErrandWay lw = m_LineWalk.Way("do");
		lw.m_ePlace = RZ_EErrandPlace.SUBJECT;
		lw.m_fSeconds = 1;
		lw.m_fRadius = 25;
		lw.m_bEndsThere = true;
		lw.m_sStep = "Walk with {giver} to {place}.";
		lw.Take("I want to see the Perimeter for myself, the line they've drawn round the Zone. The road over the hill goes up to it. Walk me as far as the top, and no further.",
			"There's a road that goes up to the line round the Zone. I want to see it with my own eyes before I believe it. Come as far as the top with me.");

		m_LineWatch = new RZ_OpeningStepDef("opening.line.watch", "The Line", 3);
		m_LineWatch.m_fLapseMinutes = 30;
		RZ_ErrandWay lt = m_LineWatch.Way("do");
		lt.m_ePlace = RZ_EErrandPlace.SUBJECT;
		lt.m_fSeconds = 1;
		lt.m_fRadius = 40;
		lt.m_bEndsThere = true;
		lt.m_sStep = "Watch the line with {giver}.";
		lt.Take("Stop here. You can see the line from here: the road, the checkpoint, the drones. Watch it with me a minute, and you'll see why nobody gets out.",
			"This is as far as anyone goes without being shot. Stay low and watch the line with me a minute.");

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
	//! Seen to with your hands, someone lying there (the giver's son, their bitten friend): a care
	//! work site (Shared > Work sites: the mattress) inside a building of `inside` near the giver
	//! (else beside them), the person lying on it (Props > Duty person, else Puzzles > Civilian), its
	//! hold `label`, and one `item` from your pack taken when it's done (without one, you're told what
	//! it needs). The tracker's diamond is on them, not on the giver.
	protected void Patient(RZ_ErrandWay w, string label, RZ_EErrandItem item, string inside)
	{
		w.Work(RZ_EWork.CARE, label, 8);
		w.m_sInside = inside;
		w.m_bPerson = true;
		w.m_bPersonLies = true;
		Give(w, item);
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
