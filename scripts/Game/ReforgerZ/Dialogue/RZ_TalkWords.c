//! What you say on the talk screen (docs/WRITING.md, "Talk"; docs/DIALOGUE.md, "The talk screen").
//! A built reply's label is an action name ("Ask for work", "Trade", "Share some food"): the world
//! prompt and the object menu still use it, and RZ_TalkNeeds reads it for what a reply needs. On the
//! talk screen the reply is what you'd actually say, in your own words, to this person: Spoken turns
//! the label into that, after RZ_TalkNeeds has read it. A check or cost written after a label
//! ("Fix it [Mechanic]") goes into the reply's tag, before the words. Labels with no entry here are
//! shown as they are (a tree's choices are written as speech in the .rzd files).
//! Also: Unquote, used by the talk screen and the subtitles, so no spoken line shows in quotes.

//------------------------------------------------------------------------------------------------
class RZ_TalkWords
{
	//------------------------------------------------------------------------------------------------
	//! A built reply on the talk screen: its label becomes what you say to this person; a bracketed
	//! check after the words becomes its tag. The screen's own rows and the admins' are left alone.
	static void Apply(RZ_TalkSession s, RZ_TalkReply r)
	{
		if (!r || r.m_bOwn || r.m_bAdmin)
			return;

		string label = r.m_sLabel.Trim();
		int open = label.IndexOf(" [");
		if (open > 0 && label.EndsWith("]"))
		{
			string tags = label.Substring(open + 1, label.Length() - open - 1);
			label = label.Substring(0, open).Trim();
			// A kind tag already set ([Errand]) stays in front of the label's own
			if (r.m_sTag.StartsWith("["))
				tags = r.m_sTag + " " + tags;
			else if (!r.m_sTag.IsEmpty())
				tags += " [" + r.m_sTag + "]";

			r.m_sTag = tags;
		}

		int faction = -1;
		if (s)
			faction = s.m_iFaction;

		if (r.m_bWay)
			r.m_sLabel = Agree(label);
		else
			r.m_sLabel = Unquote(Spoken(label, faction));
	}

	//------------------------------------------------------------------------------------------------
	//! An errand's way said as what you agree to: "Fetch the cleaning kits" is "I'll fetch the
	//! cleaning kits.", "Say you couldn't find them" is "I'll say I couldn't find them."; its "Not
	//! now" is a polite no.
	static string Agree(string label)
	{
		string t = label.Trim();
		if (t == "Not now")
			return "Not now. Ask me again another time.";

		if (t.EndsWith(".") || t.EndsWith("?") || t.EndsWith("!"))
			return t;

		array<string> words = {};
		t.Split(" ", words, true);
		string result = "I'll";
		if (!words.IsEmpty() && words[0] == "Don't")
		{
			result = "I won't";
			words.RemoveOrdered(0);
		}

		foreach (int i, string w : words)
		{
			string word = Person(w);
			if (i == 0 && word.Length() > 1)
			{
				string first = word.Substring(0, 1);
				string second = word.Substring(1, 1);
				string secondLow = second;
				secondLow.ToLower();
				if (second == secondLow)
				{
					first.ToLower();
					word = first + word.Substring(1, word.Length() - 1);
				}
			}

			result += " " + word;
		}

		return result + ".";
	}

	//------------------------------------------------------------------------------------------------
	//! A way's "you" is the one who says it: you -> I, your -> my.
	protected static string Person(string word)
	{
		switch (word)
		{
			case "you": return "I";
			case "your": return "my";
			case "yourself": return "myself";
			case "you'll": return "I'll";
			case "you're": return "I'm";
			case "you've": return "I've";
			case "you'd": return "I'd";
		}

		return word;
	}

	//------------------------------------------------------------------------------------------------
	//! The way out, said to this person: varied by who they are.
	static string WayOut(RZ_TalkSession s)
	{
		if (!s)
			return "I'll be going.";

		if (RZ_TalkStranger.Cast(s.m_Adapter))
			return "Look after yourself. I have to go.";

		switch (s.m_iFaction)
		{
			case RZ_EFaction.SURVIVORS: return "I'll let you get on.";
			case RZ_EFaction.MILITARY: return "That's all for now.";
			case RZ_EFaction.BANDITS: return "I'll be on my way.";
			case RZ_EFaction.INSTITUTE: return "I won't keep you from your work.";
		}

		return "I'll be going.";
	}

	//------------------------------------------------------------------------------------------------
	//! What you say for a built label to someone of `faction` (-1: nobody's: wanderers, strangers, the
	//! Long Table). The label itself when there's no entry.
	static string Spoken(string label, int faction)
	{
		string leader = SayLeader(label, faction);
		if (!leader.IsEmpty())
			return leader;

		string staff = SayStaff(label, faction);
		if (!staff.IsEmpty())
			return staff;

		string road = SayRoad(label);
		if (!road.IsEmpty())
			return road;

		string stranger = SayStranger(label);
		if (!stranger.IsEmpty())
			return stranger;

		// Built from a name: "Rob <name>", the majors' "Ask about <title>"
		if (label.StartsWith("Rob "))
			return "Hand over the bag. Slowly, and nobody gets hurt.";

		if (label.StartsWith("Ask about "))
			return "What do you hear about " + label.Substring(10, label.Length() - 10) + "?";

		if (label.StartsWith("Send them to "))
			return "Go to " + label.Substring(13, label.Length() - 13) + ". Tell them the truth about that arm.";

		return label;
	}

	//------------------------------------------------------------------------------------------------
	//! A leader's asks (RZ_TalkLeader) and the epilogue week's (RZ_Epilogue.LeaderReplies).
	protected static string SayLeader(string label, int faction)
	{
		switch (label)
		{
			case "Ask for work": return SayWork(faction);
			case "Heard anything?": return SayNews(faction);
			case "What do you make of me?": return "Be honest. What do you make of me?";
			case "About the season": return "The season's over. What do you make of it?";
			case "Where should I look?": return "Where should I be looking now?";
			case "The cure": return "What happens with the cure now?";
			case "This place": return "What happens to this place now?";
			case "Me": return "And what happens to me?";
			case "The dead": return "Tell me about the ones we lost.";
			case "What you didn't tell me": return "What didn't you tell me?";
			case "Settle accounts": return "Let's settle what's between us.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static string SayWork(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "Is there anything the house needs doing?";
			case RZ_EFaction.MILITARY: return "Got any work for me?";
			case RZ_EFaction.BANDITS: return "Got a job going? I'm not fussy.";
			case RZ_EFaction.INSTITUTE: return "Is there anything the lab needs fetched?";
		}

		return "Got anything that needs carrying?";	// the Courier
	}

	//------------------------------------------------------------------------------------------------
	protected static string SayNews(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "What are people saying in the houses?";
			case RZ_EFaction.MILITARY: return "What's in the morning report?";
			case RZ_EFaction.BANDITS: return "What's going round?";
			case RZ_EFaction.INSTITUTE: return "What have your radio logs picked up?";
		}

		return "Heard anything worth knowing?";
	}

	//------------------------------------------------------------------------------------------------
	protected static string SayTrade(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "What's in the stores today?";
			case RZ_EFaction.MILITARY: return "I need to draw from stores.";
			case RZ_EFaction.BANDITS: return "Show me what you've got.";
			case RZ_EFaction.INSTITUTE: return "What can I get for my Favor?";
		}

		return "Show me what you've got.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string SayInCharge(int faction)
	{
		if (faction == RZ_EFaction.MILITARY)
			return "I need to speak to whoever's in command.";

		if (faction == RZ_EFaction.BANDITS)
			return "Where's the boss? I'll talk to the boss.";

		return "Who's in charge here? I'd like a word.";
	}

	//------------------------------------------------------------------------------------------------
	//! A quartermaster's and a safehouse's (RZ_TalkStaff), and the occupants' and named people's.
	protected static string SayStaff(string label, int faction)
	{
		switch (label)
		{
			case "Trade": return SayTrade(faction);
			case "Ask for whoever's in charge": return SayInCharge(faction);
			case "Ask for the trial dose": return "I'm bitten. I'm asking for the trial dose.";
			case "Ask how things are": return "How are things here?";
			case "Anything need doing?": return "Anything need doing?";
			case "Hand over food": return "I've brought food for the house.";
			case "Ask the medic to patch you up": return "Can you patch me up?";
			case "Tell them you've been bitten": return "I have to tell you something. I've been bitten.";
			case "Listen to the safehouse radio": return "Mind if I sit by the radio a while?";
			case "Ask for something for the fever": return "Have you got anything for the fever?";
			case "Read the notice board": return "What's on the board today?";
			case "Leave a note on the board": return "I want to pin a note on the board.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The wanderers, the Long Table's people and an opening's contact.
	protected static string SayRoad(string label)
	{
		switch (label)
		{
			case "Buy supplies": return "What are you selling?";
			case "Ask for treatment": return "Could you see to me? I'm not in good shape.";
			case "Buy a rumour": return "What do you know that's worth paying for?";
			case "Ask about Bastion": return "You were Bastion. What's it really like in there?";
			case "Ask him to stop ringing the bells": return "Those bells bring them in. Could you stop ringing them?";
			case "Show the kid the way to a safehouse": return "Come on. I know the way to a safehouse.";
			case "Buy a car part": return "Got any car parts going?";
			case "Share some food": return "Here. Have something to eat.";
			case "Give them your antidote": return "Here. Take my antidote. Quickly.";
			case "Ask about cars": return "Know of any cars round here that still run?";
			case "Need a hand?": return "You look like you could use a hand.";
			case "Sit down and eat": return "Thank you. I'll sit down and eat.";
			case "Trade for a jar": return "Could I trade you for one of those jars?";
			case "Trade for a jar (ammunition or medicine)": return "Could I trade you for one of those jars?";
			case "Tell them about your bite": return "I should tell you. I've been bitten.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The strangers' ways (Missions/Errands/RZ_StrangerKinds.c), as you'd say them to the stranger.
	protected static string SayStranger(string label)
	{
		switch (label)
		{
			// Someone on a roof; someone hiding under a car
			case "Talk them down": return "They've gone. Come down slowly, I've got you.";
			case "Talk them out": return "It's over. You can come out now. It's only me.";
			case "Leave them": return "I can't stay. Keep your head down.";
			// A tin for his girl
			case "Give him a tin": return "Here, take a tin for her.";
			case "Trade two tins for his watch": return "Two tins for your watch. That's a fair trade.";
			case "Not today": return "Not today. I'm sorry.";
			// His wife's car
			case "Push it into the ditch": return "Come on, I'll help you push it off the road.";
			case "Look under the bonnet": return "Let me look under the bonnet first.";
			case "Leave him to it": return "I'll leave you to it.";
			// Which way to town
			case "Point her round it": return "Go round by the fields. It's longer, but it's quiet.";
			case "Walk her past it": return "Stay close. I'll walk you past it.";
			case "Tell her the toll's cheap": return "The toll on the straight road is cheap. Just pay it.";
			// Behind a door since Tuesday
			case "Walk them out now": return "Get your things. I'll walk you out now.";
			case "Come back for them at dawn": return "Not in the dark. I'll come back for you at dawn.";
			case "Leave them some food": return "I can't take you. Here, have some food.";
			// Two kids in the back seat
			case "Fix it": return "Let me have a look at the engine.";
			case "Give them a battery": return "Here, take my battery. Try it now.";
			case "Push-start it downhill": return "Let's push it down the hill and bump it.";
			// A woman digging by the verge
			case "Help her dig": return "Give me the shovel. I'll take a turn.";
			case "Tell her who it was": return "I know who that was. I'll tell you.";
			case "Take the dead's things": return "Whatever's in their pockets is mine.";
			// The man who's heard of a shot at the lab
			case "Point him to the lab": return "It's true. The lab's that way.";
			case "Walk him toward it": return "I'll walk you part of the way.";
			case "Sit with him till dark": return "Sit down. I'll stay with you till dark.";
			// A man in the dark, cold
			case "Let him sit": return "Sit. Warm your hands. Nobody's armed here.";
			case "Send him off": return "Keep walking. Not here.";
			// The seed potatoes
			case "Dig it up for him": return "I'll dig your sack up for you.";
			case "Say it was gone": return "I looked. Somebody got there first.";
			case "Not my business": return "That's not my business.";
			// The grandfather's bundle at the post
			case "Say they're with you": return "They're with me. I'll vouch for them.";
			case "Walk them round the post": return "Come with me. We'll go round the post.";
			case "Hand the bundle in at the barrier": return "Let them search it. It's only a bundle.";
			// The sister put out of a house
			case "Find her and tell him": return "I'll find your sister and tell you where she is.";
			case "Warn her on to another house": return "I'll find her, and I'll tell her to keep moving.";
			case "Call him out": return "That's a red hand on your wrist. Who are you really?";
			// On the wrong side of the toll
			case "Pay her toll": return "I'll pay your toll.";
			case "Say she's with you": return "She's with me. Let her through.";
			case "Walk her round the toll": return "Come on. I know a way round the toll.";
			case "Sell her to the toll": return "Wait here. I'll talk to the toll for you.";
			// Waving at the line
			case "Talk her back": return "Come back from the line. Come on, step back with me.";
			case "Walk out there with her": return "I'll walk out there with you.";
			case "Leave her be": return "I'll leave you be.";
			// The face on the notice
			case "Take the book, unseen": return "I'll get your page out of that book. Nobody sees me.";
			case "Burn the hut's books": return "I'll burn every book in that hut.";
			case "Turn him down": return "No. Find someone else.";
			// First at the loot
			case "Share what's left": return "We'll split what's left. Fair?";
			case "Trade a tin for something": return "I'll trade you a tin for something in that bag.";
			case "Leave it to them": return "It's yours. You got here first.";
			// A scratch off a fence (Send them to <house>: below, by its prefix)
			case "Look at the arm": return "Let me look at that arm.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A spoken line without quotation marks around it: a leading and trailing pair of straight or
	//! curly quotes is taken off (also after a "Name: " in front, which is kept). A line with quotes
	//! inside it (a quote within the speech, narration around it) is left as it is.
	static string Unquote(string line)
	{
		string t = line.Trim();
		string head = "";
		int colon = t.IndexOf(": ");
		if (colon > 0 && colon <= 40)
		{
			string rest = t.Substring(colon + 2, t.Length() - colon - 2);
			if (QuoteAt(rest, true) > 0)
			{
				head = t.Substring(0, colon + 2);
				t = rest;
			}
		}

		int lead = QuoteAt(t, true);
		int tail = QuoteAt(t, false);
		if (lead <= 0 || tail <= 0 || t.Length() <= lead + tail)
			return line;

		string inner = t.Substring(lead, t.Length() - lead - tail);
		if (inner.Contains("\"") || inner.Contains("“") || inner.Contains("”"))
			return line;

		return head + inner.Trim();
	}

	//------------------------------------------------------------------------------------------------
	//! The length of a quotation mark at the start (or the end) of `text`, 0 if none.
	protected static int QuoteAt(string text, bool start)
	{
		array<string> marks = {"\"", "“", "”"};
		foreach (string mark : marks)
		{
			if (start && text.StartsWith(mark))
				return mark.Length();

			if (!start && text.EndsWith(mark))
				return mark.Length();
		}

		return 0;
	}
}
