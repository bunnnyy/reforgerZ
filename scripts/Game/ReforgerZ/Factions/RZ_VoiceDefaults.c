//! The voice bank's code defaults (Factions/RZ_VoiceBank.c; docs/WRITING.md, "The voice bank"). One
//! Line() per line, so tools/voice_sim.py and the text lint can read them. Ids are stable: Voice >
//! Lines replaces a line by its id, Voice > Disabled switches it off. Moods: 1 Early, 2 Grim,
//! 4 Bleak, 8 Hope, 16 Epilogue (0 any).
class RZ_VoiceDefaults
{
	protected static const int HAVEN = 0;
	protected static const int BASTION = 1;
	protected static const int REDHAND = 2;
	protected static const int HALCYON = 3;

	//------------------------------------------------------------------------------------------------
	protected static void Line(array<ref RZ_VoiceLine> lines, string id, int faction, string kind, string manner, RZ_ERegister register, int moods, string when, string text)
	{
		RZ_VoiceLine l = RZ_VoiceLine.Cast(RZ_Util.NewWithDefaults("RZ_VoiceLine"));
		if (!l)
			return;

		l.m_sId = id;
		l.m_iFaction = faction;
		l.m_sKind = kind;
		l.m_sManner = manner;
		l.m_eRegister = register;
		l.m_iMoods = moods;
		l.m_sWhen = when;
		l.m_fWeight = 1;
		l.m_sText = text;
		lines.Insert(l);
	}

	//------------------------------------------------------------------------------------------------
	static void Fill(array<ref RZ_VoiceLine> lines)
	{
		Openers(lines);
		People(lines);
		Asides(lines);
	}

	//------------------------------------------------------------------------------------------------
	//! Openings: the built greeting for someone a leader has met (kind "greeting", RZ_LeaderVoice),
	//! and a leader's opening line by your standing (the manner), coloured by the mood. Said only with
	//! Dialogue > Trees on (RZ_RepVoice.Opening).
	protected static void Openers(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;
		RZ_ERegister B = RZ_ERegister.BLEAK;

		Line(lines, "haven.opener.unknown.1", HAVEN, "opener", "unknown", P, 0, "", "\"New face. Shoes off, rifle by the door, and you'll get soup.\"");
		Line(lines, "haven.opener.warm.1", HAVEN, "opener", "warm", P, 0, "", "\"You came back. That's half of it.\"");
		Line(lines, "haven.greeting.1", HAVEN, "greeting", "", P, 0, "", "\"You're back. Good. Every time someone walks in, I count them.\"");
		Line(lines, "haven.greeting.2", HAVEN, "greeting", "", P, 0, "", "\"Come in out of the cold. Mind the children.\"");
		Line(lines, "haven.opener.wary.1", HAVEN, "opener", "wary", P, 0, "", "\"You're welcome. Mostly. Leave the rifle by the door and we'll see.\"");
		Line(lines, "haven.opener.wary.2", HAVEN, "opener", "wary", W, 0, "", "\"The council voted on you. It was a draw. Sit near the door.\"");
		Line(lines, "haven.opener.fond.1", HAVEN, "opener", "fond", W, 0, "", "\"The council voted to like you. It was close.\"");
		Line(lines, "haven.opener.fond.2", HAVEN, "opener", "fond", P, 0, "", "\"There's a bowl. Eat first, then tell me what you broke.\"");
		Line(lines, "haven.opener.needed.1", HAVEN, "opener", "needed", W, 0, "", "\"Half the house wants to feed you. The other half's hiding the children. I'm doing both.\"");
		Line(lines, "haven.opener.needed.2", HAVEN, "opener", "needed", P, 0, "", "\"Whatever you did out there, we ask you don't do it here.\"");
		Line(lines, "haven.opener.devoted.1", HAVEN, "opener", "devoted", P, 0, "", "\"Sit. You're not queuing. Not you.\"");
		Line(lines, "haven.opener.hostile.1", HAVEN, "opener", "hostile", P, 0, "", "\"The door stays shut. That's all I'll say.\"");
		Line(lines, "haven.opener.bleak.1", HAVEN, "opener", "", B, 4, "", "\"Shut the door behind you. Quietly. People are sleeping who don't sleep much.\"");

		Line(lines, "bastion.opener.unknown.1", BASTION, "opener", "unknown", P, 0, "", "\"Civilian. State your business. Briefly.\"");
		Line(lines, "bastion.greeting.1", BASTION, "greeting", "", P, 0, "", "\"State your business.\"");
		Line(lines, "bastion.opener.warm.1", BASTION, "opener", "warm", P, 0, "", "\"Cleared. Keep moving.\"");
		Line(lines, "bastion.greeting.2", BASTION, "greeting", "", W, 0, "", "\"You again. Fine. Make it quick.\"");
		Line(lines, "bastion.opener.wary.1", BASTION, "opener", "wary", P, 0, "", "\"Uncleared. Stand there.\"");
		Line(lines, "bastion.opener.wary.2", BASTION, "opener", "wary", W, 0, "", "\"Your file's got two columns. Arms out.\"");
		Line(lines, "bastion.opener.fond.1", BASTION, "opener", "fond", W, 0, "", "\"You're on two lists. One of them's good. Arms out anyway.\"");
		Line(lines, "bastion.opener.needed.1", BASTION, "opener", "needed", J, 0, "", "\"Command would have you shot or decorated. Command isn't answering. Arms out.\"");
		Line(lines, "bastion.opener.needed.2", BASTION, "opener", "needed", W, 0, "", "\"Your file's thicker than the roster. Proceed.\"");
		Line(lines, "bastion.opener.needed.3", BASTION, "opener", "needed", P, 0, "", "\"We need you. Nobody said we had to like it. Pass.\"");
		Line(lines, "bastion.opener.devoted.1", BASTION, "opener", "devoted", P, 0, "", "\"The sergeant sends compliments. It's all there is to send.\"");
		Line(lines, "bastion.opener.hostile.1", BASTION, "opener", "hostile", P, 0, "", "\"You're on a list. Walk the other way.\"");
		Line(lines, "bastion.opener.grim.1", BASTION, "opener", "", P, 6, "", "\"Report. Keep it short. We're short of everything else too.\"");

		Line(lines, "redhand.opener.unknown.1", REDHAND, "opener", "unknown", W, 0, "", "\"Toll road, this. Or turn round. Your choice, your legs.\"");
		Line(lines, "redhand.opener.unknown.2", REDHAND, "opener", "unknown", P, 0, "", "\"Don't know you. That's not a compliment yet.\"");
		Line(lines, "redhand.opener.warm.1", REDHAND, "opener", "warm", P, 0, "", "\"Toll first, then smile. We're a business.\"");
		Line(lines, "redhand.greeting.1", REDHAND, "greeting", "", J, 0, "", "\"Look who it is. Pockets full, I hope.\"");
		Line(lines, "redhand.greeting.2", REDHAND, "greeting", "", W, 0, "", "\"Ah. My favourite stranger.\"");
		Line(lines, "redhand.opener.wary.1", REDHAND, "opener", "wary", W, 0, "", "\"We like you less than we should, and you owe us more than you think.\"");
		Line(lines, "redhand.opener.fond.1", REDHAND, "opener", "fond", J, 0, "", "\"Trouble. Useful trouble. That's practically a job title here.\"");
		Line(lines, "redhand.opener.needed.1", REDHAND, "opener", "needed", W, 0, "", "\"Danger money. Nothing personal.\"");
		Line(lines, "redhand.opener.needed.2", REDHAND, "opener", "needed", J, 0, "", "\"We hate you a normal amount. We need you a lot more.\"");
		Line(lines, "redhand.opener.devoted.1", REDHAND, "opener", "devoted", W, 0, "", "\"Make way. The boss's favourite, and the boss doesn't have favourites.\"");
		Line(lines, "redhand.opener.hostile.1", REDHAND, "opener", "hostile", P, 0, "", "\"Oh, it's you. No. Walk.\"");
		Line(lines, "redhand.opener.hope.1", REDHAND, "opener", "", J, 8, "", "\"Heard the lab's close. Prices are going up, just in case.\"");

		Line(lines, "halcyon.opener.unknown.1", HALCYON, "opener", "unknown", P, 0, "", "\"Visitor. You may wait outside the yellow line.\"");
		Line(lines, "halcyon.greeting.1", HALCYON, "greeting", "", P, 0, "", "\"Yes? I have a minute. Perhaps two.\"");
		Line(lines, "halcyon.opener.warm.1", HALCYON, "opener", "warm", P, 0, "", "\"Courier. Leave it on the table, not on me.\"");
		Line(lines, "halcyon.greeting.2", HALCYON, "greeting", "", W, 0, "", "\"Don't touch anything labelled. Which is everything.\"");
		Line(lines, "halcyon.opener.wary.1", HALCYON, "opener", "wary", P, 0, "", "\"Inconclusive. We'll keep watching.\"");
		Line(lines, "halcyon.opener.fond.1", HALCYON, "opener", "fond", W, 0, "", "\"You're productive and you're a problem. We have a column for each.\"");
		Line(lines, "halcyon.opener.needed.1", HALCYON, "opener", "needed", W, 0, "", "\"We have you down as unpredictable. It isn't a criticism. It's a category.\"");
		Line(lines, "halcyon.opener.needed.2", HALCYON, "opener", "needed", W, 0, "", "\"The director says the data justifies you. The director says that about a lot of things.\"");
		Line(lines, "halcyon.opener.devoted.1", HALCYON, "opener", "devoted", P, 0, "", "\"Your samples are in a result. That's rarer than it sounds.\"");
		Line(lines, "halcyon.opener.hostile.1", HALCYON, "opener", "hostile", P, 0, "", "\"We've nothing to discuss.\"");
		Line(lines, "halcyon.opener.hope.1", HALCYON, "opener", "", W, 8, "", "\"We may have something. Don't repeat that either.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! Named people at holdings (RZ_FactionVoice.PersonLine), moved here from code.
	protected static void People(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;

		Line(lines, "haven.person.1", HAVEN, "person", "", W, 0, "", "\"Wipe your feet. Not that it matters any more. Habit.\"");
		Line(lines, "haven.person.2", HAVEN, "person", "", W, 0, "", "\"We grow what we can. Potatoes, mostly. Everyone in {place} is sick of potatoes.\"");
		Line(lines, "haven.person.3", HAVEN, "person", "", P, 0, "", "\"If you see my brother on the roads, he's tall, he limps. Tell him we're still here.\"");
		Line(lines, "bastion.person.1", BASTION, "person", "", W, 0, "", "\"Move along, civilian. Nothing to see. That's an order, apparently.\"");
		Line(lines, "bastion.person.2", BASTION, "person", "", P, 0, "", "\"Every day, 'relief is on its way'. I've stopped counting. The sergeant hasn't.\"");
		Line(lines, "bastion.person.3", BASTION, "person", "", P, 0, "", "\"Keep your weapon slung near the post. Some of the lads are jumpy.\"");
		Line(lines, "redhand.person.1", REDHAND, "person", "", J, 0, "", "\"Nice boots. Joking. Mostly.\"");
		Line(lines, "redhand.person.2", REDHAND, "person", "", P, 0, "", "\"Toll's fair. Tins, fuel, or a story we haven't heard. We've heard most of them.\"");
		Line(lines, "redhand.person.3", REDHAND, "person", "", J, 0, "", "\"Bastion calls us bandits. We call it free enterprise. With knives.\"");
		Line(lines, "halcyon.person.1", HALCYON, "person", "", W, 0, "", "\"Don't touch anything labelled. Don't touch anything unlabelled either.\"");
		Line(lines, "halcyon.person.2", HALCYON, "person", "", P, 16, "", "\"It's done. I keep checking the numbers anyway. Nobody has asked me to stop.\"");
		Line(lines, "halcyon.person.3", HALCYON, "person", "", P, 15, "", "\"Bring us a sample from one of the big ones and you'll be more useful than half the people here.\"");
		Line(lines, "halcyon.person.4", HALCYON, "person", "", P, 15, "cure>=50", "\"We're close. We've been close for a while.\"");
		Line(lines, "halcyon.person.5", HALCYON, "person", "", P, 15, "cure<50", "\"We're nowhere near. Don't repeat that.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! What a person says on top at the evening meal, the night watch and shift change
	//! (RZ_FactionVoice.PhaseAside), moved here from code.
	protected static void Asides(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;

		Line(lines, "haven.aside.meal.1", HAVEN, "aside_meal", "", P, 0, "", "\"Sit, eat. There's always a bowl for whoever's at the door.\"");
		Line(lines, "bastion.aside.meal.1", BASTION, "aside_meal", "", W, 0, "", "\"Mess is open. Rations, same as yesterday. Help yourself, we're not checking.\"");
		Line(lines, "bastion.aside.meal.2", BASTION, "aside_meal", "", P, 0, "", "\"Mess is open. Rations. Eat standing, we're one short on the gate.\"");
		Line(lines, "redhand.aside.meal.1", REDHAND, "aside_meal", "", W, 0, "", "\"Dinner's on. Tins. Someone's written prices on them, so don't eat the expensive ones.\"");
		Line(lines, "redhand.aside.meal.2", REDHAND, "aside_meal", "", P, 0, "", "\"Dinner's on. Tins again. Grab one before the night crew does.\"");
		Line(lines, "halcyon.aside.meal.1", HALCYON, "aside_meal", "", P, 0, "", "\"We eat in shifts. Somebody always has to watch the incubators.\"");
		Line(lines, "haven.aside.night.1", HAVEN, "aside_night", "", P, 0, "", "\"Keep your voice down. People are sleeping.\"");
		Line(lines, "bastion.aside.night.1", BASTION, "aside_night", "", P, 0, "", "\"Night watch. Talk quietly and keep your hands where I can see them.\"");
		Line(lines, "redhand.aside.night.1", REDHAND, "aside_night", "", P, 0, "", "\"Night's when the money moves. Don't stand in the road.\"");
		Line(lines, "halcyon.aside.night.1", HALCYON, "aside_night", "", P, 0, "", "\"The night readings are the clean ones. Please don't bang the door.\"");
		Line(lines, "any.aside.shift.1", -1, "aside_shift", "", P, 0, "", "\"Shift change. Give me a minute. I can still help you.\"");
	}
}
