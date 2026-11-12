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
		Infected(lines);
		RunningJokes(lines);
		Routes(lines);
		Looks(lines);
		Owed(lines);
		Changes(lines);
		Relations(lines);
		Wanderers(lines);
		RadioTails(lines);
		NewLifeRoutes(lines);
		Week(lines);
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue week's jokes (kind "week", RZ_RepVoice.Opening after the day's plain opener):
	//! a few a faction for each ending, read through cure_held. Jokes only: a roll that finds no joke
	//! says nothing, and the opener's plain lines stay. No cure: only the Red Hand's last.
	protected static void Week(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;

		Line(lines, "haven.week.haven.1", HAVEN, "week", "", J, 16, "cure_held:haven", "\"We've got the cure and forty bowls. The council's voting on which to hand out first.\"");
		Line(lines, "haven.week.haven.2", HAVEN, "week", "", W, 16, "cure_held:haven", "\"The council wants it minuted that the cure was a group effort. Mostly the soup.\"");
		Line(lines, "haven.week.haven.3", HAVEN, "week", "", J, 16, "cure_held:haven", "\"Somebody wanted a cure party. We're having soup. It's a soup party.\"");
		Line(lines, "haven.week.haven.4", HAVEN, "week", "", W, 16, "cure_held:haven", "\"First time the council's agreed on anything all season. We're keeping the minutes in a jar.\"");
		Line(lines, "haven.week.haven.5", HAVEN, "week", "", J, 16, "cure_held:haven", "\"Everyone gets a dose. Everyone. The council counted twice, then counted the dog.\"");
		Line(lines, "haven.week.bastion.1", HAVEN, "week", "", J, 16, "cure_held:bastion", "\"Bastion has the cure. They've given it a form. We've filled it in. Twice.\"");
		Line(lines, "haven.week.bastion.2", HAVEN, "week", "", W, 16, "cure_held:bastion", "\"The soldiers hand it out in alphabetical order. Half the house has become an Abbott.\"");
		Line(lines, "haven.week.bastion.3", HAVEN, "week", "", J, 16, "cure_held:bastion", "\"Bastion says the queue is orderly. It's a queue. We invented those.\"");
		Line(lines, "haven.week.bastion.4", HAVEN, "week", "", W, 16, "cure_held:bastion", "\"They stamped my hand. I haven't washed it. Nobody's told me what it's for.\"");
		Line(lines, "haven.week.redhand.1", HAVEN, "week", "", J, 16, "cure_held:redhand", "\"The Red Hand has the cure. The price board's in chalk, so it can go up quicker.\"");
		Line(lines, "haven.week.redhand.2", HAVEN, "week", "", W, 16, "cure_held:redhand", "\"We paid in tins and they gave us a receipt. First receipt the Red Hand ever wrote.\"");
		Line(lines, "haven.week.redhand.3", HAVEN, "week", "", J, 16, "cure_held:redhand", "\"They sell it by the dose. We've started counting the children twice, in case.\"");
		Line(lines, "haven.week.redhand.4", HAVEN, "week", "", W, 16, "cure_held:redhand", "\"The boss says the first dose is free. The way you'd say the first step's free.\"");
		Line(lines, "haven.week.halcyon.1", HAVEN, "week", "", J, 16, "cure_held:halcyon", "\"The lab wants us to queue by blood type. Most of us don't know ours. They've written 'unknown' very neatly.\"");
		Line(lines, "haven.week.halcyon.2", HAVEN, "week", "", W, 16, "cure_held:halcyon", "\"Halcyon gave us a leaflet with the dose. Lovely leaflet. Shame about the words.\"");
		Line(lines, "haven.week.halcyon.3", HAVEN, "week", "", J, 16, "cure_held:halcyon", "\"They thanked us for our participation. We didn't know we'd participated. Turns out we had.\"");
		Line(lines, "haven.week.halcyon.4", HAVEN, "week", "", W, 16, "cure_held:halcyon", "\"A man in a white coat asked if I'd like to be in a study. I said I've been in one all season.\"");
		Line(lines, "haven.week.everyone.1", HAVEN, "week", "", J, 16, "cure_held:everyone", "\"Somebody said the cure's for everyone. The council's never been so short of things to argue about.\"");
		Line(lines, "haven.week.everyone.2", HAVEN, "week", "", W, 16, "cure_held:everyone", "\"No forms, no tins, no study. The council doesn't know what to do with its afternoon.\"");
		Line(lines, "haven.week.everyone.3", HAVEN, "week", "", J, 16, "cure_held:everyone", "\"Nobody won it. Lovely. Nobody's ever won anything round here before.\"");
		Line(lines, "haven.week.everyone.4", HAVEN, "week", "", W, 16, "cure_held:everyone", "\"It's everyone's. Six people have already told me it was their idea.\"");
		Line(lines, "haven.week.slow.1", HAVEN, "week", "", J, 16, "cure_held:slow", "\"The lab got there on its own. Everybody's claiming they helped. Even the ones who hid in the cellar.\"");
		Line(lines, "haven.week.slow.2", HAVEN, "week", "", W, 16, "cure_held:slow", "\"Turns out you can just wait for a cure. Who knew. Not the council.\"");
		Line(lines, "haven.week.slow.3", HAVEN, "week", "", J, 16, "cure_held:slow", "\"Nobody fought for it, and it came anyway. Like the post used to.\"");
		Line(lines, "haven.week.slow.4", HAVEN, "week", "", W, 16, "cure_held:slow", "\"We've all decided we were very patient. We weren't. It reads well, though.\"");
		Line(lines, "haven.week.handed.1", HAVEN, "week", "", J, 16, "cure_held:nobody", "\"The lab handed it over. Very polite. Like giving back a borrowed ladder.\"");
		Line(lines, "haven.week.handed.2", HAVEN, "week", "", W, 16, "cure_held:nobody", "\"It came to us in the end like everything else: second-hand.\"");
		Line(lines, "haven.week.handed.3", HAVEN, "week", "", J, 16, "cure_held:nobody", "\"Everyone's claiming they asked for it first. The council's keeping a list. Of course it is.\"");
		Line(lines, "haven.week.handed.4", HAVEN, "week", "", W, 16, "cure_held:nobody", "\"Handed over, no fuss. The council had a speech ready. It's keeping it for the next thing.\"");

		Line(lines, "bastion.week.haven.1", BASTION, "week", "", J, 16, "cure_held:haven", "\"The Haven controls the cure. Command would have had something to say. Command still has nothing to say.\"");
		Line(lines, "bastion.week.haven.2", BASTION, "week", "", W, 16, "cure_held:haven", "\"We're to queue with civilians. The sergeant has asked whether it counts as a posting.\"");
		Line(lines, "bastion.week.haven.3", BASTION, "week", "", J, 16, "cure_held:haven", "\"The Haven issues it with soup. Logged as rations.\"");
		Line(lines, "bastion.week.haven.4", BASTION, "week", "", W, 16, "cure_held:haven", "\"They want our names, not our numbers. Half the post had to think about it.\"");
		Line(lines, "bastion.week.bastion.1", BASTION, "week", "", J, 16, "cure_held:bastion", "\"The cure is under military control. Form nine is the cure. Form ten is the complaint. Form ten is very popular.\"");
		Line(lines, "bastion.week.bastion.2", BASTION, "week", "", W, 16, "cure_held:bastion", "\"Distribution is orderly. The sergeant cried once. It's been logged as weather.\"");
		Line(lines, "bastion.week.bastion.3", BASTION, "week", "", J, 16, "cure_held:bastion", "\"We have the cure and the paperwork. Command would be proud, if it was answering.\"");
		Line(lines, "bastion.week.bastion.4", BASTION, "week", "", W, 16, "cure_held:bastion", "\"Every dose is signed for. So are the pens. We've learned.\"");
		Line(lines, "bastion.week.bastion.5", BASTION, "week", "", J, 16, "cure_held:bastion", "\"First successful operation of the season. It's on the board under 'other'.\"");
		Line(lines, "bastion.week.redhand.1", BASTION, "week", "", J, 16, "cure_held:redhand", "\"The Red Hand controls the cure. We pay a toll for our own medicine. The irony's been logged.\"");
		Line(lines, "bastion.week.redhand.2", BASTION, "week", "", W, 16, "cure_held:redhand", "\"They gave the post a discount. Ten per cent off for uniforms. Nobody knows what to feel.\"");
		Line(lines, "bastion.week.redhand.3", BASTION, "week", "", J, 16, "cure_held:redhand", "\"The quartermaster's negotiating. First time he's been on that side of a counter.\"");
		Line(lines, "bastion.week.redhand.4", BASTION, "week", "", W, 16, "cure_held:redhand", "\"We're told the book has a page for Bastion. It's a long page.\"");
		Line(lines, "bastion.week.halcyon.1", BASTION, "week", "", J, 16, "cure_held:halcyon", "\"The lab asked for our blood types. We gave them our service numbers. It went badly.\"");
		Line(lines, "bastion.week.halcyon.2", BASTION, "week", "", W, 16, "cure_held:halcyon", "\"The lab's doing it by the book. Their book. We've asked for a copy. Request pending.\"");
		Line(lines, "bastion.week.halcyon.3", BASTION, "week", "", J, 16, "cure_held:halcyon", "\"We've been reclassified as a cohort. The sergeant's taking it personally.\"");
		Line(lines, "bastion.week.halcyon.4", BASTION, "week", "", W, 16, "cure_held:halcyon", "\"Dose by appointment. Nobody here has had an appointment since the barracks dentist.\"");
		Line(lines, "bastion.week.everyone.1", BASTION, "week", "", J, 16, "cure_held:everyone", "\"The cure goes to everyone. No chain of command. The post isn't sure who to salute.\"");
		Line(lines, "bastion.week.everyone.2", BASTION, "week", "", W, 16, "cure_held:everyone", "\"Nobody's in charge of it. Filed under 'unprecedented'. Then under 'fine'.\"");
		Line(lines, "bastion.week.everyone.3", BASTION, "week", "", J, 16, "cure_held:everyone", "\"Everybody gets it. Even the ones on our list. The list's been very quiet about it.\"");
		Line(lines, "bastion.week.everyone.4", BASTION, "week", "", W, 16, "cure_held:everyone", "\"No faction controls it. Command would call that a failure. Command still hasn't called.\"");
		Line(lines, "bastion.week.slow.1", BASTION, "week", "", J, 16, "cure_held:slow", "\"The lab got there on its own. We held the line for that. Log it as a win. Log it as anything.\"");
		Line(lines, "bastion.week.slow.2", BASTION, "week", "", W, 16, "cure_held:slow", "\"No operation, no objective, and it's done. The sergeant's sat down for the first time in weeks.\"");
		Line(lines, "bastion.week.slow.3", BASTION, "week", "", J, 16, "cure_held:slow", "\"It came without us. Like the relief. Except it came.\"");
		Line(lines, "bastion.week.slow.4", BASTION, "week", "", W, 16, "cure_held:slow", "\"Everyone's claiming a share. The post has claimed the radio. Nobody else wanted it.\"");
		Line(lines, "bastion.week.handed.1", BASTION, "week", "", J, 16, "cure_held:nobody", "\"The lab handed it over. Received and signed for. First delivery that's arrived all season.\"");
		Line(lines, "bastion.week.handed.2", BASTION, "week", "", W, 16, "cure_held:nobody", "\"Handed over without a fight. The post's never seen that. It's unsettling.\"");
		Line(lines, "bastion.week.handed.3", BASTION, "week", "", J, 16, "cure_held:nobody", "\"Logged as a transfer of materiel. The sergeant wanted 'miracle'. It isn't on the form.\"");
		Line(lines, "bastion.week.handed.4", BASTION, "week", "", W, 16, "cure_held:nobody", "\"Handed over and nobody shot. We're checking it was the right building.\"");

		Line(lines, "redhand.week.haven.1", REDHAND, "week", "", J, 16, "cure_held:haven", "\"The Haven has the cure and they're giving it away. Terrible for business. Sort of beautiful. Mostly terrible.\"");
		Line(lines, "redhand.week.haven.2", REDHAND, "week", "", W, 16, "cure_held:haven", "\"Soup and a dose, no charge. We tried competing. Nobody wants our soup.\"");
		Line(lines, "redhand.week.haven.3", REDHAND, "week", "", J, 16, "cure_held:haven", "\"The council voted to let us queue. Seven to six. We've been robbed by worse.\"");
		Line(lines, "redhand.week.haven.4", REDHAND, "week", "", W, 16, "cure_held:haven", "\"They're giving it out free. The boss has gone for a walk. A long one.\"");
		Line(lines, "redhand.week.haven.5", REDHAND, "week", "", J, 16, "cure_held:haven", "\"We offered to sell them a queue. They had one already. Free, apparently.\"");
		Line(lines, "redhand.week.bastion.1", REDHAND, "week", "", J, 16, "cure_held:bastion", "\"Bastion has the cure. Forms, stamps, queues. We'd sell you a shortcut if there was one.\"");
		Line(lines, "redhand.week.bastion.2", REDHAND, "week", "", W, 16, "cure_held:bastion", "\"There's a form for the cure, and there's a market for the form. Not saying whose.\"");
		Line(lines, "redhand.week.bastion.3", REDHAND, "week", "", J, 16, "cure_held:bastion", "\"They searched me for the cure on my way to get the cure. Professional. You have to give them that.\"");
		Line(lines, "redhand.week.bastion.4", REDHAND, "week", "", W, 16, "cure_held:bastion", "\"The cure's free. The pen costs ten Favor. That bit's us.\"");
		Line(lines, "redhand.week.redhand.1", REDHAND, "week", "", J, 16, "cure_held:redhand", "\"We have the cure. The book balances. First time ever. Frame it.\"");
		Line(lines, "redhand.week.redhand.2", REDHAND, "week", "", W, 16, "cure_held:redhand", "\"Prices are fair. Fair-ish. Fair for us.\"");
		Line(lines, "redhand.week.redhand.3", REDHAND, "week", "", J, 16, "cure_held:redhand", "\"Under new management: the same management. Cure's in the back.\"");
		Line(lines, "redhand.week.redhand.4", REDHAND, "week", "", W, 16, "cure_held:redhand", "\"The boss wants it known there were tears. There weren't. Write it down anyway.\"");
		Line(lines, "redhand.week.redhand.5", REDHAND, "week", "", J, 16, "cure_held:redhand", "\"Every dose has a price and every price comes with a smile. The smile's extra.\"");
		Line(lines, "redhand.week.halcyon.1", REDHAND, "week", "", J, 16, "cure_held:halcyon", "\"Halcyon has the cure. Lovely labels. You could sell the labels on their own.\"");
		Line(lines, "redhand.week.halcyon.2", REDHAND, "week", "", W, 16, "cure_held:halcyon", "\"They want our blood before they give us theirs. Fair trade. We'd respect it if it was us.\"");
		Line(lines, "redhand.week.halcyon.3", REDHAND, "week", "", J, 16, "cure_held:halcyon", "\"The lab hands it out in order of need. We've never needed anything this much. Or this politely.\"");
		Line(lines, "redhand.week.halcyon.4", REDHAND, "week", "", W, 16, "cure_held:halcyon", "\"Tried to bribe a scientist. He wrote it down. As data.\"");
		Line(lines, "redhand.week.everyone.1", REDHAND, "week", "", J, 16, "cure_held:everyone", "\"Free cure, for everyone, no toll. Somebody's ruined the economy. I want to shake their hand, then check my pockets.\"");
		Line(lines, "redhand.week.everyone.2", REDHAND, "week", "", W, 16, "cure_held:everyone", "\"Nobody owns it. We've looked. We had a really good look.\"");
		Line(lines, "redhand.week.everyone.3", REDHAND, "week", "", J, 16, "cure_held:everyone", "\"Free. Say it slower. Still hurts.\"");
		Line(lines, "redhand.week.everyone.4", REDHAND, "week", "", W, 16, "cure_held:everyone", "\"Everyone gets one. The book's got nobody to write down. Feels naked.\"");
		Line(lines, "redhand.week.slow.1", REDHAND, "week", "", J, 16, "cure_held:slow", "\"The lab finished it on its own. Nobody fought for it. Nobody to charge. Tragic.\"");
		Line(lines, "redhand.week.slow.2", REDHAND, "week", "", W, 16, "cure_held:slow", "\"We waited and it came. Should've charged for the waiting.\"");
		Line(lines, "redhand.week.slow.3", REDHAND, "week", "", J, 16, "cure_held:slow", "\"Everyone says they helped. We helped too. Put us down for a share. A large share.\"");
		Line(lines, "redhand.week.slow.4", REDHAND, "week", "", W, 16, "cure_held:slow", "\"No winner. We've decided that means us. It's in the book.\"");
		Line(lines, "redhand.week.handed.1", REDHAND, "week", "", J, 16, "cure_held:nobody", "\"The lab handed it over. For nothing. We'd have charged a finder's fee.\"");
		Line(lines, "redhand.week.handed.2", REDHAND, "week", "", W, 16, "cure_held:nobody", "\"Handed over, no toll, no haggling. Amateurs.\"");
		Line(lines, "redhand.week.handed.3", REDHAND, "week", "", J, 16, "cure_held:nobody", "\"Robbed. Of the chance to rob. There's a word for that and it isn't nice.\"");
		Line(lines, "redhand.week.handed.4", REDHAND, "week", "", W, 16, "cure_held:nobody", "\"It changed hands without us. Never happened before. Felt cold.\"");
		Line(lines, "redhand.week.none.1", REDHAND, "week", "", J, 16, "cure_held:none", "\"No cure. The book closes when the gates open. Debts die with the page. Drinks are on whoever's left.\"");
		Line(lines, "redhand.week.none.2", REDHAND, "week", "", W, 16, "cure_held:none", "\"No cure, no point, and we're still charging the toll. Standards.\"");
		Line(lines, "redhand.week.none.3", REDHAND, "week", "", J, 16, "cure_held:none", "\"Last week of business. Everything must go. Including us.\"");

		Line(lines, "halcyon.week.haven.1", HALCYON, "week", "", J, 16, "cure_held:haven", "\"The Haven controls distribution. They've replaced our dosage tables with soup bowls. The bowls are, admittedly, consistent.\"");
		Line(lines, "halcyon.week.haven.2", HALCYON, "week", "", W, 16, "cure_held:haven", "\"Our data is read aloud at supper now. Some of it as names. The director has asked to sit at the back.\"");
		Line(lines, "halcyon.week.haven.3", HALCYON, "week", "", J, 16, "cure_held:haven", "\"The Haven calls it a cure. We call it a compound. They're winning the naming.\"");
		Line(lines, "halcyon.week.haven.4", HALCYON, "week", "", W, 16, "cure_held:haven", "\"They thanked the lab in a prayer. We've recorded it. There isn't a column for it.\"");
		Line(lines, "halcyon.week.bastion.1", HALCYON, "week", "", J, 16, "cure_held:bastion", "\"Bastion controls distribution. Every dose signed for. We've never been so well documented, or so unhappy about it.\"");
		Line(lines, "halcyon.week.bastion.2", HALCYON, "week", "", W, 16, "cure_held:bastion", "\"Our compound has a serial number now. Several. The director is coping.\"");
		Line(lines, "halcyon.week.bastion.3", HALCYON, "week", "", J, 16, "cure_held:bastion", "\"Distribution by rank. Our subjects have never been so well organised. Or so annoyed.\"");
		Line(lines, "halcyon.week.bastion.4", HALCYON, "week", "", W, 16, "cure_held:bastion", "\"The army salutes the crates. We're not sure what that does to the dose.\"");
		Line(lines, "halcyon.week.redhand.1", HALCYON, "week", "", J, 16, "cure_held:redhand", "\"The Red Hand controls distribution. Our compound has a price list. The price list has a sense of humour. Ours is under review.\"");
		Line(lines, "halcyon.week.redhand.2", HALCYON, "week", "", W, 16, "cure_held:redhand", "\"They asked what a dose costs to make. We told them. They laughed for some time.\"");
		Line(lines, "halcyon.week.redhand.3", HALCYON, "week", "", J, 16, "cure_held:redhand", "\"The director asked that nobody round anything up. They're rounding everything up.\"");
		Line(lines, "halcyon.week.redhand.4", HALCYON, "week", "", W, 16, "cure_held:redhand", "\"The boss calls our compound 'the good stuff'. It has not been peer reviewed as such.\"");
		Line(lines, "halcyon.week.halcyon.1", HALCYON, "week", "", J, 16, "cure_held:halcyon", "\"Distribution is ours. Every subject in order. The director smiled. It's been noted.\"");
		Line(lines, "halcyon.week.halcyon.2", HALCYON, "week", "", W, 16, "cure_held:halcyon", "\"The cure is under study. So is everyone receiving it. Efficient.\"");
		Line(lines, "halcyon.week.halcyon.3", HALCYON, "week", "", J, 16, "cure_held:halcyon", "\"We controlled the outcome. That's the paper's first sentence. The director wrote it twice.\"");
		Line(lines, "halcyon.week.halcyon.4", HALCYON, "week", "", W, 16, "cure_held:halcyon", "\"Dosing proceeds by protocol. The protocol has a protocol. It's going very well.\"");
		Line(lines, "halcyon.week.halcyon.5", HALCYON, "week", "", J, 16, "cure_held:halcyon", "\"Thank you for your participation. You've always been participating. Now you know.\"");
		Line(lines, "halcyon.week.everyone.1", HALCYON, "week", "", J, 16, "cure_held:everyone", "\"Uncontrolled distribution. Every textbook warns against it. It's going rather well.\"");
		Line(lines, "halcyon.week.everyone.2", HALCYON, "week", "", W, 16, "cure_held:everyone", "\"The cure goes to everyone. Our sample size is now the Zone. Lovely sample.\"");
		Line(lines, "halcyon.week.everyone.3", HALCYON, "week", "", J, 16, "cure_held:everyone", "\"No faction controls it. The director's drafting a sentence about that. Draft nine.\"");
		Line(lines, "halcyon.week.everyone.4", HALCYON, "week", "", W, 16, "cure_held:everyone", "\"Nobody owns the result. Peer review will be fascinating.\"");
		Line(lines, "halcyon.week.slow.1", HALCYON, "week", "", J, 16, "cure_held:slow", "\"The lab finished on its own work. Nobody fought for it. The cleanest result we've had.\"");
		Line(lines, "halcyon.week.slow.2", HALCYON, "week", "", W, 16, "cure_held:slow", "\"Everyone's claiming a share. We have the data. The data claims nobody.\"");
		Line(lines, "halcyon.week.slow.3", HALCYON, "week", "", J, 16, "cure_held:slow", "\"No route, no winner, one result. The director calls it elegant. The director is alone in this.\"");
		Line(lines, "halcyon.week.slow.4", HALCYON, "week", "", W, 16, "cure_held:slow", "\"It arrived on schedule. Our schedule. Nobody's ever believed our schedule.\"");
		Line(lines, "halcyon.week.handed.1", HALCYON, "week", "", J, 16, "cure_held:nobody", "\"We handed it over. It's recorded as a transfer, not a loss. The director insisted on the wording.\"");
		Line(lines, "halcyon.week.handed.2", HALCYON, "week", "", W, 16, "cure_held:nobody", "\"Handed over with full documentation. The documentation lit somebody's stove.\"");
		Line(lines, "halcyon.week.handed.3", HALCYON, "week", "", J, 16, "cure_held:nobody", "\"The handover took four minutes. The paperwork took four days. Normal.\"");
		Line(lines, "halcyon.week.handed.4", HALCYON, "week", "", W, 16, "cure_held:nobody", "\"Our compound, their hands. That's what compounds are for, the director says. To the wall, mostly.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The open channels' tails (kind "radio_tail", RZ_VoiceBank.PickShared): one short line after a
	//! piece of chatter, picked once for everyone who hears it. Never a player, never a number the
	//! world doesn't have. Plain lines outnumber the jokes; a channel in grief says only plain ones.
	protected static void RadioTails(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;

		Line(lines, "haven.tail.1", HAVEN, "radio_tail", "", P, 0, "", "Haven out.");
		Line(lines, "haven.tail.2", HAVEN, "radio_tail", "", P, 0, "", "Doors are open. Bring yourself.");
		Line(lines, "haven.tail.3", HAVEN, "radio_tail", "", W, 0, "", "Soup's thin. It's honest, though.");
		Line(lines, "haven.tail.4", HAVEN, "radio_tail", "", W, 0, "", "Somebody bring the big pot back. You know who you are.");
		Line(lines, "bastion.tail.1", BASTION, "radio_tail", "", P, 0, "", "Bastion out.");
		Line(lines, "bastion.tail.2", BASTION, "radio_tail", "", P, 0, "", "Acknowledge.");
		Line(lines, "bastion.tail.3", BASTION, "radio_tail", "", W, 0, "", "Radio discipline, all stations. That includes you, post two.");
		Line(lines, "bastion.tail.4", BASTION, "radio_tail", "", W, 0, "", "Still nothing from Command. Logged.");
		Line(lines, "redhand.tail.1", REDHAND, "radio_tail", "", P, 0, "", "That's all.");
		Line(lines, "redhand.tail.2", REDHAND, "radio_tail", "", J, 0, "", "Tolls are up. Morale is up. One of those is a lie.");
		Line(lines, "redhand.tail.3", REDHAND, "radio_tail", "", J, 0, "", "If you're Bastion and you're listening: hello. Pay the toll.");
		Line(lines, "redhand.tail.4", REDHAND, "radio_tail", "", W, 0, "", "Book's open. Book's always open.");
		Line(lines, "halcyon.tail.1", HALCYON, "radio_tail", "", P, 0, "", "Halcyon out.");
		Line(lines, "halcyon.tail.2", HALCYON, "radio_tail", "", P, 0, "", "Thank you for your cooperation.");
		Line(lines, "halcyon.tail.3", HALCYON, "radio_tail", "", W, 0, "", "The director asks that nobody round anything up.");

		// Feud and truce on the channels (the same for everyone who hears it)
		Line(lines, "bastion.tail.truce", BASTION, "radio_tail", "", W, 0, "truce:redhand", "Joint patrol with the Red Hand tonight. Both sides counting the ammunition.");
		Line(lines, "redhand.tail.truce", REDHAND, "radio_tail", "", J, 0, "truce:bastion", "We're friends with Bastion this week. Hide the good tins.");
		Line(lines, "redhand.tail.feud", REDHAND, "radio_tail", "", P, 0, "feud:bastion", "Bastion's trucks are fair game. Pass it on.");
		Line(lines, "haven.tail.feud", HAVEN, "radio_tail", "", P, 0, "feud:redhand", "Red Hand crews aren't welcome at the doors while this lasts.");
		Line(lines, "halcyon.tail.feud", HALCYON, "radio_tail", "", P, 0, "feud:redhand", "Field teams avoid the toll roads until further notice.");
	}

	//------------------------------------------------------------------------------------------------
	//! A locked identity on a new life, at a leader of its route: they've heard (kind "route", the
	//! first visit each real day; RZ_RepVoice.RouteLine).
	protected static void NewLifeRoutes(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		Line(lines, "haven.route.newlife", HAVEN, "route", "", P, 0, "route:in & fresh", "\"New face. Somebody said you're with us. Then you're with us.\"");
		Line(lines, "bastion.route.newlife", BASTION, "route", "", P, 0, "route:in & fresh", "\"Different face, same name on the roster. Somebody said you're with us.\"");
		Line(lines, "redhand.route.newlife", REDHAND, "route", "", P, 0, "route:in & fresh", "\"Somebody said you're with us. The book says so too. New face suits you.\"");
		Line(lines, "halcyon.route.newlife", HALCYON, "route", "", P, 0, "route:in & fresh", "\"A new subject on an old record. Somebody said you're with us.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The wanderers' greeting (kind "wanderer", RZ_RepVoice.WandererGreeting, Trees on): by the
	//! manner of your Haven standing (the Deserter: Bastion's, reversed), after what they remember of
	//! you this life. Plain voice: the road's people belong to nobody.
	protected static void Wanderers(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;

		Line(lines, "wanderer.greet.unknown.1", HAVEN, "wanderer", "unknown", P, 0, "", "\"Don't know you. That's not a complaint. Out here it's almost a compliment.\"");
		Line(lines, "wanderer.greet.unknown.2", HAVEN, "wanderer", "unknown", W, 0, "", "\"New face. Keep it that way, it suits you.\"");
		Line(lines, "wanderer.greet.warm.1", HAVEN, "wanderer", "warm", P, 0, "", "\"The houses speak well of you. I listen to the houses.\"");
		Line(lines, "wanderer.greet.warm.2", HAVEN, "wanderer", "warm", P, 0, "", "\"Oh, it's you. Good. Sit a minute, if you've got one.\"");
		Line(lines, "wanderer.greet.devoted.1", HAVEN, "wanderer", "devoted", P, 0, "", "\"Every house from here to the river says your name at supper. Did you know that?\"");
		Line(lines, "wanderer.greet.wary.1", HAVEN, "wanderer", "wary", P, 0, "", "\"I've heard things. I'll make up my own mind, in my own time.\"");
		Line(lines, "wanderer.greet.wary.2", HAVEN, "wanderer", "wary", W, 0, "", "\"The houses count their spoons after you've been. I'm told.\"");
		Line(lines, "wanderer.greet.fond.1", HAVEN, "wanderer", "fond", W, 0, "", "\"Half the stories about you are good. I like those ones.\"");
		Line(lines, "wanderer.greet.needed.1", HAVEN, "wanderer", "needed", P, 0, "", "\"People say you're trouble. People also say you came when they called. Both, then.\"");
		Line(lines, "wanderer.greet.hostile.1", HAVEN, "wanderer", "hostile", P, 0, "", "\"Stay over there. I'll talk, but from over here.\"");
		Line(lines, "wanderer.greet.helped.1", HAVEN, "wanderer", "helped", P, 0, "", "\"You helped me once. I don't forget that. I forget most things.\"");
		Line(lines, "wanderer.greet.saved.1", HAVEN, "wanderer", "saved", P, 0, "", "\"You. You're the reason I'm still walking. Sit. Please.\"");
		Line(lines, "wanderer.greet.wronged.1", HAVEN, "wanderer", "wronged", P, 0, "", "\"I remember you. I'm talking because I'd rather not run again.\"");
		Line(lines, "deserter.greet.bastion_hostile.1", BASTION, "wanderer_deserter", "hostile", W, 0, "", "\"Bastion wants you? Then we've got something in common. Pull up a rock.\"");
		Line(lines, "deserter.greet.bastion_warm.1", BASTION, "wanderer_deserter", "warm", P, 0, "", "\"You're one of their favourites. Don't take it personally that I keep my distance.\"");
		Line(lines, "deserter.greet.bastion_unknown.1", BASTION, "wanderer_deserter", "unknown", P, 0, "", "\"Bastion doesn't know you. Keep it that way. It's the best thing you've got.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The change notice in the faction's voice (RZ_RepVoice.ChangeLine, Trees on): a new name, said
	//! by manner; the rule follows in brackets. {rep} is the listener's new name with them.
	protected static void Changes(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;

		Line(lines, "haven.change.warm", HAVEN, "change", "warm", P, 0, "", "\"The houses are calling you {rep} now. Wipe your feet anyway.\"");
		Line(lines, "haven.change.devoted", HAVEN, "change", "devoted", P, 0, "", "\"{rep}. That's what they say at supper now, when they say you.\"");
		Line(lines, "haven.change.mixed", HAVEN, "change", "wary", W, 0, "", "\"The council has a new word for you: {rep}. It argued about it. It argues about everything.\"");
		Line(lines, "haven.change.hostile", HAVEN, "change", "hostile", P, 0, "", "\"The doors are shut to you now. Nobody's happy about it.\"");
		Line(lines, "bastion.change.warm", BASTION, "change", "warm", P, 0, "", "\"Your file's been updated. {rep}. Initialled.\"");
		Line(lines, "bastion.change.mixed", BASTION, "change", "fond", W, 0, "", "\"New entry: {rep}. The column for good and the column for trouble are both getting long.\"");
		Line(lines, "bastion.change.hostile", BASTION, "change", "hostile", P, 0, "", "\"You're on the list. Every post has the list.\"");
		Line(lines, "redhand.change.warm", REDHAND, "change", "warm", W, 0, "", "\"The book's got a new line for you. {rep}. Don't let it go to your head. It's in pencil.\"");
		Line(lines, "redhand.change.needed", REDHAND, "change", "needed", W, 0, "", "\"{rep}, the crews say. Said fondly. Mostly fondly.\"");
		Line(lines, "redhand.change.hostile", REDHAND, "change", "hostile", P, 0, "", "\"Your page in the book is red now, top to bottom.\"");
		Line(lines, "halcyon.change.warm", HALCYON, "change", "warm", P, 0, "", "\"You've been recategorised: {rep}. It's a category, not a compliment.\"");
		Line(lines, "halcyon.change.mixed", HALCYON, "change", "wary", P, 0, "", "\"Your category changed: {rep}. The data disagree with each other. So do we.\"");
		Line(lines, "halcyon.change.hostile", HALCYON, "change", "hostile", P, 0, "", "\"You are excluded from the study. Effective now.\"");
		Line(lines, "any.change.owed", -1, "change", "owed", P, 0, "", "\"They want you dead now, and they owe you a hearing. Both, at once.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! A feud sharpens the Red Hand's taunts and Bastion's lists; a truce allows one joke across the
	//! line. Face to face (people at holdings), read from faction life's truces and feuds.
	protected static void Relations(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;

		Line(lines, "redhand.person.truce.bastion", REDHAND, "person", "", J, 0, "truce:bastion", "\"We shared a fire with Bastion last night. Both sides counted the matches.\"");
		Line(lines, "bastion.person.truce.redhand", BASTION, "person", "", W, 0, "truce:redhand", "\"There's a truce with the Red Hand. We've been told to log it as weather.\"");
		Line(lines, "haven.person.truce.redhand", HAVEN, "person", "", W, 0, "truce:redhand", "\"The Red Hand sent a sack of flour under the truce. We're checking it for a price tag.\"");
		Line(lines, "redhand.person.feud.bastion", REDHAND, "person", "", J, 0, "feud:bastion", "\"Bastion's at war with us again. Good. They're more fun angry.\"");
		Line(lines, "bastion.person.feud.redhand", BASTION, "person", "", P, 0, "feud:redhand", "\"The Red Hand list is longer this week. Read it twice before your shift.\"");
		Line(lines, "halcyon.person.feud.bastion", HALCYON, "person", "", P, 0, "feud:bastion", "\"Bastion and the lab aren't speaking. The samples still arrive. Nobody signs for them.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! Route lines face to face after the 90% lock (docs/WRITING.md, "The race and the lock"): the
	//! first visit to a leader each real day, said instead of the opener. In-group: one of their own;
	//! out-group: cool for two real days (lockdays<2), then civil; Independent: a little afraid;
	//! unsided: courted once a visit; after the cure, the losing route's own leaders give one plain
	//! line, and the winners say theirs (flag:finale.route is the controlling route + 1). Never on air.
	protected static void Routes(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;

		Line(lines, "haven.route.in.1", HAVEN, "route", "", P, 0, "route:in", "\"Somebody said you're with the houses now. Sit. You'll want feeding.\"");
		Line(lines, "haven.route.in.2", HAVEN, "route", "", J, 0, "route:in", "\"With the houses, are you? Then you're on the rota. Everybody's on the rota.\"");
		Line(lines, "haven.route.out.cool", HAVEN, "route", "", P, 0, "route:out & lockdays<2", "\"You picked someone else's door. That's allowed. The soup's still soup.\"");
		Line(lines, "haven.route.out.1", HAVEN, "route", "", W, 0, "route:out & lockdays>=2", "\"The other lot's, aren't you. We'll feed you anyway. We're hopeless at grudges.\"");
		Line(lines, "haven.route.indep.1", HAVEN, "route", "", W, 0, "route:independent", "\"Nobody's, they tell me. That sounds lonely. Eat something.\"");
		Line(lines, "haven.route.unsided.1", HAVEN, "route", "", P, 0, "route:unsided", "\"Nobody's asked you yet? Then I will. Bowl first. Then the question: who do you trust to share it?\"");
		Line(lines, "haven.route.lost.1", HAVEN, "route", "", P, 0, "route:lost & route:haven", "\"It didn't go our way. It wasn't you. Have some soup.\"");
		Line(lines, "haven.route.winner.1", HAVEN, "route", "", P, 0, "route:lost & flag:finale.route=1", "\"You stood somewhere else. It's all right. There's a queue, and you're in it, same as anyone.\"");

		Line(lines, "bastion.route.in.1", BASTION, "route", "", P, 0, "route:in", "\"Attached. Your name's on the board in pencil. Don't make us rub it out.\"");
		Line(lines, "bastion.route.in.2", BASTION, "route", "", J, 0, "route:in", "\"Welcome to the roster. The pay is the same as ours. Nothing, on time.\"");
		Line(lines, "bastion.route.out.cool", BASTION, "route", "", P, 0, "route:out & lockdays<2", "\"Civilian.\"");
		Line(lines, "bastion.route.out.1", BASTION, "route", "", W, 0, "route:out & lockdays>=2", "\"Civilian. Filed with the other lot. Move along, please.\"");
		Line(lines, "bastion.route.indep.1", BASTION, "route", "", W, 0, "route:independent", "\"Unaffiliated. There's a form for that. It's blank.\"");
		Line(lines, "bastion.route.unsided.1", BASTION, "route", "", P, 0, "route:unsided", "\"There's a form for people who haven't decided. It's short. Sign it and you've decided.\"");
		Line(lines, "bastion.route.lost.1", BASTION, "route", "", P, 0, "route:lost & route:bastion", "\"It went the other way. Your part was logged. That's what a log is for.\"");
		Line(lines, "bastion.route.winner.1", BASTION, "route", "", P, 0, "route:lost & flag:finale.route=2", "\"You'll be processed with everyone else. In order. That's the point.\"");

		Line(lines, "redhand.route.in.1", REDHAND, "route", "", J, 0, "route:in", "\"In on it. Good. Your share's in the book. So's your debt. Don't worry about it.\"");
		Line(lines, "redhand.route.in.2", REDHAND, "route", "", P, 0, "route:in", "\"You're with us now. That means we lie to everyone but you, on a good day.\"");
		Line(lines, "redhand.route.out.cool", REDHAND, "route", "", J, 0, "route:out & lockdays<2", "\"The other crew, is it? Bold. Wrong, but bold.\"");
		Line(lines, "redhand.route.out.1", REDHAND, "route", "", J, 0, "route:out & lockdays>=2", "\"Still with that lot? We'll keep a page open for when you come to your senses.\"");
		Line(lines, "redhand.route.indep.1", REDHAND, "route", "", J, 0, "route:independent", "\"Freelance. I respect it. I'd also like to know where you sleep.\"");
		Line(lines, "redhand.route.unsided.1", REDHAND, "route", "", W, 0, "route:unsided", "\"Haven't picked yet? Smart. Hear our offer first. It's terrible, but it's honest.\"");
		Line(lines, "redhand.route.lost.1", REDHAND, "route", "", P, 0, "route:lost & route:redhand", "\"Didn't come off. The book takes the loss, not you. Go on.\"");
		Line(lines, "redhand.route.winner.1", REDHAND, "route", "", J, 0, "route:lost & flag:finale.route=3", "\"No hard feelings. Some soft ones.\"");

		Line(lines, "halcyon.route.in.1", HALCYON, "route", "", P, 0, "route:in", "\"Signed on. Your initials are on page three. Try to keep them there.\"");
		Line(lines, "halcyon.route.out.cool", HALCYON, "route", "", P, 0, "route:out & lockdays<2", "\"You're a confound now. Nothing personal. Everything is recorded.\"");
		Line(lines, "halcyon.route.out.1", HALCYON, "route", "", W, 0, "route:out & lockdays>=2", "\"Still with the other study. It's in the minutes. Underlined.\"");
		Line(lines, "halcyon.route.indep.1", HALCYON, "route", "", P, 0, "route:independent", "\"An uncontrolled variable. You understand why I'd rather you stood over there.\"");
		Line(lines, "halcyon.route.unsided.1", HALCYON, "route", "", P, 0, "route:unsided", "\"Here's a pen. You don't have to sign anything yet. The pen is for later.\"");
		Line(lines, "halcyon.route.lost.1", HALCYON, "route", "", P, 0, "route:lost & route:halcyon", "\"The result went against us. The method was sound. So were you.\"");
		Line(lines, "halcyon.route.winner.1", HALCYON, "route", "", P, 0, "route:lost & flag:finale.route=4", "\"Your objections are noted. Page two. Everyone reads page two eventually.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! What a guard says when a claim in their kit is looked at (docs/WRITING.md, "Faction kit and
	//! disguises"): a pass often has no line at all (the engine's pick), a catch says what they saw.
	protected static void Looks(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;

		Line(lines, "bastion.look.pass.1", BASTION, "look_pass", "", P, 0, "", "\"Right. Through you go.\"");
		Line(lines, "bastion.look.pass.2", BASTION, "look_pass", "", W, 0, "", "\"Didn't recognise you. Must be the light.\"");
		Line(lines, "bastion.look.caught.1", BASTION, "look_caught", "", P, 0, "", "\"That's not your uniform. Hands where I can see them.\"");
		Line(lines, "bastion.look.caught.2", BASTION, "look_caught", "", W, 0, "", "\"Nice uniform. Whose was it?\"");
		Line(lines, "redhand.look.pass.1", REDHAND, "look_pass", "", P, 0, "", "\"Alright. Off you trot.\"");
		Line(lines, "redhand.look.pass.2", REDHAND, "look_pass", "", J, 0, "", "\"Thought I knew everyone. Must be getting old. Go on.\"");
		Line(lines, "redhand.look.caught.1", REDHAND, "look_caught", "", J, 0, "", "\"Ha. Lads, look. Dressed up for us.\"");
		Line(lines, "redhand.look.caught.2", REDHAND, "look_caught", "", P, 0, "", "\"That armband was on someone else last week.\"");
		Line(lines, "halcyon.look.pass.1", HALCYON, "look_pass", "", P, 0, "", "\"Badge later. Go on.\"");
		Line(lines, "halcyon.look.caught.1", HALCYON, "look_caught", "", P, 0, "", "\"You're not on the rota. You're not on any rota.\"");
		Line(lines, "halcyon.look.caught.2", HALCYON, "look_caught", "", W, 0, "", "\"That coat has someone else's initials in it.\"");

		// Outside a conversation: the noticing line while a guard is turned to watch you (an again
		// look's 10 s notice), and straight to the guns at a post that would shoot you anyway
		Line(lines, "bastion.look.notice.1", BASTION, "look_notice", "", P, 0, "", "A sentry has turned your way. He's looking at your boots.");
		Line(lines, "bastion.look.notice.2", BASTION, "look_notice", "", P, 0, "", "The sentry stops talking and watches you walk.");
		Line(lines, "redhand.look.notice.1", REDHAND, "look_notice", "", P, 0, "", "One of the crew has turned round and is looking at your armband.");
		Line(lines, "redhand.look.notice.2", REDHAND, "look_notice", "", W, 0, "", "Someone at the fire is counting the faces. Yours is new.");
		Line(lines, "halcyon.look.notice.1", HALCYON, "look_notice", "", P, 0, "", "A guard at the door is reading the name on your coat.");
		Line(lines, "halcyon.look.notice.2", HALCYON, "look_notice", "", P, 0, "", "Someone with a clipboard has stopped writing and is watching you.");
		Line(lines, "bastion.look.guns.1", BASTION, "look_guns", "", P, 0, "", "\"Contact. That's not one of ours.\" Nobody asks you to stand still.");
		Line(lines, "redhand.look.guns.1", REDHAND, "look_guns", "", P, 0, "", "\"That's not our armband.\" Nobody asks twice.");
		Line(lines, "halcyon.look.guns.1", HALCYON, "look_guns", "", P, 0, "", "\"Unauthorised.\" The guards stop being polite.");
	}

	//------------------------------------------------------------------------------------------------
	//! The owed (Hunted with a hearing still owed: the parley, docs/REPUTATION.md), the word a leader
	//! sets ("Sergeant {word}"), said the way each faction says a rank, and the crew name at Champion,
	//! said politely because the book says so. {best}, {worst}, {word} and {crew} are filled for the
	//! listener (RZ_RepVoice.Opening).
	protected static void Owed(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;

		Line(lines, "haven.opener.owed.1", HAVEN, "opener", "owed", P, 0, "", "\"Put out, and prayed for. We still owe you a hearing. Sit on the step and say it.\"");
		Line(lines, "bastion.opener.owed.1", BASTION, "opener", "owed", P, 0, "", "\"Halt. Hands. You {best}. You also {worst}. Both go in the log. Talk.\"");
		Line(lines, "redhand.opener.owed.1", REDHAND, "opener", "owed", J, 0, "", "\"In the book, both pages. Lucky for you, we read both. Talk fast.\"");
		Line(lines, "redhand.opener.owed.2", REDHAND, "opener", "owed", P, 0, "", "\"You {best}. You also {worst}. The book keeps both. So do I.\"");
		Line(lines, "halcyon.opener.owed.1", HALCYON, "opener", "owed", P, 0, "", "\"Excluded, with thanks. The thanks means we owe you a hearing. One.\"");

		Line(lines, "haven.opener.setword.1", HAVEN, "opener", "", W, 0, "setword:haven & tier:haven>=known", "\"There's our {word}. Don't let it go to your head. It's a pencil title.\"");
		Line(lines, "bastion.opener.setword.1", BASTION, "opener", "", P, 0, "setword:bastion & tier:bastion>=known", "\"{word}. Report.\"");
		Line(lines, "redhand.opener.setword.1", REDHAND, "opener", "", W, 0, "setword:redhand & tier:redhand>=known", "\"{word}. It says so in the book. We remember who wrote it.\"");
		Line(lines, "halcyon.opener.setword.1", HALCYON, "opener", "", P, 0, "setword:halcyon & tier:halcyon>=known", "\"{word}. It's in the minutes now. Nobody edits the minutes.\"");
		Line(lines, "redhand.opener.crew.champion", REDHAND, "opener", "devoted", J, 0, "crew & tier:redhand=champion", "\"{crew}. The book says I'm to say it politely now. {crew}, sir.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The running jokes on the majors' flags (docs/WRITING.md, "Running jokes"): people's lines that
	//! run while a stance, a rule or the book stands, and stop when it changes. Never on a day.
	protected static void RunningJokes(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;

		Line(lines, "redhand.person.kettle", REDHAND, "person", "", J, 0, "flag:orders.stance<=1", "\"Bastion still calls Command every night. We keep a kettle on for whoever answers.\"");
		Line(lines, "redhand.person.expended", REDHAND, "person", "", J, 0, "flag:orders.stance=4", "\"Ask a Bastion boy what he is. Expended. We taught him the word, and he says it better than we do.\"");
		Line(lines, "bastion.person.call", BASTION, "person", "", W, 0, "flag:orders.stance<=1", "\"The call goes out at half eight. The operator writes the same two words after it, every night.\"");
		Line(lines, "bastion.person.stations", BASTION, "person", "", P, 0, "flag:orders.stance=2", "\"The CO reads the call to everyone now. Civilians first. Feels odd, saying it out loud.\"");
		Line(lines, "bastion.person.command", BASTION, "person", "", W, 0, "flag:orders.stance=3", "\"Command answers now. It sounds a lot like the CO, if you listen.\"");
		Line(lines, "haven.person.pot", HAVEN, "person", "", P, 0, "flag:haven.rule=1", "\"Everyone eats. It's written on the stores door, in the chair's handwriting.\"");
		Line(lines, "haven.person.workers", HAVEN, "person", "", P, 0, "flag:haven.rule=2", "\"Houses that work, eat. The rest go where there's room. We don't talk about where that is.\"");
		Line(lines, "haven.person.cards", HAVEN, "person", "", W, 0, "flag:haven.rule=3", "\"Bastion gave us cards with our names on. The children trade them like football stickers.\"");
		Line(lines, "haven.person.tins", HAVEN, "person", "", P, 0, "flag:haven.rule=4", "\"The tins come on Thursdays. Nobody asks who brings them.\"");
		Line(lines, "redhand.person.book", REDHAND, "person", "", J, 0, "flag:book.debts_dead=0", "\"Everything's in the book. Your name too, probably. Spelt wrong, but it's in.\"");
		Line(lines, "redhand.person.nobook", REDHAND, "person", "", P, 0, "flag:book.debts_dead=1", "\"No book now. People still pay at the toll. Nobody's told them they don't have to.\"");
		Line(lines, "halcyon.person.pages", HALCYON, "person", "", P, 0, "flag:majors.pages_open>=1", "\"The other strongholds have started reading their pages out loud. The director has noticed.\"");
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

		// A short safehouse's residents (RZ_Safehouses, Ask how things are): about the soup, the
		// potatoes and the chores, twice the mood's share, never about who eats; grief silences them
		Line(lines, "haven.house_short.1", HAVEN, "house_short", "", J, 0, "", "\"Potato soup today. Potato soup yesterday. Tomorrow's a surprise. It's potatoes.\"");
		Line(lines, "haven.house_short.2", HAVEN, "house_short", "", W, 0, "", "\"The soup's mostly water, but it's good water. We boiled it ourselves.\"");
		Line(lines, "haven.house_short.3", HAVEN, "house_short", "", J, 0, "", "\"New rota. Whoever peels, stirs. Whoever stirs, washes up. Whoever complains, peels.\"");
		Line(lines, "haven.house_short.4", HAVEN, "house_short", "", W, 0, "", "\"There's a stone in the soup pot. Old story. We're still waiting for it to work.\"");
		Line(lines, "haven.house_short.5", HAVEN, "house_short", "", W, 0, "", "\"Chores are the one thing we've plenty of. Help yourself.\"");
		Line(lines, "haven.house_short.6", HAVEN, "house_short", "", J, 0, "", "\"The children have started naming the potatoes. Makes supper awkward.\"");
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

	//------------------------------------------------------------------------------------------------
	//! The packs (kind "infected", Zombies/RZ_TribePeople.c): each faction's lines about them, by
	//! manner. {place} is the speaker's name for the pack with its place ("the Quarry lot, up at the
	//! quarry by Morton"), so four names are always one pack. Never "alpha", "tribe" or "crawler".
	protected static void Infected(array<ref RZ_VoiceLine> lines)
	{
		RZ_ERegister P = RZ_ERegister.PLAIN;
		RZ_ERegister W = RZ_ERegister.WRY;
		RZ_ERegister J = RZ_ERegister.JOKE;
		RZ_ERegister B = RZ_ERegister.BLEAK;

		Line(lines, "haven.infected.warn.1", HAVEN, "infected", "warn", P, 0, "", "\"Nobody on the roads after seven. {place} are out.\"");
		Line(lines, "haven.infected.warn.2", HAVEN, "infected", "warn", P, 0, "", "\"Bring the washing in early. {place} are hungry, and they've been in the fields.\"");
		Line(lines, "haven.infected.warn.3", HAVEN, "infected", "warn", B, 6, "", "\"{place} came to the hedge last night. Count the children before dark.\"");
		Line(lines, "haven.infected.roads.1", HAVEN, "infected", "roads", P, 0, "", "\"That's all we know. Ask again tomorrow, it changes.\"");
		Line(lines, "haven.infected.roads.2", HAVEN, "infected", "roads", W, 0, "", "\"Mind, we only know what came past the window.\"");
		Line(lines, "haven.infected.furious.1", HAVEN, "infected", "furious", P, 0, "", "\"Whatever was taken from {place}, they want it back. Stay off the lab road.\"");
		Line(lines, "haven.infected.moved.1", HAVEN, "infected", "moved", P, 0, "", "\"Bastion pushed {place} off their ground, and now they're on ours.\"");
		Line(lines, "bastion.infected.contact.1", BASTION, "infected", "contact", P, 0, "", "Contact, {place}. Logged. No reply. Bastion out.");
		Line(lines, "bastion.infected.contact.2", BASTION, "infected", "contact", W, 0, "", "New contact on the board: {place}. Command says monitor. Command says a lot of things. Bastion out.");
		Line(lines, "bastion.infected.cull.1", BASTION, "infected", "cull", P, 0, "", "{place}, culled. Not our sector now. Bastion out.");
		Line(lines, "redhand.infected.crew.1", REDHAND, "infected", "crew", W, 0, "", "\"{place}. Same as us: the boss is whoever's got something on the rest.\"");
		Line(lines, "redhand.infected.crew.2", REDHAND, "infected", "crew", J, 0, "", "\"{place} are on time again. More than I can say for you lot.\"");
		Line(lines, "redhand.infected.crew.3", REDHAND, "infected", "crew", P, 0, "", "\"{place} keep the soldiers honest. Don't feed them. That's our job.\"");
		Line(lines, "halcyon.infected.index.1", HALCYON, "infected", "index", P, 0, "", "\"{place}: the index is well. Please don't shoot it.\"");
		Line(lines, "halcyon.infected.index.2", HALCYON, "infected", "index", W, 0, "", "\"{place}: the index is doing better than any of us, frankly.\"");
		Line(lines, "halcyon.infected.sorry.1", HALCYON, "infected", "sorry", P, 0, "", "\"{place} took someone on the road last night. The index is in good condition. We're sorry.\"");
		Line(lines, "halcyon.infected.lost.1", HALCYON, "infected", "lost", B, 0, "", "\"The tag on {place} stopped moving. That was a year of data.\"");
	}
}
