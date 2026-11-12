//! The epilogue's words (docs/EPILOGUE.md; how they're written: docs/WRITING.md). Route lines are
//! keyed by the finale's route (0 the Haven, 1 Bastion, 2 the Red Hand, 3 Halcyon, 4 the
//! Independent, -1 nobody) and terms (1-4, as docs/FINALE.md lists them per route). No player names
//! here: callers add frozen, filtered names (RZ_EpiNames).
class RZ_EpiText
{
	//------------------------------------------------------------------------------------------------
	//! "held the lab", "fought for it at the lab"... (finale.how).
	static string How(int how)
	{
		switch (how)
		{
			case 2: return "fought for it at the lab";
			case 3: return "agreed it between them";
			case 4: return "turned it at the end";
			case 5: return "took it from under them";
			case 6: return "were handed it";
		}

		return "held the lab";
	}

	//------------------------------------------------------------------------------------------------
	//! Where the cure is given out, in one sentence (the outlet table's row).
	static string Where(int route, int terms, int faction)
	{
		switch (route)
		{
			case 0:
			{
				if (terms == 3)
					return "The Haven gives it at every big house, by the bite book: the sick first, whoever they are.";
				if (terms == 4)
					return "The Haven gives it at every big house, the crossed-out names first.";

				return "The Haven gives it at its houses. Ask at any big house for the dose.";
			}
			case 1:
			{
				if (terms == 3)
					return "Bastion issues it at the north road and its posts, to anyone bitten. Come unarmed.";

				return "Bastion issues it at the north road and its posts, against name and signature. And one firearm.";
			}
			case 2:
			{
				if (terms == 2)
					return "The Red Hand sells it at Rudi's crossroads. One tin now, one in the book.";
				if (terms == 3)
					return "The Red Hand sent the first pan to a Haven house. The board at the crossroads sells the rest.";
				if (terms == 4)
					return "The Red Hand gives it free at Rudi's crossroads, and writes it in the book.";

				return "The Red Hand sells it at Rudi's crossroads. Two tins a dose.";
			}
			case 3:
			{
				if (terms == 4)
					return "Halcyon published the method. The lab's table gives doses, and houses with a medic make home batches.";

				return "Halcyon gives it at the lab's table, in the order on the door.";
			}
			case 4:
			{
				if (terms == 2)
					return string.Format("It went to %1, on terms. Ask where they give it.", RZ_Factions.GetNameInText(faction));
				if (terms == 3)
					return "It's for sale at a crate on the north road. Thirty Favor, any colour.";
				if (terms == 4)
					return "It's somewhere safe. Nobody will say where.";

				return "There's a crate at the north road stencilled ONE EACH.";
			}
		}

		if (faction >= 0)
			return string.Format("The lab finished it and it went to %1. Ask where they give it.", RZ_Factions.GetNameInText(faction));

		return "The lab finished it. Nobody's giving it out yet.";
	}

	//------------------------------------------------------------------------------------------------
	//! The cure line, as everyone hears it at the commit.
	static string CureLine(int route, int how, int terms, int faction, bool slow)
	{
		string who = "Nobody";
		if (route >= 0 && route < 4)
			who = RZ_Util.Capitalize(RZ_Factions.GetNameInText(route));
		else if (route == 4)
			who = "Nobody's side";

		string line = string.Format("The cure went to %1, who %2.", who, How(how));
		if (route < 0)
		{
			line = string.Format("Nobody held the lab at the end. The cure went to %1.", RZ_Factions.GetNameInText(faction));
			if (faction < 0)
				line = "Nobody held the lab at the end.";
		}

		if (slow)
			line += " The lab's own work, in the end.";

		return line + " " + Where(route, terms, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! The stand board by the lab gate (how it was won), without the names.
	static string StandBoard(int how, int day, string routeName)
	{
		switch (how)
		{
			case 2: return string.Format("HELD THE LAB, DAY %1. Bullet holes in the board. Underneath: CAME FOR IT.", day);
			case 3: return string.Format("HELD THE LAB, DAY %1. Two marks side by side. BY AGREEMENT.", day);
			case 4: return string.Format("HELD THE LAB, DAY %1. Two names scratched out at the top, written in again at the bottom.", day);
			case 5: return string.Format("HELD THE LAB, DAY %1. In a different pen: TOOK IT: %2.", day, routeName);
			case 6: return "A plain board by the gate. BATCH COMPLETE. ISSUED.";
		}

		return string.Format("HELD THE LAB, DAY %1.", day);
	}

	//------------------------------------------------------------------------------------------------
	//! The lab gate's mark (the board by the fence).
	static string LabMark(int route, int terms, int faction)
	{
		switch (route)
		{
			case 0: return "A sheet on two posts by the fence: DOSES WENT TO THE HOUSES. ASK AT ANY HAVEN DOOR.";
			case 1: return "A stencil board: TREATMENT ISSUED AGAINST NAME AND SIGNATURE. NORTH ROAD. 0530.";
			case 2: return "A board with a red hand on it, leaned against the lab's sign post. The lab's stickers are stuck to the step.";
			case 3:
			{
				if (terms == 4)
					return "A notice by the door: THE METHOD. 6 PAGES. READ IT. COPY IT. PASS IT ON.";

				return "A notice by the door: TREATMENT HERE. ORDER OF ISSUE ON THE DOOR.";
			}
			case 4:
			{
				if (terms == 2)
					return string.Format("A board: HANDED TO %1. ON TERMS.", RZ_Factions.GetShortName(faction));
				if (terms == 3)
					return "Chalk on a board: IT'S OURS. ALL OF OURS. 30 FAVOR.";
				if (terms == 4)
					return "Chalk on a board at the clean room door: BACK SOON. The padlock isn't the lab's.";

				return "A board: TAKEN OUT FOR EVERYONE.";
			}
		}

		return "A notice: BATCH COMPLETE. ISSUED. THE LAB'S OWN WORK.";
	}

	//------------------------------------------------------------------------------------------------
	//! The director, on the route.
	static string Director(int route, int terms)
	{
		switch (route)
		{
			case 0: return "\"The Haven has the batch in its kitchens. They're writing down names. We asked for numbers.\"";
			case 1: return "\"The method is in a crate, and the crate is signed for. I signed for it. Twice: the first one smudged.\"";
			case 2: return "\"They took the batch and left the paperwork. They said we could keep the paperwork.\"";
			case 3:
			{
				if (terms == 4)
					return "\"It went out on the open channel at six this morning, every reading. If anyone outside is writing our numbers down, they can stop.\"";

				return "\"We're doing it in order. The order is on the door. The order is correct.\"";
			}
			case 4:
			{
				if (terms == 2)
					return "\"The conditions are written on the back of my own form. They're better than the form.\"";
				if (terms == 3)
					return "\"They're selling it at the crossroads price, minus the crossroads. I'm told that's progress.\"";
				if (terms == 4)
					return "\"It's somewhere safe. Nobody will tell me where. That was my line for three weeks.\"";

				return "\"They took the batch out of my hands and gave it to everyone. I would have done the same. Eventually.\"";
			}
		}

		return "\"Nobody came for it in the end. We finished it anyway. It's what we're for.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The route's table at the queue.
	static string QueueTable(int route, int terms)
	{
		switch (route)
		{
			case 0: return "A pot and cups on a folding table. \"Soup's for the queue. The queue's for everyone.\"";
			case 1: return "A corporal at a folding table with the roster. \"Weapon. Thank you. Name. Sign. No, here. Arm.\"";
			case 2: return "A lad on a kitchen chair with a tin. \"The queue's free. A place near the front isn't.\"";
			case 3: return "Someone in a beak mask with a clipboard, reading out the order of issue.";
			case 4:
			{
				if (terms == 3)
					return "A crate with a price chalked on the lid: 30 FAVOR.";
				if (terms == 4)
					return "Nothing. The queue's a little longer than it would be.";
				if (terms == 2)
					return "A folding table and a note saying where the batch went.";

				return "A crate stencilled ONE EACH, nearly empty, with a name chalked on the lid.";
			}
		}

		return "A folding table with nobody behind it, and a sign: ENQUIRIES.";
	}

	//------------------------------------------------------------------------------------------------
	//! A stronghold leader's verdict on the route (the Haven's chair, the CO, the boss; the
	//! director's is Director()).
	static string Verdict(int faction, int route, int terms, int winner)
	{
		if (faction == RZ_EFaction.INSTITUTE)
			return Director(route, terms);

		if (faction == RZ_EFaction.SURVIVORS)
		{
			switch (route)
			{
				case 0: return "\"Everyone gets a dose, then everyone gets a bowl. In that order, for once.\"";
				case 1: return "\"They want our names before they'll give us a dose. We've got a list of names. We've had one for weeks.\"";
				case 2: return "\"Two tins a dose at the crossroads. The houses are pooling. We're always pooling.\"";
				case 3: return "\"The lab put the recipe on our board. Half of us can't read it. The nurse can. Mostly.\"";
				case 4:
				{
					if (terms == 2)
						return "\"It went out on terms. We read the terms at supper. Twice.\"";
					if (terms == 3)
						return "\"It's for sale at the north road. We're pooling.\"";
					if (terms == 4)
						return "\"Somebody has it somewhere. We've sent a note. We've sent a pot.\"";

					return "\"Somebody walked in with a crate and didn't ask for anything. We've counted it twice.\"";
				}
			}

			return string.Format("\"It came to %1. Nobody fought for it. We'll take it.\"", RZ_Factions.GetNameInText(winner));
		}

		if (faction == RZ_EFaction.MILITARY)
		{
			switch (route)
			{
				case 0: return "\"Cure is with the civilians. Requesting instructions. Over.\" A long hiss. \"No reply.\"";
				case 1: return "\"Cure secured. Distribution by roster. All signed for.\"";
				case 2: return "\"The Red Hand holds the cure. Bastion is buying it. Entered in the log as contribution.\"";
				case 3: return "\"Published without clearance. Noted. Bastion out.\"";
				case 4: return "\"Unauthorised distribution by civilians. Logged. Not stopped. Bastion out.\"";
			}

			return "\"Cure issued. Source: the lab. Logged.\"";
		}

		// The boss
		switch (route)
		{
			case 0: return "\"They're giving it away. You can't sell water by a river. We'll sell cups.\"";
			case 1: return "\"Bastion's handing it out in a queue. We're selling places in the queue.\"";
			case 2:
			{
				if (terms == 1)
					return "\"Market price. Everybody pays. That's what fair means.\"";

				return "\"Already decided. Not by me. The book still balances.\"";
			}
			case 3: return "\"They gave it away. Who does that? We've gone into printing.\"";
			case 4:
			{
				if (terms == 2)
					return "\"Somebody walked in with the only thing worth selling and gave it away on terms. Amateurs with terms.\"";
				if (terms == 3)
					return "\"Somebody's undercutting us with our own business model. I'm nearly proud.\"";
				if (terms == 4)
					return "\"Holding it till the price is right. Now that I respect.\"";

				return "\"Free. Fine. Then nobody owes anybody. First time for everything.\"";
			}
		}

		return "\"Nobody won. Everybody's buying. Best week we've had.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's price board under the boss's price, by the evolving step.
	static string PriceBoard(int step, bool dropped)
	{
		if (step >= 3)
			return "The price board at the crossroads: FREE. WE'RE NOT CARRYING IT OUT.";

		if (dropped)
			return "The price board at the crossroads: 1 TIN. The 2 is painted over, not well.";

		switch (step)
		{
			case 1: return "The price board at the crossroads: 2 TINS. NO CREDIT.";
			case 2: return "The price board at the crossroads: 2 TINS. LAST WEEK.";
		}

		return "The price board at the crossroads: 2 TINS.";
	}

	//------------------------------------------------------------------------------------------------
	//! The queue's sign by the evolving step, with the count waiting.
	static string QueueSign(int step, int waiting, string table)
	{
		string sign = "Bags on the verge, and people sitting on them. Nobody's been processed.";
		if (step == 1)
			sign = "A sign on a post: QUEUE STARTS HERE.";
		else if (step == 2)
			sign = "A sign on a post: QUEUE STARTS HERE. STILL.";
		else if (step >= 3)
			sign = string.Format("A sign on a post: QUEUE STARTS HERE, with an arrow. Underneath: %1 WAITING.", waiting);

		return sign + " " + table;
	}

	//------------------------------------------------------------------------------------------------
	//! What someone in the queue says, by step.
	static string QueueTalk(int step)
	{
		if (step >= 3)
			return "\"I've packed twice. I keep taking the kettle out.\"";

		return "\"Been here since the order. Nobody's been processed. I'm keeping a place for my sister. She won't come, but it's a good place.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse's candles, by step: died this season.
	static string Candles(int step, int died)
	{
		if (died <= 0)
			return "";

		if (step >= 2)
			return string.Format("Candles on the step, all out, the wax scraped up for the next ones. %1 chalked on the stone.", died);

		return string.Format("Candles on the step, and %1 chalked on the stone beside them. Two are still lit.", died);
	}

	//------------------------------------------------------------------------------------------------
	//! A resident's line in the week, in the first person.
	static string Resident(int step, string house, int died, int grew)
	{
		if (step >= 3)
			return "\"I've packed twice. I keep taking the kettle out.\"";

		if (grew > 0)
			return string.Format("\"There's a new rota on the door. %1 names on it. No excuses.\"", grew);

		if (died > 0)
			return string.Format("\"%1 of ours this season. The head reads them at supper. Then the ones we don't say.\"", died);

		return "\"Gates open soon, they say. I'll believe it when I'm on the other side of one.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer's road news in the week, by role (RZ_EWandererRole as an int: 0 trader, 1 doctor,
	//! 2 scavenger, 3 deserter, 4 preacher, 5 courier, 6 kid, 7 mechanic).
	static string Road(int role, string where, string echo)
	{
		switch (role)
		{
			case 0: return "\"Tins are worth nothing and everything now. " + where + "\"";
			case 1: return "\"If you're bitten, don't wait. " + where + "\"";
			case 3: return "\"They'll read the roll at the north road. Nobody answers for a man like me.\"";
			case 4: return "\"I read the names where I rest. It takes longer every town.\"";
			case 5: return "\"Last letters. Everybody's writing to somebody who isn't there.\"";
			case 6: return "\"Going to the queue. Everyone is.\"";
			case 7: return "\"The road north is clear to the bend. Leave a car by the queue, if you've got one spare.\"";
		}

		if (!echo.IsEmpty())
			return "\"Worth a look: " + echo + "\"";

		return "\"Places worth seeing, now there's time. Mostly boards.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's open channel in the week (the chatter's slot).
	static string Chatter(int faction, int route, int step)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (step >= 3)
					return "\"Mine's in a car on the north road. If you can hear this, the pot's still on at the houses.\"";

				return "\"The houses are open. The pot's on. Bring a cup, and bring the names of anyone you've lost.\"";
			}
			case RZ_EFaction.MILITARY:
			{
				if (route == 1)
					return "\"All stations: issue at the north road from 0530. Roster first. Bastion out.\"";

				return "\"All stations: listening watch at 20:30. Nothing heard so far. Bastion out.\"";
			}
			case RZ_EFaction.BANDITS:
			{
				if (route == 2)
					return "\"Crossroads board says two tins. The board is always right.\"";

				return "\"Queue's free at the north road. Places near the front aren't. Ask the lad.\"";
			}
		}

		if (route == 3)
			return "\"The lab's table is open. Order of issue on the door. Please read the door.\"";

		return "\"The lab is closed to samples. We're writing it up. It will take longer than the cure did.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The secure channel under it (Radio Operators).
	static string Secure(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "\"All stations: issue to the roster first. Civilians after. Log it as priority.\"";
			case RZ_EFaction.BANDITS: return "\"Double at the checkpoint road. Half price for the Haven. Don't tell the Haven it's half.\"";
			case RZ_EFaction.INSTITUTE: return "\"How many copies of page one are left? Don't hand them out at the barrier.\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The four grey choices (0 the chair, 1 the CO, 2 the boss, 3 the director): the question.
	static string Question(int which)
	{
		switch (which)
		{
			case 0: return "\"Whose names do we read tonight?\"";
			case 1: return "\"The 20:30 watch. Do we keep listening?\"";
			case 2: return "\"The price. Leave it to the market?\"";
			case 3: return "\"The thing in the isolation room. Do we keep it for the paper?\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A grey choice's answer label (first: the usual way).
	static string Answer(int which, bool first)
	{
		switch (which)
		{
			case 0:
			{
				if (first)
					return "\"Ours, as usual.\"";
				return "\"All of them. The ones you put out too.\"";
			}
			case 1:
			{
				if (first)
					return "\"Keep the watch.\"";
				return "\"Close the net.\"";
			}
			case 2:
			{
				if (first)
					return "\"Leave it to the market.\"";
				return "\"Drop it now. One tin.\"";
			}
			case 3:
			{
				if (first)
					return "\"Keep it for the paper.\"";
				return "\"Let it go quiet.\"";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! What everyone hears when a grey choice is decided.
	static string Decided(int which, bool first)
	{
		switch (which)
		{
			case 0:
			{
				if (first)
					return "The Haven reads its own names tonight, as it always has.";
				return "The Haven reads all of them tonight, the exiles too. Every board has a note pinned up: if we put you out, come and eat.";
			}
			case 1:
			{
				if (first)
					return "Bastion keeps the 20:30 watch. Five minutes on the net, every evening, to the end.";
				return "\"Bastion closing the net. No reply expected. Bastion out.\" The roster board gets a new line: NET CLOSED.";
			}
			case 2:
			{
				if (first)
					return "The Red Hand leaves the price to the market. The board stays at two tins.";
				return "The price board at the crossroads goes to one tin, today. The boss calls it bad for the book.";
			}
			case 3:
			{
				if (first)
					return "Halcyon keeps the index in isolation for the paper. The night shift keeps its earplugs.";
				return "The isolation room is quiet. The night shift took their earplugs out.";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The Last Broadcast's versions (0 the names, 1 the three weeks, 2 the truth, 3 the plain line).
	static string BroadcastName(int version)
	{
		switch (version)
		{
			case 0: return "the names";
			case 1: return "the three weeks";
			case 2: return "the truth";
		}

		return "the plain line";
	}

	//------------------------------------------------------------------------------------------------
	static string BroadcastText(int version, string names)
	{
		switch (version)
		{
			case 0:
			{
				if (names.IsEmpty())
					return "\"This is the Zone. These are our dead. We don't have all their names. We've written down the ones we have.\"";

				return "\"This is the Zone. These are our dead: " + names + ". And the ones we don't say.\"";
			}
			case 1: return "\"This is the Zone. The lab knew three weeks before Day 1. It's written in its own pages. We're reading them out.\"";
			case 2: return "\"This is the Zone. Every page we found is in the open now: the List, the orders, the book's back page, the site log. Ask for them.\"";
		}

		return "\"This is the Zone. We're still here.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! Each background's last goal (docs/EPILOGUE.md, Personal Goals). "" for a background with none.
	static string LastGoal(string bg)
	{
		switch (bg)
		{
			case "nurse": return "The last round: be there when someone is dosed, or bring a cup to the queue twice.";
			case "mechanic": return "One for the queue: leave a running car at the queue on the north road.";
			case "hunter": return "The cabin: reach your old town once more, then bring something back to a house.";
			case "police": return "Close the file: carry one of the dead home (Bring Them Home).";
			case "excon": return "The stash: go back to your old town, then leave what you find at the queue.";
			case "labtech": return "Into the record: carry a name home to the lab, or bring something to the station's last broadcast.";
			case "radioham": return "Sign off: bring something to the station's last broadcast.";
			case "unemployed": return "Something to do: finish two pieces of closing work of any kind.";
			case "soldier": return "Stand down: sit out a pack's last call on the ridges, or carry one of the dead home.";
			case "doctor": return "The last clinic: be there when two people are dosed, or sit with someone who's bitten.";
			case "raider": return "Square it: pay into the book at the crossroads, or buy a keepsake back from the fence.";
			case "elder": return "Read them out: hear the names at the long table at supper.";
			case "drifter": return "Somewhere to stop: put your name down for the walk, or walk someone to their kitchen.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A wanted poster at a holding, by its state (0 wanted, 1 settled, 2 paid, 3 not paid, 4 walked out).
	static string Poster(string name, int faction, int state, int day)
	{
		switch (state)
		{
			case 1: return string.Format("A wanted poster for %1, stamped across the face: SETTLED. DAY %2.", name, day);
			case 2: return string.Format("A wanted poster for %1, stamped across the face: PAID. DAY %2.", name, day);
			case 3: return string.Format("A wanted poster for %1: NOT PAID. THE INFECTED GOT THERE FIRST.", name);
			case 4: return string.Format("A wanted poster for %1: WALKED OUT. NOT PAID.", name);
		}

		return string.Format("A wanted poster: %1. BY %2. THE CURE DOESN'T COVER IT.", name, RZ_Factions.GetShortName(faction));
	}
}
