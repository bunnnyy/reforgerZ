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
	//! One line about who they were before the gates closed.
	static string Backstory(RZ_Rng rng, RZ_EFaction faction, string place)
	{
		array<string> lines = {};
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				lines.Insert("Ran the bakery in %1 until the flour ran out. Now runs everything else.");
				lines.Insert("A primary school teacher. Still takes the register every morning, and still counts the gaps.");
				lines.Insert("Drove the bus between the villages. Knows every family on the route, and which ones stopped answering.");
				lines.Insert("A retired nurse who came back to work on Day 2 and never went home.");
				lines.Insert("Kept the parish books in %1. Now keeps the food ledger, and is stricter about it.");
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				lines.Insert("Logistics, before. Signed for every crate in the Zone. Still does, and signs it correct.");
				lines.Insert("Came in with the first cordon detail. Was told it was a two-week posting.");
				lines.Insert("Reservist. Was a surveyor in civilian life and still measures everything twice.");
				lines.Insert("Twenty years in. Has buried more of the platoon than the outbreak has.");
				lines.Insert("Was on leave in %1 when the gates closed, and reported for duty anyway.");
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				lines.Insert("Sold used cars in %1. Says the trade hasn't changed much.");
				lines.Insert("Did four years for something nobody mentions. Got out on Day 1, when the guards left.");
				lines.Insert("Ran the card games behind the bowling alley. Still runs them, with higher stakes.");
				lines.Insert("A bailiff, before. Repossessing things is still the job; the paperwork is simpler.");
				lines.Insert("Nobody knows. Ask and you get a different story every time.");
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				lines.Insert("Wrote the site's original safety assessment. Signed it, too.");
				lines.Insert("A virologist on a six-month contract. The contract ended on Day 9.");
				lines.Insert("Ran the sample freezers. Knows exactly what was in them before the lockdown.");
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
			case RZ_EFaction.SURVIVORS: return "\"We're the Haven. Just people who were here when the gates closed. We feed who we can, and nobody shoots in the house.\"";
			case RZ_EFaction.MILITARY: return "\"Task Force Bastion. What's left of it. Nobody's coming for us, so we look after our own. Stay on the right side of that.\"";
			case RZ_EFaction.BANDITS: return "\"The Red Hand. We're the ones who noticed the rules stopped. Don't look so worried. Yet.\"";
			case RZ_EFaction.INSTITUTE:
			{
				// After the cure the proof is out: the gates wait on the outside now
				if (RZ_WorldFacts.Get().IsCured())
					return "\"Halcyon. We were studying the site before the lockdown. The proof went out with the cure. Now we wait for the gates, like everyone else.\"";

				return "\"Halcyon. We were studying the site before the lockdown. We're the only ones who can prove the Zone is clean, and nobody outside opens the gates without proof.\"";
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
				if (other == RZ_EFaction.MILITARY) return "\"Bastion keeps the roads, when it remembers to. Follow their rules and they'll protect you. Their rules change.\"";
				if (other == RZ_EFaction.BANDITS) return "\"The Red Hand will smile at you and count your boots. Don't trade with them unless you have to. Some of us have to.\"";
				if (other == RZ_EFaction.INSTITUTE) return "\"The scientists say they can end this. Bring them samples from the changed ones. It's the only real hope anyone's offered.\"";
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				if (other == RZ_EFaction.SURVIVORS) return "\"The Haven runs the safehouses. Civilians. Feed them and they'll vouch for you, which is worth more than you'd think.\"";
				if (other == RZ_EFaction.BANDITS) return "\"Red Hand. Looters with a logo. If you deal with them, we'll know.\"";
				if (other == RZ_EFaction.INSTITUTE) return "\"Halcyon has the lab. Command wanted it secured. Command isn't answering, so they secure it themselves.\"";
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				if (other == RZ_EFaction.SURVIVORS) return "\"The Haven? Lovely people. Lots of tins. Terrible locks.\"";
				if (other == RZ_EFaction.MILITARY) return "\"Bastion? The one crew we can't buy a name from. Stay off their roads, or pay toll on ours.\"";
				if (other == RZ_EFaction.INSTITUTE) return "\"The eggheads pay in hope. We pay in cash. Samples sell both ways, mind.\"";
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (other == RZ_EFaction.SURVIVORS) return "\"The Haven keeps people alive long enough to matter. Their medics are better than they have any right to be.\"";
				if (other == RZ_EFaction.MILITARY) return "\"Bastion would like the cure in a locked crate. We would like it in people. Be careful which of us you help.\"";
				if (other == RZ_EFaction.BANDITS) return "\"The Red Hand buys samples. Every one they buy is research we lose. I'd ask you not to. I can't stop you.\"";
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
			return "\"The scientists did it. The cure's out. Now we just have to still be here when the gates open.\"";

		if (speaker == RZ_EFaction.BANDITS && other == RZ_EFaction.INSTITUTE)
			return "\"The cure's out. Nobody's buying samples now, not even us. The eggheads finally paid in something real.\"";

		if (speaker == RZ_EFaction.INSTITUTE && other == RZ_EFaction.BANDITS)
			return "\"The Red Hand used to buy samples. It doesn't matter now. Nothing they took can take the cure back.\"";

		// Who got the cure (the season's winner) decides which of these is true
		if (speaker == RZ_EFaction.INSTITUTE && other == RZ_EFaction.MILITARY)
		{
			RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
			int winner = -1;
			if (gm)
				winner = gm.GetSeasonWinner();

			if (winner == RZ_EFaction.MILITARY)
				return "\"Bastion has the cure in its locked crate, as it wanted. We wrote the method. They decide who gets it. We're told that's temporary.\"";

			if (winner == RZ_EFaction.INSTITUTE)
				return "\"Bastion wanted the cure in a locked crate. It went out over every frequency instead. They're still deciding how they feel about that.\"";

			return "\"Bastion wanted the cure in a locked crate. It didn't end up in one. They're still deciding how they feel about that.\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string Greeting(RZ_Rng rng, RZ_EFaction faction)
	{
		array<string> lines = {};
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: lines.Insert("\"You're back. Good. Every time someone walks in, I count them.\""); lines.Insert("\"Come in out of the cold. Mind the children.\""); break;
			case RZ_EFaction.MILITARY: lines.Insert("\"State your business.\""); lines.Insert("\"You again. Fine. Make it quick.\""); break;
			case RZ_EFaction.BANDITS: lines.Insert("\"Look who it is. Pockets full, I hope.\""); lines.Insert("\"Ah. My favourite stranger.\""); break;
			case RZ_EFaction.INSTITUTE: lines.Insert("\"Don't touch anything labelled. Which is everything.\""); lines.Insert("\"Yes? I have a minute. Perhaps two.\""); break;
		}

		return rng.Pick(lines);
	}

	//------------------------------------------------------------------------------------------------
	static string Refuse(RZ_EFaction faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "\"I'm sorry. Not you. Not after what you did.\"";
			case RZ_EFaction.MILITARY: return "\"You're on a list. Turn around.\"";
			case RZ_EFaction.BANDITS: return "\"Oh, it's you. No. Walk.\"";
			case RZ_EFaction.INSTITUTE: return "\"We've nothing to discuss. Please leave before the guards decide that for me.\"";
		}

		return "\"No.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! How they feel about you. attitude: see RZ_Leaders.Attitude.
	static string Opinion(int attitude)
	{
		if (attitude >= 3)
			return "\"You're one of the good ones. Don't make me regret saying so.\"";
		if (attitude >= 1)
			return "\"I know your name. That's a start.\"";
		if (attitude == 0)
			return "\"I don't know you yet.\"";

		return "\"I've heard things about you. Prove them wrong.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! Said only to people they trust: where they stand with their own stronghold.
	static string OwnStand(RZ_EFaction faction, RZ_ELoyalty loyalty)
	{
		if (loyalty == RZ_ELoyalty.INDEPENDENT)
		{
			switch (faction)
			{
				case RZ_EFaction.SURVIVORS: return "\"The council in town sends its decisions. I make the ones that keep people here fed.\"";
				case RZ_EFaction.MILITARY: return "\"Orders come down. I follow the ones that make sense out here.\"";
				case RZ_EFaction.BANDITS: return "\"The boss gets a cut. The boss doesn't get a say.\"";
				case RZ_EFaction.INSTITUTE: return "\"The director has priorities. So do the people in front of me.\"";
			}
		}

		if (loyalty == RZ_ELoyalty.ROGUE)
		{
			switch (faction)
			{
				case RZ_EFaction.SURVIVORS: return "\"What the council doesn't know keeps us alive. I'll answer for it later.\"";
				case RZ_EFaction.MILITARY: return "\"Command left. I'm still here. Work it out.\"";
				case RZ_EFaction.BANDITS: return "\"The boss is getting old. Everyone can see it but the boss.\"";
				case RZ_EFaction.INSTITUTE: return "\"The director would call some of what I do unethical. The director would be right. People are dying in the meantime.\"";
			}
		}

		return "";
	}
}
