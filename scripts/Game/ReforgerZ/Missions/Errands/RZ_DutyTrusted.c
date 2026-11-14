//! The rest of the Trusted rung (docs/ERRANDS.md, "The Trusted rung"): the Red Hand's debt book and
//! Halcyon's names on the week's work, written the way the casualty return is (evidence at a place,
//! and a word that follows someone); who gets a dose, at a house, a post and a camp (a dose that
//! exists already, the lab's or the black market's, goes to one of three, never to the player); and
//! Halcyon's animal house. All played alone, in person. Server only.
class RZ_DutyTrusted
{
	// ================================================================ The debt book (the Red Hand)
	//------------------------------------------------------------------------------------------------
	//! Three names in the boss's book, and a door each. What's at the door says whether they're
	//! hiding it, can't pay, or paid already and the book is wrong. Then the book.
	static void DebtBook(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 37);
		array<string> hiding = {"A thin larder, and a loose board by the stove with tins under it. Eleven.", "Bare rooms. A new padlock on the coal shed, and the key on a string round the boy's neck.", "Nothing on the table. Fresh tobacco on the air, and a good coat behind the door."};
		array<string> broke = {"Nothing in the larder but a jar of dripping. Two children under one blanket.", "A man in bed with a leg that won't mend, and his brother boiling nettles.", "The furniture's gone for firewood. The last chair is in the grate."};
		array<string> paid = {"A chit on the nail by the door, with the boss's red thumbprint on it, and last week's date.", "He shows you the mark on his wrist: red thread, the paid knot. The book has no line for it.", "A receipt in the crew's own hand, for the tins and the interest. Somebody didn't copy it into the book."};
		RZ_DutyScript.Into(scene.m_aBeats);
		for (int i = 0; i < 3; i++)
		{
			string who = RZ_DutyPosts.Name(rng);
			int truth = rng.Next() % 3;
			string seen = hiding[rng.Next() % 3];
			if (truth == 1)
				seen = broke[rng.Next() % 3];
			else if (truth == 2)
				seen = paid[rng.Next() % 3];

			RZ_DutyBeat b = RZ_DutyScript.T("debt" + i.ToString(), string.Format("The door of %1, twelve tins in the boss's book. %2", who, seen), RZ_DutyBeat.AT_NEAR_DOOR);
			b.m_bAlone = true;
			array<string> codes = {"wrong", "wrong", "wrong"};
			codes[truth] = "right";
			RZ_DutyOption o = RZ_DutyScript.O("Mark it owed, and send the collectors", "Owed, in the book, and two of the crew will call tomorrow.", codes[0] + " write hold:4");
			if (truth == 1)
				o.Told(string.Format("You sent the collectors to %1. There was nothing there to take, so they took the stove. It's a cold house now, and everyone on the lane knows whose book it was.", who));
			else if (truth == 2)
				o.Told(string.Format("You sent the collectors for a debt %1 had paid. He showed them the knot on his wrist. The crew don't like being made to look like thieves to their own.", who));

			o = RZ_DutyScript.O("Write it off as can't pay", "A line through it, and the words. The boss reads those lines himself.", codes[1] + " write hold:4");
			if (truth == 0)
				o.Told(string.Format("You wrote off %1 as can't pay. He was selling tins at the swap table by the weekend. The boss asked whose hand that line was in.", who));
			else if (truth == 1)
				o.Fx("local").Later("who owed the crew and couldn't pay", "You wrote me off. The boss sent a sack of meal instead of the collectors. I don't know how you managed it.");

			o = RZ_DutyScript.O("Mark it paid", "Paid, with the date. The line's closed.", codes[2] + " write hold:4");
			if (truth == 2)
				o.Fx("rumour").Later("", "Someone went through the boss's book door to door and found the crew had been collecting twice. The book is right now. Nobody's saying who was doing it.");
			else
				o.Told(string.Format("You marked %1 paid. %1 hadn't paid. That's twelve tins out of the camp's pot, and the crew want to know why.", who));
		}

		RZ_DutyBeat sign = RZ_DutyScript.T("book", "The book, on the boss's crate. Three lines, and your thumbprint under them in the red.", RZ_DutyBeat.AT_HERE);
		sign.m_bAlone = true;
		RZ_DutyScript.O("Press your thumb under the lines", "Your print, in the red, beside his. The book is closed for the week.", "write hold:5 record");
	}

	// ================================================================ Whose work it was (Halcyon)
	//------------------------------------------------------------------------------------------------
	//! The week's bench report names who did each run, and the names go in the lab's record. Three
	//! desks: what's on each says who did the work, who helped, and who signed for work that isn't
	//! theirs. It never touches the cure's count or the paper the questline is about.
	static void Names(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 41);
		array<string> did = {"Notebooks in one hand from the first line to the last, the nights included. Coffee rings on every page.", "The run log is in this hand, and so are the corrections. The corrections are good.", "A camp bed folded under the desk. The method's sketched on the blotter, and crossed out, and sketched again, right."};
		array<string> helped = {"A neat hand, the samples labelled and counted. No notes on what they meant.", "Washing-up rota, and the stock book, kept perfectly. The run log has this name only on the delivery lines.", "Two days of the run in this hand, then nothing: a pass for the infirmary, and a cold cup."};
		array<string> signed = {"A clean desk. The run log is signed by this name at the bottom of every page, and written in another.", "A fountain pen, the report's cover, and nothing at all of the work. A note: 'Type mine first.'", "The report's draft, in this hand. The figures are copied, and one is copied wrong."};
		RZ_DutyScript.Into(scene.m_aBeats);
		for (int i = 0; i < 3; i++)
		{
			string who = RZ_DutyPosts.Name(rng);
			int truth = rng.Next() % 3;
			string seen = did[rng.Next() % 3];
			if (truth == 1)
				seen = helped[rng.Next() % 3];
			else if (truth == 2)
				seen = signed[rng.Next() % 3];

			RZ_DutyBeat b = RZ_DutyScript.T("desk" + i.ToString(), string.Format("The desk of %1. %2", who, seen), RZ_DutyBeat.AT_NEAR);
			b.m_bAlone = true;
			array<string> codes = {"wrong", "wrong", "wrong"};
			codes[truth] = "right";
			RZ_DutyOption o = RZ_DutyScript.O("Put the name first, for the work", "First on the report: whose the work was, in the lab's record.", codes[0] + " write hold:3");
			if (truth == 0)
				o.Fx("local").Later("who works the night bench at the lab", "My name's first on the week's report. I didn't ask for it. I'd stopped thinking anyone read the notebooks.");
			else if (truth == 2)
				o.Told(string.Format("You put %1 first on the report. The night bench knows whose method that was. They'll keep their notebooks locked from now on.", who));

			o = RZ_DutyScript.O("Thank them, below the line", "Thanked, at the foot: for help with the run.", codes[1] + " write hold:3");
			if (truth == 0)
				o.Told(string.Format("You put %1 below the line. It was their run. They read the report at the bench, and then they went outside for a while.", who));

			o = RZ_DutyScript.O("Leave the name off", "Not on the report. That's a word on the record, and it follows them.", codes[2] + " write hold:3");
			if (truth == 2)
				o.Fx("rumour").Later("", "There's a name missing from the lab's week report, and it's the one that used to be on all of them. People are reading the notebooks now.");
			else
				o.Fx("morale-").Told(string.Format("You left %1 off the report. Their work is in the notebooks for anyone who looks. Somebody looked.", who));
		}

		RZ_DutyBeat sign = RZ_DutyScript.T("report", "The report, on the director's tray. Three names placed, and a space for yours as the one who read the desks.", RZ_DutyBeat.AT_HERE);
		sign.m_bAlone = true;
		RZ_DutyScript.O("Initial it and put it on the tray", "Initialled. It goes in the record tonight.", "write hold:5 record");
	}

	// ================================================================ Who gets a dose
	//------------------------------------------------------------------------------------------------
	//! One dose and three who are bitten, all of whom asked. Read each bed, then say who. The dose
	//! comes off a stock that exists (the lab's for the Haven and Bastion; the black market's for
	//! the Red Hand) when it's given, and goes to the one you name. It never goes to the player. Or
	//! send it back, and the stock keeps it.
	static void Dose(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 43);
		array<string> who = {RZ_DutyPosts.Name(rng), RZ_DutyPosts.Name(rng), RZ_DutyPosts.Name(rng)};
		array<string> charts = {};
		string back = "Send it back up the hill";
		string backSaid = "Back in its case, to the lab. Nobody here has it, so nobody here was chosen.";
		string backTold = "You sent the dose back. All three are still waiting. One of them won't wait much longer.";
		if (e.m_iFaction == RZ_EFaction.MILITARY)
		{
			charts.Insert(string.Format("Corporal %1. Bitten on the wire yesterday, holding a section together since. They need him.", who[0]));
			charts.Insert(string.Format("Private %1. Eighteen, bitten three days ago, and the youngest on the post.", who[1]));
			charts.Insert(string.Format("%1, the cook's girl, not on the strength. Bitten carrying water to the wire.", who[2]));
			back = "Send it back to the lab";
		}
		else if (e.m_iFaction == RZ_EFaction.BANDITS)
		{
			charts.Insert(string.Format("%1, the crew's best driver. Bitten on a run, and the boss would pay twice over for him.", who[0]));
			charts.Insert(string.Format("%1. Owes the crew forty tins, bitten paying it off on the road.", who[1]));
			charts.Insert(string.Format("%1, the boss's sister's boy. Twelve. Bitten in the yard.", who[2]));
			back = "Sell it back to the fence";
			backSaid = "Back to the fence, who puts it on the shelf with the others. It'll go to whoever can pay.";
			backTold = "You put the dose back on the fence's shelf. A stranger bought it before supper.";
		}
		else
		{
			charts.Insert(string.Format("%1, who keeps the house's well and the night roof. Bitten on the roof, yesterday.", who[0]));
			charts.Insert(string.Format("%1. A mother of three, bitten four days ago and hiding it until this morning.", who[1]));
			charts.Insert(string.Format("%1. Seventy, bitten at the gate holding it shut, and asking only that it be given to someone else.", who[2]));
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		array<string> labels = {"Read the first bed", "Read the second bed", "Read the third bed"};
		for (int i = 0; i < 3; i++)
		{
			int where = RZ_DutyBeat.AT_NEAR;
			if (i == 0)
				where = RZ_DutyBeat.AT_HERE;

			RZ_DutyBeat bed = RZ_DutyScript.T("bed" + i.ToString(), "", where);
			bed.m_bAlone = true;
			RZ_DutyScript.O(labels[i], charts[i] + " They asked for it.", "care hold:3");
		}

		RZ_DutyBeat b = RZ_DutyScript.T("dose", "The case, on the table by the door. One dose in it, and a card for a name.", RZ_DutyBeat.AT_HERE);
		b.m_bAlone = true;
		for (int n = 0; n < 3; n++)
		{
			string later = string.Format("%1 had the dose. The other two are still waiting, and they know who wrote the card.", who[n]);
			RZ_DutyOption o = RZ_DutyScript.O(string.Format("Write %1 on the card", who[n]), string.Format("%1 on the card, and the case goes to their bed.", who[n]), "write hold:4 dose record rumour", "", later);
			if (n == 0)
				o.Pay(1).Told("You chose the one we need most. It was the right sum. I wouldn't want to be the one who did it.");
			else if (n == 1)
				o.Fx("dose,record,rumour,morale+").Told("You chose the one with most life ahead. Everyone understood that one.");
			else if (e.m_iFaction == RZ_EFaction.SURVIVORS)
				o.Fx("dose,record,rumour,regard-").Told("You chose the one who asked to be passed over. They took it, because it was you who asked. They haven't looked at you since.");
			else
			{
				o.Fx("dose,record,rumour,local");
				o.m_sWho = "who was given the dose when nobody thought they would be";
			}
		}

		RZ_DutyScript.O(back, backSaid).Told(backTold);
	}

	// ================================================================ The animal house (Halcyon)
	//------------------------------------------------------------------------------------------------
	//! Three cages and their cards. What each animal does says whether it's turning, whether the dose
	//! is working in it, or whether the card is on the wrong cage. Put it down, keep it for the
	//! trial, or back to the clean room. A turning one kept gets out.
	static void Animals(RZ_Errand e, RZ_DutyScene scene)
	{
		array<string> beast = {"a rhesus monkey", "a rabbit", "a pair of rats", "a dog, a beagle", "a goat", "a cage of mice"};
		array<string> card = {"The card says infected, and dosed on day two.", "The card says infected, and dosed on day four.", "The card says infected, with no dose: kept as the control.", "The card says clean, the control for the clean room.", "The card says infected, and dosed yesterday.", "The card says infected, and the dose doubled."};
		array<string> turning = {"It doesn't look at the light. It looks at your hand.", "Still in the corner, and then not still at all, at the bars.", "Food untouched. Water untouched. It hasn't blinked."};
		array<string> working = {"Thin, and shaking, and it eats when you hold it out.", "Feverish to look at. It flinches from the torch, which is good.", "It sleeps. The breathing is slow and even."};
		array<string> swapped = {"Glossy, quick, curious. Nothing about it matches the card.", "It wants out to play. The card on its cage is for something dying.", "Fat, and clean, and bored. The card says day four."};
		RZ_DutyScript.Into(scene.m_aBeats);
		array<int> drawn = {};
		while (drawn.Count() < 3)
		{
			int pick = Math.RandomInt(0, beast.Count());
			if (!drawn.Contains(pick))
				drawn.Insert(pick);
		}

		foreach (int i, int a : drawn)
		{
			int truth = Math.RandomInt(0, 3);
			string act = turning[Math.RandomInt(0, 3)];
			if (truth == 1)
				act = working[Math.RandomInt(0, 3)];
			else if (truth == 2)
				act = swapped[Math.RandomInt(0, 3)];

			int where = RZ_DutyBeat.AT_NEAR;
			if (i == 0)
				where = RZ_DutyBeat.AT_HERE;

			array<string> codes = {"", "", "flag:clean"};
			if (truth == 0)
				codes[1] = "infect+ heat";

			RZ_DutyBeat b = RZ_DutyScript.Case("cage" + i.ToString(), string.Format("The %1 cage: %2.", RZ_DutyHalcyon.Ord(i), beast[a]), where, 2,
				RZ_DutyScript.A("Read the card on the cage", "Watch it for a while", "Hold out food"),
				RZ_DutyScript.A(card[a], act, act),
				RZ_DutyScript.A("Put it down", "Keep it for the trial", "Back to the clean room"),
				RZ_DutyScript.A("Quick, and in the bin for the incinerator. The cage is scrubbed.", "Kept, and written up for the morning round.", "Moved, card and all, to the clean room, and the card corrected."),
				codes, truth);
			b.m_bAlone = true;
		}

		RZ_DutyBeat log = RZ_DutyScript.T("log", "The animal house log, on its hook by the door.", RZ_DutyBeat.AT_HERE);
		log.m_bAlone = true;
		RZ_DutyScript.O("Sign the log", "Three lines, and the time. The animal house is quiet again.", "write hold:4 record");
	}
}
