//! The beats of the duties you stand alone at (docs/ERRANDS.md, "A watch", "The Trusted rung"): a
//! watch and what comes by it, the lock-up and what its prisoner says, the names of the dead and who
//! is told, the list and who goes on it. Data for RZ_DutyScenes. The people in them are named from
//! the place and the season day, so the same watch has the same prisoner all night. Server only.
class RZ_DutyPosts
{
	//------------------------------------------------------------------------------------------------
	//! Names that stay the same at one place for one season day.
	static RZ_Rng Seed(RZ_Errand e, int salt)
	{
		int place = e.m_iHouse + 1;
		if (e.m_iHolding >= 0)
			place = 100 + e.m_iHolding;

		return new RZ_Rng(place * 7919 + RZ_SeasonClock.Day() * 104729 + salt * 31 + 17);
	}

	//------------------------------------------------------------------------------------------------
	static string Name(RZ_Rng rng)
	{
		return RZ_NoteText.FirstName(rng) + " " + RZ_NoteText.Surname(rng);
	}

	// ================================================================ A watch
	//------------------------------------------------------------------------------------------------
	//! One thing that comes by a post. `what`: 0 a threat (stand the gate to), 1 someone (send
	//! someone out), 2 nothing (let it pass). The line holds the tell.
	protected static RZ_DutyBeat Sighting(string key, string line, int what, float wait)
	{
		RZ_DutyBeat b = new RZ_DutyBeat(key, line, RZ_DutyBeat.AT_SAME);
		b.m_fWait = wait;
		b.m_bScored = true;
		b.m_bAlone = true;
		RZ_DutyOption o = b.Opt("Stand the gate to", "The gate stands to. Rifles on the wall, lamps out.");
		if (what == 0)
			o.Right(1).Told("You stood the gate to for the right thing. It came, and it found us ready.");
		else
			o.Right(-1).Told("You had the gate stood to for nothing. People who were asleep remember that.");

		o = b.Opt("Send someone out to them", "Someone goes out, slowly, with a lamp held wide.");
		if (what == 1)
			o.Right(1).Fx("local").Later("who was met on the road by the gate at night", "Someone came out to me with a lamp, because of whoever had the post. I'd have walked past otherwise.");
		else if (what == 0)
			o.Right(-1).Fx("morale-").Told("You sent one of ours out to that. They came back. Only just.");
		else
			o.Right(-1);

		o = b.Opt("Let it pass", "You watch it, and write the time, and let it be.");
		if (what == 2)
			o.Right(1);
		else if (what == 0)
			o.Right(-1).Fx("infect+").Told("What you let pass on your watch was at the back fence by morning.");
		else
			o.Right(-1);

		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! A watch: take the post, then three things come by, a while apart. Each is a threat, someone,
	//! or nothing, and the line says enough to tell, if you think about it.
	static void Watch(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_DutyBeat post = new RZ_DutyBeat("post", "", RZ_DutyBeat.AT_GATE);
		post.m_bAlone = true;
		post.Opt("Take the post", "You have the post. Three things will come by before you're relieved. Each is a threat, or someone, or nothing: stand the gate to, send someone out, or let it pass.").Work(RZ_EWork.HANDS, 5);
		scene.m_aBeats.Insert(post);
		array<ref RZ_DutyBeat> pool = {};
		pool.Insert(Sighting("stands", "A shape on the road, slow, alone. It stops at the marker and stands there, facing nothing.", 0, 0));
		pool.Insert(Sighting("sits", "A shape on the road, slow, alone. It stops at the marker and sits down on its pack.", 1, 0));
		pool.Insert(Sighting("lamp", "A light in the field, low, moving left to right, and held still when the wind gets up.", 1, 0));
		pool.Insert(Sighting("wire", "Something at the fence, low, pushing. The wire sings. Not a sound from whatever it is.", 0, 0));
		pool.Insert(Sighting("fox", "Something at the fence, low, pushing. A whine, and the wire goes slack again.", 2, 0));
		pool.Insert(Sighting("hedge", "Three shapes close together, keeping to the hedge, and stopping each time the cloud clears.", 1, 0));
		pool.Insert(Sighting("door", "A door banging across the road, every few seconds, in time with the wind.", 2, 0));
		pool.Insert(Sighting("voice", "A voice, far off, calling one name over and over. A woman's.", 1, 0));
		pool.Insert(Sighting("feet", "Feet on the road, a great many, not in step. And no voices at all.", 0, 0));
		pool.Insert(Sighting("cart", "A cart with one wheel squealing, and somebody swearing at it under their breath.", 1, 0));
		pool.Insert(Sighting("birds", "Every bird in the far wood goes up at once. Then nothing, for a long time.", 2, 0));
		pool.Insert(Sighting("crawl", "Something on the road on its hands, coming steadily, never once lifting its head.", 0, 0));
		// Bastion's stag at the wire is a longer relief: four come by
		int relief = 3;
		if (e.m_iFaction == RZ_EFaction.MILITARY)
			relief = 4;

		for (int i = 0; i < relief && !pool.IsEmpty(); i++)
		{
			int pick = Math.RandomInt(0, pool.Count());
			RZ_DutyBeat b = pool[pick];
			pool.Remove(pick);
			b.m_fWait = 25 + Math.RandomInt(0, 20);
			scene.m_aBeats.Insert(b);
		}
	}

	// ================================================================ The lock-up
	//------------------------------------------------------------------------------------------------
	//! A night's watch on someone held. They talk. Keep the watch, pass on what they said, or open
	//! the door: nobody sees which, tonight, but the door was yours to keep.
	static void LockUp(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = Seed(e, 3);
		string who = Name(rng);
		RZ_DutyBeat door = new RZ_DutyBeat("door", "", RZ_DutyBeat.AT_GATE);
		door.m_bAlone = true;
		door.m_sThing = RZ_DutyThings.For("lockup:door");
		scene.m_sDress = RZ_DutyThings.LockUp(e.m_iFaction);
		door.Opt("Take the watch at the door", string.Format("You have the door. %1 is behind it. Nobody else is awake.", who)).Work(RZ_EWork.HANDS, 5);
		scene.m_aBeats.Insert(door);
		string says;
		string freed;
		if (e.m_iFaction == RZ_EFaction.MILITARY)
		{
			says = string.Format("%1, through the door, quietly. I didn't run from the dead. I ran from the order to fire on the road. Ask them what was on the road.", who);
			freed = "I'm two valleys over, and nobody's fired on a road since. I won't say your name, even to myself.";
		}
		else if (e.m_iFaction == RZ_EFaction.BANDITS)
		{
			says = string.Format("%1, through the boards. I owe the crew forty tins and I've a girl at the Haven who'll starve paying it. Tell the boss I'll work it off. Or don't tell him anything.", who);
			freed = "I got to her. We're paying it off, slower than the boss would like. You're the reason there's anyone left to pay.";
		}
		else
		{
			says = string.Format("%1, through the door. I'm not bitten. I'm frightened, and they can't tell the difference in the dark. Let me sit where there's a window.", who);
			freed = "They found me asleep under a window in the morning, and no worse. Nobody asked how. Thank you for the window.";
		}

		RZ_DutyBeat talk = new RZ_DutyBeat("talk", says, RZ_DutyBeat.AT_SAME);
		talk.m_fWait = 30;
		talk.m_bAlone = true;
		talk.m_sThing = door.m_sThing;
		RZ_DutyOption o = talk.Opt("Keep the watch", "You keep the door until you're relieved. The talking stops, after a while.");
		o = talk.Opt("Write down what was said", "Written down, word for word, for whoever reads the watch book.");
		o.Fx("report,record").Pay(0, 1).Told("What you wrote in the watch book was read out this morning. It changed what happens to them. I won't say how.");
		o = talk.Opt("Open the door", "The bar comes up. Bare feet on the yard, then nothing. You bar it again behind them.");
		o.Pay(-20).Flag("freed").Fx("local,morale-").Later("who you let out of the lock-up", freed).Told("The door was barred this morning and the room was empty. It was your watch. I'm not asking. I'm telling you what that costs.");
		o.m_iInfamy = 3;
		scene.m_aBeats.Insert(talk);
	}

	// ================================================================ The names
	//------------------------------------------------------------------------------------------------
	//! The Haven's Book of Names: the house's dead this week, a door each, and how each family is
	//! told. Then the book.
	static void Names(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene, int count)
	{
		RZ_Rng rng = Seed(e, 5);
		array<string> kin = {"mother", "husband", "sister", "son", "wife", "brother"};
		for (int i = 0; i < count; i++)
		{
			string who = Name(rng);
			string relation = kin[rng.Next() % kin.Count()];
			RZ_DutyBeat b = new RZ_DutyBeat("name" + i.ToString(), string.Format("The door of %1. The %2 is inside, and doesn't know yet.", who, relation), RZ_DutyBeat.AT_GATE);
			b.m_bAlone = true;
			RZ_DutyOption o = b.Opt("Knock, and tell them all of it", "You tell it all, how and where. They hold the door frame. They thank you, which is the worst part.");
			o.Fx("record,later").Later("", string.Format("The %1 of %2 asked for you at the burying. You told them straight. They wanted you to stand with them.", relation, who));
			o = b.Opt("Knock, and tell them gently", "You tell them it was quick. It wasn't. They want to believe it, and they do, for now.");
			o.Fx("record,later").Later("", string.Format("The %1 of %2 heard the rest of it from someone else. They know why you didn't say, or most of them do.", relation, who));
			o = b.Opt("Leave it for the clerk", "You walk past the door. It'll come in the clerk's handwriting, tomorrow, with a number on it.");
			o.Pay(-2).Fx("regard-");
			scene.m_aBeats.Insert(b);
		}

		RZ_DutyBeat book = new RZ_DutyBeat("book", "The book, on its shelf by the stove. Every name the house has lost is in it.", RZ_DutyBeat.AT_HERE);
		book.m_bAlone = true;
		RZ_DutyOption write = book.Opt("Write the names in the book", "Written, in ink, with the day. They're the house's now, for as long as there's a house.");
		write.Work(RZ_EWork.WRITE, 6).Fx("record,morale+,rumour").Later("", "The names were written in the book last night, and read at the stove. Nobody left before the end.");
		scene.m_aBeats.Insert(book);
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's casualty return: three bunks, and what's left at each says what became of the man.
	//! Killed, missing or gone of his own will: the return says which, and the wrong word follows him.
	static void Return(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = Seed(e, 7);
		array<string> killed = {"His kit is all here, squared away, and a letter half written on the blanket.", "Everything in its place. A photograph tucked in the locker door, and his boots under the bunk.", "His rifle came back without him. It's on the bunk, tagged, with a hand's mark dried on the stock."};
		array<string> missing = {"His kit is here. His rifle and his water are not, and he was on the far patrol.", "Bunk made, locker full. The section says he was last seen going back for someone.", "Everything here but what he'd carry out. The patrol's map has a cross where they lost sight of him."};
		array<string> gone = {"The locker is empty to the paint. Boots gone, both blankets gone, and the ration tin.", "His tags are on the pillow. A man doesn't lose his tags on a pillow.", "Kit gone, bunk stripped, and a private two bunks down who won't look at you."};
		for (int i = 0; i < 3; i++)
		{
			string who = Name(rng);
			int truth = rng.Next() % 3;
			string left = killed[rng.Next() % 3];
			if (truth == 1)
				left = missing[rng.Next() % 3];
			else if (truth == 2)
				left = gone[rng.Next() % 3];

			RZ_DutyBeat b = new RZ_DutyBeat("bunk" + i.ToString(), string.Format("The bunk of Private %1. %2", who, left), RZ_DutyBeat.AT_HERE);
			b.m_bScored = true;
			b.m_bAlone = true;
			RZ_DutyOption o = b.Opt("Return him killed", "Killed. A letter goes to a family, if the letters ever go.");
			o.Right(-1);
			if (truth == 0)
				o.Right(1);
			else if (truth == 2)
				o.Told(string.Format("You returned %1 killed. He was seen at a Red Hand toll on Tuesday. The letter had already been written.", who));

			o = b.Opt("Return him missing", "Missing. He stays on the strength, and his ration is still drawn.");
			o.Right(-1);
			if (truth == 1)
				o.Right(1);

			o = b.Opt("Return him gone of his own will", "Absent. His name goes on the list at every post.");
			o.Right(-1);
			if (truth == 2)
				o.Right(1).Fx("rumour").Later("", string.Format("Private %1 is posted absent at every Bastion gate. Somebody did the return properly.", who));
			else
				o.Fx("morale-").Told(string.Format("You posted %1 as a deserter. His section carried what was left of him in yesterday. They know who signed the return.", who));

			scene.m_aBeats.Insert(b);
		}

		RZ_DutyBeat sign = new RZ_DutyBeat("sign", "", RZ_DutyBeat.AT_SAME);
		sign.m_bAlone = true;
		sign.Opt("Sign the return", "Signed. Three lines, three men, and your name under them.").Work(RZ_EWork.WRITE, 5).Fx("record");
		scene.m_aBeats.Insert(sign);
	}

	// ================================================================ The list
	//------------------------------------------------------------------------------------------------
	//! Halcyon's Trial List: three beds, a chart at each, and one place on the list. Nothing here is
	//! the right answer. It is written down, it is spoken of, and the two you didn't choose know.
	static void TrialList(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = Seed(e, 11);
		string young = Name(rng);
		string old = Name(rng);
		string kin = Name(rng);
		RZ_DutyBeat b = new RZ_DutyBeat("chart0", "", RZ_DutyBeat.AT_HERE);
		b.m_bAlone = true;
		b.Opt("Read the chart at the first bed", string.Format("%1. Nineteen, bitten two days ago, strong. The dose would very likely hold. It would tell the trial almost nothing.", young)).Work(RZ_EWork.CARE, 3);
		scene.m_aBeats.Insert(b);
		b = new RZ_DutyBeat("chart1", "", RZ_DutyBeat.AT_NEAR);
		b.m_bAlone = true;
		b.Opt("Read the chart at the second bed", string.Format("%1. Sixty-one, bitten six days ago, failing. The dose would probably not hold. What it did on the way would tell the trial a great deal.", old)).Work(RZ_EWork.CARE, 3);
		scene.m_aBeats.Insert(b);
		b = new RZ_DutyBeat("chart2", "", RZ_DutyBeat.AT_NEAR);
		b.m_bAlone = true;
		b.Opt("Read the chart at the third bed", string.Format("%1. Thirty-four, bitten four days ago. Nothing marks the chart out, except a note in another hand: the night technician's sister.", kin)).Work(RZ_EWork.CARE, 3);
		scene.m_aBeats.Insert(b);
		b = new RZ_DutyBeat("list", "The list, on its board by the door. One line is empty. Whoever you write there is who the trial tries to save.", RZ_DutyBeat.AT_HERE);
		b.m_bAlone = true;
		RZ_DutyOption o = b.Opt("Write in the one most likely to live", string.Format("%1, in block capitals. A life, very likely. A page of nothing for the trial.", young));
		o.Work(RZ_EWork.WRITE, 5).Fx("record,rumour,morale+").Later("", string.Format("%1 is on Halcyon's list. %2 and %3 are not. The one who chose picked the one who'd live.", young, old, kin)).Told("You chose the one who'd live. The director calls that sentiment. The ward calls it something else.");
		o = b.Opt("Write in the one the trial learns most from", string.Format("%1, in block capitals. The trial gets its data. The ward goes quiet when you turn round.", old));
		o.Work(RZ_EWork.WRITE, 5).Fx("record,rumour").Pay(1).Later("", string.Format("%1 is on Halcyon's list. %2 and %3 are not. They say the choice was made for the data.", old, young, kin)).Told("You chose the useful one. It has been noted, in the good column. Don't look for thanks on the ward.");
		o = b.Opt("Write in the technician's sister", string.Format("%1, in block capitals. No reason you could put on a form. The night technician will know by morning.", kin));
		o.Work(RZ_EWork.WRITE, 5).Fx("record,rumour,local").Later("who is the night technician at the station", string.Format("You wrote %1 on the list. I didn't ask you to. I won't forget that I didn't have to.", kin)).Told("You chose a name with a friend on the staff. People will say so. Some of them will be glad.");
		scene.m_aBeats.Insert(b);
	}
}
