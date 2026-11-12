//! Everything a faction leader says (docs/WRITING.md, "The voices"; docs/REPUTATION.md,
//! "Faction leaders"). Kept as small tables so the same contract reads differently from a
//! Greedy crew boss and a Merciful council head. Faction voices:
//!   Haven     warm, worried
//!   Bastion   clipped, procedural, tired
//!   Red Hand  mocking, casual
//!   Halcyon   precise, a bit cold, sometimes guilty
//! Every pick takes an RZ_Rng, so one leader's lines stay consistent with their seed.
class RZ_LeaderVoice
{
	//------------------------------------------------------------------------------------------------
	static string TraitName(int trait)
	{
		switch (trait)
		{
			case RZ_ELeaderTrait.GREEDY: return "Greedy";
			case RZ_ELeaderTrait.PARANOID: return "Paranoid";
			case RZ_ELeaderTrait.MERCIFUL: return "Merciful";
			case RZ_ELeaderTrait.RUTHLESS: return "Ruthless";
			case RZ_ELeaderTrait.IDEALIST: return "Idealist";
			case RZ_ELeaderTrait.PRAGMATIST: return "Pragmatist";
			case RZ_ELeaderTrait.VETERAN: return "Veteran";
			case RZ_ELeaderTrait.TALKER: return "Talker";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string LoyaltyName(RZ_ELoyalty loyalty)
	{
		switch (loyalty)
		{
			case RZ_ELoyalty.LOYAL: return "Loyal";
			case RZ_ELoyalty.INDEPENDENT: return "Independent";
			case RZ_ELoyalty.ROGUE: return "Rogue";
		}

		return "";
	}

	// ================================================================ Names and titles
	//------------------------------------------------------------------------------------------------
	//! "the commanding officer of Task Force Bastion", "the council head at the safehouse in Morton".
	static string Title(RZ_EFaction faction, RZ_ELeaderRole role, string place)
	{
		if (role == RZ_ELeaderRole.SAFEHOUSE)
			return "council head at the safehouse in " + place;

		if (role == RZ_ELeaderRole.STRONGHOLD)
		{
			switch (faction)
			{
				case RZ_EFaction.SURVIVORS: return "chair of the Haven council";
				case RZ_EFaction.MILITARY: return "commanding officer of Task Force Bastion";
				case RZ_EFaction.BANDITS: return "boss of the Red Hand";
				case RZ_EFaction.INSTITUTE: return "research director of Halcyon";
			}
		}

		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "council head in " + place;
			case RZ_EFaction.MILITARY: return "outpost commander in " + place;
			case RZ_EFaction.BANDITS: return "crew boss in " + place;
			case RZ_EFaction.INSTITUTE: return "lead researcher at the field station in " + place;
		}

		return "leader in " + place;
	}

	//------------------------------------------------------------------------------------------------
	//! Display name ("Major Anton Weber") and the short form people use ("Weber").
	static void MakeName(RZ_Rng rng, RZ_EFaction faction, RZ_ELeaderRole role, out string display, out string shortName)
	{
		string first = RZ_NoteText.FirstName(rng);
		string last = RZ_NoteText.Surname(rng);
		switch (faction)
		{
			case RZ_EFaction.MILITARY:
			{
				array<string> ranks = {"Lieutenant", "Sergeant", "Captain"};
				string rank = rng.Pick(ranks);
				if (role == RZ_ELeaderRole.STRONGHOLD)
					rank = "Major";

				display = string.Format("%1 %2 %3", rank, first, last);
				shortName = rank + " " + last;
				return;
			}
			case RZ_EFaction.BANDITS:
			{
				array<string> nicks = {"Teeth", "the Tailor", "Smiles", "Deacon", "Ledger", "Polka", "Sixpence", "the Dentist"};
				string nick = rng.Pick(nicks);
				display = string.Format("%1 \"%2\" %3", first, nick, last);
				shortName = nick;
				return;
			}
			case RZ_EFaction.INSTITUTE:
			{
				display = string.Format("Dr %1 %2", first, last);
				shortName = "Dr " + last;
				return;
			}
		}

		display = first + " " + last;
		shortName = first;
	}

	//------------------------------------------------------------------------------------------------
	//! A leader with a job for you that they haven't offered yet: said at the greeting, so you know
	//! to ask (RZ_Leaders.HandleTalk).
	static string JobWaiting(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "Before you go, I've a job that needs someone from outside the house. Ask me about it when you're ready.";
			case RZ_EFaction.MILITARY: return "There's a task on my desk that needs a body who isn't on my roster. Ask me for work if you want it.";
			case RZ_EFaction.BANDITS: return "I've got something that pays, if you're the asking sort. So ask.";
			case RZ_EFaction.INSTITUTE: return "There is work, if you want it. The lab is short of something only someone outside can fetch. Ask.";
		}

		return "I might have something for you. Ask me for it.";
	}

	//------------------------------------------------------------------------------------------------
	//! What a pitched job pays, said by the giver (RZ_Leaders.Pitch).
	static string PayLine(RZ_EFaction faction, int favor, int fame)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("It's worth %1 Favor with the houses, and people will hear who helped. That's %2 Fame, for what it's worth.", favor, fame);
			case RZ_EFaction.MILITARY: return string.Format("Pay is %1 Favor on completion, and %2 Fame on the roster. Nothing up front.", favor, fame);
			case RZ_EFaction.BANDITS: return string.Format("%1 Favor in the book when it's done, and %2 Fame, because people talk.", favor, fame);
			case RZ_EFaction.INSTITUTE: return string.Format("The lab will credit you %1 Favor, and %2 Fame. Those are the numbers.", favor, fame);
		}

		return string.Format("It pays %1 Favor and %2 Fame.", favor, fame);
	}

	//------------------------------------------------------------------------------------------------
	//! A job off the stronghold's books: the danger, said plainly.
	static string RogueWarning(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "This isn't on Command's books. If anyone at the stronghold asks, you never heard it from me.";
			case RZ_EFaction.BANDITS: return "The boss doesn't know about this one, and the boss doesn't like surprises. Keep it quiet.";
			case RZ_EFaction.INSTITUTE: return "This isn't in the director's log. If it gets out, it ends both our careers.";
		}

		return "The council doesn't know about this one. If they hear, it's my seat, and you'll be the one they blame.";
	}

	//------------------------------------------------------------------------------------------------
	//! A job that someone else will hold against you; %1 is who.
	static string RivalWarning(RZ_EFaction faction)
	{
		if (faction == RZ_EFaction.BANDITS)
			return "%1 won't like it. That's half the fun.";

		return "%1 won't thank you for it, and they'll remember your face.";
	}

	//------------------------------------------------------------------------------------------------
	//! Your background suits the job.
	static string BonusLine(RZ_EFaction faction)
	{
		if (faction == RZ_EFaction.MILITARY)
			return "Your background fits the task. That counts for extra on the roster.";

		return "Someone with your background is exactly who this needs. It'll count for more.";
	}

	//------------------------------------------------------------------------------------------------
	//! You said yes to a pitched job.
	static string Agreed(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "Thank you. Really. Come back in one piece, that's the main thing.";
			case RZ_EFaction.MILITARY: return "Good. You're on it as of now. Report back when it's done.";
			case RZ_EFaction.BANDITS: return "Knew you would. Don't make me regret the book entry.";
			case RZ_EFaction.INSTITUTE: return "Good. I've noted it. Bring back exactly what was asked for.";
		}

		return "Good. Come back when it's done.";
	}

	//------------------------------------------------------------------------------------------------
	//! You said no to a pitched job.
	static string Declined(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "That's all right. I'll ask someone else. The offer stands while it lasts.";
			case RZ_EFaction.MILITARY: return "Understood. It stays on my desk until someone takes it.";
			case RZ_EFaction.BANDITS: return "Your loss. Someone hungrier will take it by dark.";
			case RZ_EFaction.INSTITUTE: return "Noted. I'll find someone else. Don't take too long deciding if you change your mind.";
		}

		return "Suit yourself.";
	}

	//------------------------------------------------------------------------------------------------
	//! The job went (someone finished it, or it ran out) while you talked.
	static string Gone(RZ_EFaction faction)
	{
		if (faction == RZ_EFaction.BANDITS)
			return "Too slow. That one's gone. Ask me again later.";

		return "Ah. That's already been dealt with, while we were talking. Ask me again later.";
	}

	//------------------------------------------------------------------------------------------------
	//! Before the stronghold's contracts are listed (RZ_Leaders.OfferWork).
	static string ContractsIntro(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "Nothing of mine, but the council has a list. These are the jobs the houses need doing. They're yours if you want them:";
			case RZ_EFaction.MILITARY: return "Nothing from me. These are the stronghold's standing tasks. They're on your list now:";
			case RZ_EFaction.BANDITS: return "Not from me. The boss has a list, though. Here's what's paying:";
			case RZ_EFaction.INSTITUTE: return "I have nothing of my own. These are the lab's open requests. I've put your name against them:";
		}

		return "This is what the stronghold wants doing:";
	}

	//------------------------------------------------------------------------------------------------
	//! Nothing for you today.
	static string NoWork(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "Nothing today, love. The jobs that matter got taken at breakfast. Come back tomorrow.";
			case RZ_EFaction.MILITARY: return "Nothing on the board for you today. Check back tomorrow, or the day after.";
			case RZ_EFaction.BANDITS: return "Nothing worth your time today. Come back when the book's fuller.";
			case RZ_EFaction.INSTITUTE: return "There's nothing outstanding. The lab's requests come in batches. Try again tomorrow.";
		}

		return "Nothing for you today. Come back tomorrow, or the day after.";
	}

	//------------------------------------------------------------------------------------------------
	//! One line about who they were before the gates closed.
	static string Backstory(RZ_Rng rng, RZ_EFaction faction, string place)
	{
		array<string> lines = {};
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				lines.Insert("Ran the bakery in %1 until the flour ran out. Since then, everything else.");
				lines.Insert("A primary school teacher, before. The register still gets taken every morning, and the gaps still get counted.");
				lines.Insert("Drove the bus between the villages. Every family on the route, by name, and every one that stopped answering.");
				lines.Insert("A retired nurse who came back to work on Day 2 and never went home.");
				lines.Insert("Kept the parish books in %1. Now it's the food ledger, kept stricter than the parish's ever was.");
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				lines.Insert("Logistics, before. Signed for every crate in the Zone, and still signing, every one of them correct.");
				lines.Insert("Came in with the first cordon detail. Was told it was a two-week posting.");
				lines.Insert("Reservist. A surveyor in civilian life, and everything still gets measured twice.");
				lines.Insert("Twenty years in. Buried more of the platoon than anyone else at the post, and read every name at the roll.");
				lines.Insert("On leave in %1 when the gates closed, and reported for duty anyway.");
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				lines.Insert("Sold used cars in %1. The trade hasn't changed much, apparently.");
				lines.Insert("Did four years for something nobody mentions. Got out on Day 1, when the guards left.");
				lines.Insert("Ran the card games behind the bowling alley. Same games now, higher stakes.");
				lines.Insert("A bailiff, before. Repossessing things is still the job; the paperwork is simpler.");
				lines.Insert("Nobody knows. Ask and you get a different story every time.");
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				lines.Insert("Wrote the site's original safety assessment. Signed it, too.");
				lines.Insert("A virologist on a six-month contract. The contract ended on Day 9.");
				lines.Insert("Ran the sample freezers, and knew exactly what was in them before the lockdown.");
				lines.Insert("A field epidemiologist who has seen two outbreaks before this one, and none like it.");
				lines.Insert("Was the junior on the team. Most of the senior staff are gone.");
				break;
			}
		}

		return string.Format(rng.Pick(lines), place);
	}

	// ================================================================ Contract voice
	//------------------------------------------------------------------------------------------------
	//! Before the brief, by trait.
	static string Intro(RZ_Rng rng, int trait)
	{
		array<string> lines = {};
		switch (trait)
		{
			case RZ_ELeaderTrait.GREEDY: lines.Insert("Let's talk about what it pays first. Then the job."); lines.Insert("Everything costs something now. This costs you a walk."); break;
			case RZ_ELeaderTrait.PARANOID: lines.Insert("Close the door. Who saw you come in?"); lines.Insert("I'll say this once, and I'll deny it after."); break;
			case RZ_ELeaderTrait.MERCIFUL: lines.Insert("Sit down first. You look like you haven't eaten."); lines.Insert("Nobody needs to get hurt for this one."); break;
			case RZ_ELeaderTrait.RUTHLESS: lines.Insert("I don't care how. I care that it's done."); lines.Insert("This one isn't pretty. That's why I'm asking you."); break;
			case RZ_ELeaderTrait.IDEALIST: lines.Insert("This is how we get through this. One job at a time."); lines.Insert("It's small. It's also the right thing."); break;
			case RZ_ELeaderTrait.PRAGMATIST: lines.Insert("I don't like who we're dealing with. I like starving less."); lines.Insert("Everybody's somebody's rival. Business is business."); break;
			case RZ_ELeaderTrait.VETERAN: lines.Insert("Listen carefully. I've done this before, and I've buried people who didn't."); lines.Insert("Here's the plan. Here's what goes wrong with it."); break;
			case RZ_ELeaderTrait.TALKER: lines.Insert("You'll have heard the rumours. Here's the truth, more or less."); lines.Insert("Funny story, this. Well. Not funny."); break;
		}

		return rng.Pick(lines);
	}

	//------------------------------------------------------------------------------------------------
	//! After the brief, by trait.
	static string Outro(RZ_Rng rng, int trait)
	{
		array<string> lines = {};
		switch (trait)
		{
			case RZ_ELeaderTrait.GREEDY: lines.Insert("Paid on delivery. Not before."); lines.Insert("Don't haggle. I've heard it all."); break;
			case RZ_ELeaderTrait.PARANOID: lines.Insert("If anyone asks, you were never here."); lines.Insert("Watch who follows you back."); break;
			case RZ_ELeaderTrait.MERCIFUL: lines.Insert("If it goes wrong, come back anyway. We'll work it out."); lines.Insert("Mistakes happen. Come back alive."); break;
			case RZ_ELeaderTrait.RUTHLESS: lines.Insert("Don't come back with excuses."); lines.Insert("If someone gets in the way, that was their choice."); break;
			case RZ_ELeaderTrait.IDEALIST: lines.Insert("Remember why we're doing this."); lines.Insert("When this is over, people will ask what we did. Give them a good answer."); break;
			case RZ_ELeaderTrait.PRAGMATIST: lines.Insert("Nobody has to love anybody. They just have to deliver."); lines.Insert("Keep it civil. We'll need them again."); break;
			case RZ_ELeaderTrait.VETERAN: lines.Insert("Check your kit twice. Then go."); lines.Insert("No heroics. Heroes don't get paid."); break;
			case RZ_ELeaderTrait.TALKER: lines.Insert("And if you hear anything on the way, I want to know."); lines.Insert("Tell me everything after. Everything."); break;
		}

		return rng.Pick(lines);
	}

	//------------------------------------------------------------------------------------------------
	//! The faction's own way of ending a conversation.
	static string SignOff(RZ_Rng rng, RZ_EFaction faction)
	{
		array<string> lines = {};
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: lines.Insert("Be careful out there. Please."); lines.Insert("We'll keep a plate for you."); break;
			case RZ_EFaction.MILITARY: lines.Insert("That's all. Dismissed."); lines.Insert("Questions? No. Good."); break;
			case RZ_EFaction.BANDITS: lines.Insert("Don't die. You owe me."); lines.Insert("Off you go, sunshine."); break;
			case RZ_EFaction.INSTITUTE: lines.Insert("Precision matters. Please be precise."); lines.Insert("Thank you. I mean that."); break;
		}

		return rng.Pick(lines);
	}

	// ================================================================ Conversation
	//------------------------------------------------------------------------------------------------
	//! First contact: who the faction is, in its own words.
	static string WhoWeAre(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "We're the Haven: just people who were here when the gates closed. Every house keeps the same rules: everybody works, you declare a bite, and nobody fires a gun near the house.";
			case RZ_EFaction.MILITARY: return "Task Force Bastion, what's left of it. We call Command every night, and every night the log says no reply. So we feed our own and bring our own home. Stay on the right side of that.";
			case RZ_EFaction.BANDITS: return "The Red Hand. We noticed first that nobody was coming, and nothing taken would ever be given back. Tolls, a market, and a book with everyone's debts in it. Don't look so worried. Yet.";
			case RZ_EFaction.INSTITUTE:
			{
				// After the cure the proof is out: the gates wait on the outside now
				if (RZ_WorldFacts.Get().IsCured())
					return "Halcyon. We were studying the site in the hills before the lockdown, and we ran the only lab. The proof went out with the cure. Now we wait for the gates, like everyone else.";

				return "Halcyon. We were studying the site in the hills before the lockdown, and we run the only lab. Nobody opens the gates without proof the Zone is clean. We're the only ones who can make it.";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! What `speaker` says about `other`, in the speaker's voice. After the cure the lines about
	//! samples and the cure as a hope have their own words.
	static string HintAbout(RZ_EFaction speaker, RZ_EFaction other)
	{
		if (RZ_WorldFacts.Get().IsCured())
		{
			string after = HintAfterCure(speaker, other);
			if (!after.IsEmpty())
				return after;
		}

		switch (speaker)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (other == RZ_EFaction.MILITARY) return "Bastion keeps the roads, when it remembers to. Follow their rules and they'll protect you. Their rules change.";
				if (other == RZ_EFaction.BANDITS) return "The Red Hand will smile at you and count your boots. Don't trade with them unless you have to. Some of us have to.";
				if (other == RZ_EFaction.INSTITUTE) return "The scientists say they can end this. Bring them samples from the changed ones. It's the only real hope anyone's offered.";
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				if (other == RZ_EFaction.SURVIVORS) return "The Haven runs the safehouses. Civilians. Feed them and they'll vouch for you, which is worth more than you'd think.";
				if (other == RZ_EFaction.BANDITS) return "Red Hand. Looters with a logo. If you deal with them, we'll know.";
				if (other == RZ_EFaction.INSTITUTE) return "Halcyon has the lab. Command wanted it secured. Command isn't answering, so they secure it themselves.";
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				if (other == RZ_EFaction.SURVIVORS) return "The Haven? Lovely people. Lots of tins. Terrible locks.";
				if (other == RZ_EFaction.MILITARY) return "Bastion? The one crew we can't buy a name from. Stay off their roads, or pay toll on ours.";
				if (other == RZ_EFaction.INSTITUTE) return "The eggheads pay in hope. We pay in cash. Samples sell both ways, mind.";
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (other == RZ_EFaction.SURVIVORS) return "The Haven keeps people alive long enough to matter. Their medics are better than they have any right to be.";
				if (other == RZ_EFaction.MILITARY) return "Bastion would like the cure in a locked crate. We would like it in people. Be careful which of us you help.";
				if (other == RZ_EFaction.BANDITS) return "The Red Hand buys samples. Every one they buy is research we lose. I'd ask you not to. I can't stop you.";
				break;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue's lines for the hints that spoke of samples or of the cure to come. "" keeps
	//! the usual line.
	protected static string HintAfterCure(RZ_EFaction speaker, RZ_EFaction other)
	{
		if (speaker == RZ_EFaction.SURVIVORS && other == RZ_EFaction.INSTITUTE)
			return "The scientists did it. The cure's out. Now we just have to still be here when the gates open.";

		if (speaker == RZ_EFaction.BANDITS && other == RZ_EFaction.INSTITUTE)
			return "The cure's out. Nobody's buying samples now, not even us. The eggheads finally paid in something real.";

		if (speaker == RZ_EFaction.INSTITUTE && other == RZ_EFaction.BANDITS)
			return "The Red Hand used to buy samples. It doesn't matter now. Nothing they took can take the cure back.";

		// Who got the cure (the season's winner) decides which of these is true
		if (speaker == RZ_EFaction.INSTITUTE && other == RZ_EFaction.MILITARY)
		{
			RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
			int winner = -1;
			if (gm)
				winner = gm.GetSeasonWinner();

			if (winner == RZ_EFaction.MILITARY)
				return "Bastion has the cure in its locked crate, as it wanted. We wrote the method. They decide who gets it. We're told that's temporary.";

			if (winner == RZ_EFaction.INSTITUTE)
				return "Bastion wanted the cure in a locked crate. It went out over every frequency instead. They're still deciding how they feel about that.";

			return "Bastion wanted the cure in a locked crate. It didn't end up in one. They're still deciding how they feel about that.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The voice bank's "greeting" lines for the faction (Factions/RZ_VoiceDefaults.c), which read the
	//! season mood and joke memory.
	static string Greeting(RZ_Rng rng, RZ_EFaction faction)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
		{
			string line = RZ_VoiceBank.PickFor(gm, "greeting", faction, "", null, 0, vector.Zero);
			if (!line.IsEmpty())
				return line;
		}

		return "Yes? What do you need?";
	}

	//------------------------------------------------------------------------------------------------
	static string Refuse(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "I'm sorry. Not you. Not after what you did.";
			case RZ_EFaction.MILITARY: return "You're on a list. Turn around.";
			case RZ_EFaction.BANDITS: return "Oh, it's you. No. Walk.";
			case RZ_EFaction.INSTITUTE: return "We've nothing to discuss. Please leave before the guards decide that for me.";
		}

		return "No. We're done talking.";
	}

	//------------------------------------------------------------------------------------------------
	//! How they feel about you. attitude: see RZ_Leaders.Attitude.
	static string Opinion(int attitude)
	{
		if (attitude >= 3)
			return "You're one of the good ones. Don't make me regret saying so.";
		if (attitude >= 1)
			return "I know your name. That's a start.";
		if (attitude == 0)
			return "I don't know you yet.";

		return "I've heard things about you. Prove them wrong.";
	}

	//------------------------------------------------------------------------------------------------
	//! Said only to people they trust: where they stand with their own stronghold.
	static string OwnStand(RZ_EFaction faction, RZ_ELoyalty loyalty)
	{
		if (loyalty == RZ_ELoyalty.INDEPENDENT)
		{
			switch (faction)
			{
				case RZ_EFaction.SURVIVORS: return "The council in town sends its decisions. I make the ones that keep people here fed.";
				case RZ_EFaction.MILITARY: return "Orders come down. I follow the ones that make sense out here.";
				case RZ_EFaction.BANDITS: return "The boss gets a cut. The boss doesn't get a say.";
				case RZ_EFaction.INSTITUTE: return "The director has priorities. So do the people in front of me.";
			}
		}

		if (loyalty == RZ_ELoyalty.ROGUE)
		{
			switch (faction)
			{
				case RZ_EFaction.SURVIVORS: return "What the council doesn't know keeps us alive. I'll answer for it later.";
				case RZ_EFaction.MILITARY: return "Command left. I'm still here. Work it out.";
				case RZ_EFaction.BANDITS: return "The boss is getting old. Everyone can see it but the boss.";
				case RZ_EFaction.INSTITUTE: return "The director would call some of what I do unethical. The director would be right. People are dying in the meantime.";
			}
		}

		return "";
	}
}
