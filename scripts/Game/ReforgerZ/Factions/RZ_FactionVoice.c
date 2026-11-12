//! What the factions say in the background (docs/WRITING.md, "The voices"): their outposts and
//! the people in them, routines, supply trucks, radio chatter, life events, truces and feuds,
//! remembered moments and rumours. Small tables, one voice per faction:
//!   Haven     warm, worried
//!   Bastion   clipped, procedural, tired, close with its own
//!   Red Hand  mocking, casual, always counting
//!   Halcyon   precise, a bit cold, sometimes guilty

//! What a faction needs most right now (RZ_FactionLife), from the state of the world.
enum RZ_ENeed
{
	NONE,
	FOOD,		// a hungry safehouse, a Haven truck lost
	SUPPLIES,	// trucks lost on the road
	SECURITY,	// people dead, an outpost lost
	SAMPLES,	// Halcyon while the cure isn't done
	PEOPLE,		// the Haven's safehouses are emptying
	REVENGE		// a feud, or a town taken from them
}

class RZ_FactionVoice
{
	// ================================================================ Holdings
	//------------------------------------------------------------------------------------------------
	//! "Bastion post", "Red Hand camp", "Haven farm", "Halcyon field station"; strongholds by name.
	static string HoldingKind(RZ_EFaction f, bool stronghold)
	{
		if (stronghold)
		{
			switch (f)
			{
				case RZ_EFaction.SURVIVORS: return "Haven stronghold";
				case RZ_EFaction.MILITARY: return "Bastion stronghold";
				case RZ_EFaction.BANDITS: return "Red Hand stronghold";
				case RZ_EFaction.INSTITUTE: return "Halcyon lab";
			}
		}

		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "Haven farm";
			case RZ_EFaction.MILITARY: return "Bastion post";
			case RZ_EFaction.BANDITS: return "Red Hand camp";
			case RZ_EFaction.INSTITUTE: return "Halcyon field station";
		}

		return "outpost";
	}

	//------------------------------------------------------------------------------------------------
	//! A guard or occupant's name, in the faction's style ("Private Emil Kral", "Dana Novak").
	static string PersonName(RZ_Rng rng, RZ_EFaction f)
	{
		string first = RZ_NoteText.FirstName(rng);
		string last = RZ_NoteText.Surname(rng);
		switch (f)
		{
			case RZ_EFaction.MILITARY:
			{
				array<string> ranks = {"Private", "Private", "Corporal"};
				return string.Format("%1 %2 %3", rng.Pick(ranks), first, last);
			}
			case RZ_EFaction.BANDITS:
			{
				array<string> nicks = {"Spoons", "Magpie", "the Vicar", "Lucky", "Knuckles", "Tin", "Mouse", "Crowbar"};
				return string.Format("%1 \"%2\"", first, rng.Pick(nicks));
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (rng.Chance(0.5))
					return "Dr " + first + " " + last;

				return first + " " + last;
			}
		}

		return first + " " + last;
	}

	//------------------------------------------------------------------------------------------------
	static string PersonRole(RZ_Rng rng, RZ_EFaction f)
	{
		array<string> roles = {};
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: roles.Insert("cook"); roles.Insert("lookout"); roles.Insert("farmer"); roles.Insert("nurse"); break;
			case RZ_EFaction.MILITARY: roles.Insert("sentry"); roles.Insert("radio operator"); roles.Insert("mechanic"); roles.Insert("medic"); break;
			case RZ_EFaction.BANDITS: roles.Insert("lookout"); roles.Insert("toll man"); roles.Insert("mechanic"); roles.Insert("cook"); break;
			case RZ_EFaction.INSTITUTE: roles.Insert("lab assistant"); roles.Insert("guard"); roles.Insert("technician"); break;
		}

		return rng.Pick(roles);
	}

	//------------------------------------------------------------------------------------------------
	//! One or two lines from a named guard or occupant, from the voice bank (Factions/RZ_VoiceDefaults.c,
	//! kinds person and aside_*). phase: RZ_Territory.PHASE_*. playerId and pos (optional) let the bank
	//! read grief near them and the player's joke memory.
	static string PersonLine(RZ_Rng rng, RZ_EFaction f, string role, int phase, string place, int playerId = 0, vector pos = "0 0 0")
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		string line = "";
		if (gm)
			line = RZ_VoiceBank.PickFor(gm, "person", f, "", null, playerId, pos, place);

		if (line.IsEmpty())
			line = "Can't stop, I'm on shift. Ask at the gate, they'll know.";

		string routine = PhaseAside(f, phase, playerId, pos);
		if (!routine.IsEmpty())
			line += " " + routine;

		return string.Format("The %1. %2", role, line);
	}

	//------------------------------------------------------------------------------------------------
	//! What a person says on top, at shift change, the evening meal or night watch.
	protected static string PhaseAside(RZ_EFaction f, int phase, int playerId, vector pos)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return "";

		string kind = "";
		if (phase == RZ_Territory.PHASE_MEAL)
			kind = "aside_meal";
		else if (phase == RZ_Territory.PHASE_NIGHT)
			kind = "aside_night";
		else if (phase == RZ_Territory.PHASE_MORNING || phase == RZ_Territory.PHASE_EVENING)
			kind = "aside_shift";

		if (kind.IsEmpty())
			return "";

		return RZ_VoiceBank.PickFor(gm, kind, f, "", null, playerId, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Atmosphere only, when the routine changes while you're near: nobody stops serving you.
	static string RoutineLine(RZ_Rng rng, RZ_EFaction f, int phase, string place)
	{
		if (phase == RZ_Territory.PHASE_MORNING || phase == RZ_Territory.PHASE_EVENING)
		{
			switch (f)
			{
				case RZ_EFaction.SURVIVORS: return "Somebody rings a hand bell: change of the watch at " + place + ". Coffee that isn't coffee goes round.";
				case RZ_EFaction.MILITARY: return "Someone calls the shift change twice at " + place + ". Boots on gravel, a two-minute handover, and the same report as yesterday: no contact, no relief, rations counted.";
				case RZ_EFaction.BANDITS: return "At " + place + " the night crew wakes the day crew with a kick and a joke.";
				case RZ_EFaction.INSTITUTE: return "At " + place + " someone reads out the overnight numbers. Nobody comments on them.";
			}
		}

		if (phase == RZ_Territory.PHASE_MEAL)
		{
			switch (f)
			{
				case RZ_EFaction.SURVIVORS: return "Supper at " + place + ". Before anyone eats, the council reads out a name, someone the house lost. Then the bowls go round, counted twice.";
				case RZ_EFaction.MILITARY: return "Evening mess at " + place + ". Somebody has found a tin of peaches and it's a whole event.";
				case RZ_EFaction.BANDITS: return "Dusk at " + place + ". A fire in a barrel, the toll tins stacked and counted, and a radio still tuned to a station that went off the air on Day 3.";
				case RZ_EFaction.INSTITUTE: return "Dinner at " + place + " is eaten standing up, next to the centrifuge. Somebody has to watch the incubators, so somebody always does.";
			}
		}

		if (phase == RZ_Territory.PHASE_NIGHT)
		{
			switch (f)
			{
				case RZ_EFaction.SURVIVORS: return "Night watch at " + place + ". Lamps go out one by one; the lookout stays.";
				case RZ_EFaction.MILITARY: return "The roll at " + place + ". The sergeant reads every name on the board. The living answer 'Here.' For the dead, the whole post answers together. Then the night watch.";
				case RZ_EFaction.BANDITS: return "Night at " + place + ". Somebody on the roof whistles a tune, badly, to show they're awake.";
				case RZ_EFaction.INSTITUTE: return "Night at " + place + ". The generator is switched to the freezers and the lights go dim.";
			}
		}

		return "";
	}

	// ================================================================ Territory headlines
	//------------------------------------------------------------------------------------------------
	static string Founded(RZ_EFaction f, string place, string leader)
	{
		string who = "";
		if (!leader.IsEmpty())
			who = " " + leader + " is in charge.";

		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "The Haven has opened a house at " + place + ": people in the fields, and a bear stencilled on the door." + who;
			case RZ_EFaction.MILITARY: return "Bastion has set up a post in " + place + ": sandbags on the road in, a roster on a board, and a curfew at dusk." + who;
			case RZ_EFaction.BANDITS: return "The Red Hand has moved into " + place + ". There's a toll on the road now, one tin a car." + who;
			case RZ_EFaction.INSTITUTE: return "Halcyon has opened a field station in " + place + ": sensors, a cold box and a radio in somebody's front room." + who;
		}

		return "Somebody holds " + place + " again. The lights are on at night." + who;
	}

	//------------------------------------------------------------------------------------------------
	static string Taken(RZ_EFaction winner, RZ_EFaction loser, string place)
	{
		return string.Format("%1 has driven %2 out of %3. Their things are still in the windows.", RZ_Factions.GetName(winner), RZ_Factions.GetNameInText(loser), place);
	}

	//------------------------------------------------------------------------------------------------
	static string Abandoned(RZ_EFaction f, string place)
	{
		return string.Format("%1 has pulled out of %2. Nobody shut the doors behind them.", RZ_Factions.GetName(f), place);
	}

	//------------------------------------------------------------------------------------------------
	static string PressureStart(RZ_EFaction f, string place)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "The Haven house in " + place + " is struggling: no trucks out, and the watch doubled at night.";
			case RZ_EFaction.MILITARY: return "Bastion's stronghold in " + place + " is under pressure: no convoys out, the armoury locked, and the sergeants counting rounds.";
			case RZ_EFaction.BANDITS: return "The Red Hand's people in " + place + " are on edge. The good stuff's gone under the floorboards.";
			case RZ_EFaction.INSTITUTE: return "Halcyon's lab in " + place + " is under pressure: no couriers out, and the generator saved for the freezers.";
		}

		return place + " is under pressure. The gates stay shut and nobody goes out after dark.";
	}

	//------------------------------------------------------------------------------------------------
	static string PressureEnd(RZ_EFaction f, string place)
	{
		return string.Format("%1 in %2 is holding again. The trucks are running and the gate is open by day.", RZ_Factions.GetName(f), place);
	}

	// ================================================================ Trucks
	//------------------------------------------------------------------------------------------------
	//! "Corporal Weber", "Sixpence", the person behind the wheel.
	static string DriverName(RZ_Rng rng, RZ_EFaction f)
	{
		return PersonName(rng, f);
	}

	//------------------------------------------------------------------------------------------------
	static string TruckArrives(RZ_EFaction f, string place, string driver)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("The Haven's truck pulls into %1. %2 climbs down, and people start passing sacks hand to hand.", place, driver);
			case RZ_EFaction.MILITARY: return string.Format("A Bastion supply truck parks at %1. %2 signs the manifest without reading it.", place, driver);
			case RZ_EFaction.BANDITS: return string.Format("A Red Hand truck rolls into %1, horn going. %2 is already arguing about the split.", place, driver);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon's van stops at %1. %2 carries the cool boxes in first.", place, driver);
		}

		return "A truck arrives at " + place + ", and people come out to see what's on it.";
	}

	//------------------------------------------------------------------------------------------------
	//! Told to players near the destination when its truck doesn't come.
	static string TruckLost(RZ_EFaction f, string place)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("The truck to %1 isn't coming. People are already rationing, and the children eat first.", place);
			case RZ_EFaction.MILITARY: return string.Format("The supply truck to %1 didn't make it. Prices are up, and Requisition is closed until the next one.", place);
			case RZ_EFaction.BANDITS: return string.Format("Somebody hit the Red Hand's truck to %1. Prices just went up, and somebody's going to pay for that.", place);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon's delivery to %1 never arrived. The lab is short of what was in it, and someone is rewriting the week's plan.", place);
		}

		return "The truck to " + place + " didn't arrive. Nobody knows yet whether it was the road or the people on it.";
	}

	// ================================================================ Life events
	//------------------------------------------------------------------------------------------------
	static string Funeral(RZ_EFaction f, string name, string place)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("The Haven buried %1 at %2. Someone sang. Nobody could remember all the words.", name, place);
			case RZ_EFaction.MILITARY: return string.Format("Bastion buried %1 outside %2. No salute: they're saving ammunition.", name, place);
			case RZ_EFaction.BANDITS: return string.Format("The Red Hand gave %1 a send-off in %2. Loud. Very loud. A horde came to see what the fuss was.", name, place);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon held a short service for %1 at %2. The director spoke for two minutes, and the notebook was read into the record.", name, place);
		}

		return name + " was buried at " + place + ". Somebody marked the grave with what they had.";
	}

	//------------------------------------------------------------------------------------------------
	static string Promotion(string oldName, string newName, string place)
	{
		return string.Format("At the Bastion post in %1, %2 is %3 now. The stripes were cut from a dead man's sleeve, but they're stripes.", place, oldName, newName);
	}

	//------------------------------------------------------------------------------------------------
	static string Recruiting(RZ_EFaction f, string place)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("The Haven is asking for volunteers for the night watch in %1. Soup for anyone who sits a shift.", place);
			case RZ_EFaction.MILITARY: return string.Format("Bastion is taking volunteers in %1. Rations and a rifle for anyone who can follow orders.", place);
			case RZ_EFaction.BANDITS: return string.Format("The Red Hand is recruiting in %1. Three new faces with red hands painted on, and they look pleased about it.", place);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon is looking for anyone who has worked in a lab, a pharmacy or a kitchen. Ask in %1. They're not fussy any more.", place);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string Quitting(string name, string place)
	{
		return string.Format("%1 walked out of Halcyon's lab in %2 with a bag and no goodbye. The note on the bench said: 'I can't do the next part.'", name, place);
	}

	// ================================================================ Truces and feuds
	//------------------------------------------------------------------------------------------------
	static string TruceStart(RZ_EFaction a, RZ_EFaction b)
	{
		return string.Format("%1 and %2 have agreed to leave each other alone, for now. Both sides are still counting the other's rifles.", RZ_Factions.GetName(a), RZ_Factions.GetNameInText(b));
	}

	//------------------------------------------------------------------------------------------------
	static string TruceEnd(RZ_EFaction a, RZ_EFaction b)
	{
		return string.Format("The truce between %1 and %2 is over. The road between them is closed again.", RZ_Factions.GetNameInText(a), RZ_Factions.GetNameInText(b));
	}

	//------------------------------------------------------------------------------------------------
	static string FeudStart(RZ_EFaction a, RZ_EFaction b)
	{
		return string.Format("Bad blood between %1 and %2. Each side remembers who ran its jobs, and will for a while.", RZ_Factions.GetNameInText(a), RZ_Factions.GetNameInText(b));
	}

	//------------------------------------------------------------------------------------------------
	static string FeudEnd(RZ_EFaction a, RZ_EFaction b)
	{
		return string.Format("Things have cooled between %1 and %2. The guns are back on the infected, mostly.", RZ_Factions.GetNameInText(a), RZ_Factions.GetNameInText(b));
	}

	// ================================================================ Needs
	//------------------------------------------------------------------------------------------------
	//! What a leader says the faction needs right now. "" for none.
	static string NeedLine(RZ_EFaction f, RZ_ENeed need)
	{
		switch (need)
		{
			case RZ_ENeed.FOOD: return "What we need is food. The trucks bring half what we eat, and the shops in town were stripped by Day 3. Tins, flour, anything.";
			case RZ_ENeed.PEOPLE: return "We're losing people faster than we find them. If you meet anyone hiding out there, bring them in. Every pair of hands counts.";
			case RZ_ENeed.SAMPLES: return "Samples. The cure moves when the lab gets tissue from the big ones, fresh, and stops when it doesn't. Bring them in.";
		}

		if (need == RZ_ENeed.SUPPLIES)
		{
			switch (f)
			{
				case RZ_EFaction.MILITARY: return "Priority is supply. We've lost trucks on the roads, and every name on the roster still has to eat. Anything that gets stock moving is work.";
				case RZ_EFaction.BANDITS: return "Somebody's been taking our trucks. Taking is our job. Find out who, and the book will remember you kindly.";
			}

			return "We've lost trucks on the road, and the shelves show it. Anything that gets supplies through helps.";
		}

		if (need == RZ_ENeed.SECURITY)
		{
			switch (f)
			{
				case RZ_EFaction.SURVIVORS: return "We've lost people on the roads, and the council reads more names at supper than it used to. We need anyone who'll stand a watch.";
				case RZ_EFaction.MILITARY: return "Security is the priority. We've had losses, and new tags on the nail. Hold ground, clear threats, bring our people back.";
				case RZ_EFaction.BANDITS: return "We've had losses. Bad for business. Bad for whoever did it, too.";
				case RZ_EFaction.INSTITUTE: return "We need protection. We've lost staff and couriers, and their notebooks are in the record now. Science doesn't happen behind a dead guard.";
			}
		}

		if (need == RZ_ENeed.REVENGE)
		{
			switch (f)
			{
				case RZ_EFaction.MILITARY: return "Someone has made an enemy of this task force. They're on the list, and work against them is approved.";
				case RZ_EFaction.BANDITS: return "Someone owes us, and it's in the book. Anyone who helps collect eats well.";
			}

			return "Someone has wronged us. I'm not proud of what I'd pay to see it put right.";
		}

		return "";
	}

	// ================================================================ Remembered moments
	//------------------------------------------------------------------------------------------------
	//! A reference to a big server moment. speaker -1 = a wanderer.
	static string MomentLine(RZ_Rng rng, int speaker, string phrase)
	{
		array<string> lines = {};
		switch (speaker)
		{
			case RZ_EFaction.SURVIVORS: lines.Insert("We haven't been this worried since %1."); lines.Insert("People still light a candle for %1."); break;
			case RZ_EFaction.MILITARY: lines.Insert("Not since %1. The roll got longer that night."); lines.Insert("We train for nights like %1. Didn't help."); break;
			case RZ_EFaction.BANDITS: lines.Insert("Quietest it's been since %1. Don't like it."); lines.Insert("Best night we've had since %1. Or worst. Hard to tell."); break;
			case RZ_EFaction.INSTITUTE: lines.Insert("Nothing like it since %1. It's in three notebooks."); lines.Insert("We still have the data from %1. Nobody reads it twice."); break;
		}

		if (lines.IsEmpty())
		{
			lines.Insert("People still talk about %1.");
			lines.Insert("Haven't seen the like since %1.");
		}

		return string.Format(rng.Pick(lines), phrase);
	}

	// ================================================================ Rumours
	//------------------------------------------------------------------------------------------------
	//! "They say...", told as gossip, true or not.
	static string RumourStrong(RZ_EFaction subject)
	{
		return string.Format("They say %1 is stronger than it's been in weeks. More guns on the gates, more trucks on the road.", RZ_Factions.GetNameInText(subject));
	}

	//------------------------------------------------------------------------------------------------
	static string RumourWeak(RZ_EFaction subject)
	{
		return string.Format("Word is %1 is on its last legs. Pulling people back from the small towns to save the big one.", RZ_Factions.GetNameInText(subject));
	}

	//------------------------------------------------------------------------------------------------
	static string RumourHolds(RZ_EFaction subject, string place)
	{
		return string.Format("%1 is holding %2, last I heard. Could have changed since. Things do.", RZ_Factions.GetName(subject), place);
	}

	//------------------------------------------------------------------------------------------------
	static string RumourLost(RZ_EFaction subject, string place)
	{
		return string.Format("I heard %1 got thrown out of %2. Good riddance.", RZ_Factions.GetNameInText(subject), place);
	}

	//------------------------------------------------------------------------------------------------
	static string RumourTruck(RZ_EFaction subject, string place)
	{
		return string.Format("There's a %1 truck heading for %2. Someone should do something about that. Or for it.", RZ_Factions.GetShortName(subject), place);
	}

	//------------------------------------------------------------------------------------------------
	static string RumourSmear(RZ_Rng rng, RZ_EFaction subject)
	{
		array<string> lines = {};
		switch (subject)
		{
			case RZ_EFaction.SURVIVORS: lines.Insert("The Haven waters the soup and sells the rest to the Red Hand. A woman in a queue told me."); lines.Insert("They say the Haven turns bitten people out at night, and lies about it."); break;
			case RZ_EFaction.MILITARY: lines.Insert("Bastion's had a radio to the outside all along. They just don't like what it says."); lines.Insert("Bastion shoots its own deserters. Writes them up as killed in action, and sends someone after them."); break;
			case RZ_EFaction.BANDITS: lines.Insert("The Red Hand poisoned the well at the last place that wouldn't pay."); lines.Insert("Red Hand trucks carry people, not tins. The drivers are paid not to ask where."); break;
			case RZ_EFaction.INSTITUTE: lines.Insert("Halcyon made this. The lab's not curing it, it's finishing it."); lines.Insert("Halcyon's paying the Red Hand for live ones. For tests."); break;
		}

		return rng.Pick(lines);
	}

	//------------------------------------------------------------------------------------------------
	static string RumourNeed(RZ_EFaction subject, RZ_ENeed need)
	{
		switch (need)
		{
			case RZ_ENeed.FOOD: return string.Format("%1 is short of food. They'd pay for tins, and pay well.", RZ_Factions.GetName(subject));
			case RZ_ENeed.SUPPLIES: return string.Format("%1 has been losing trucks. They'll pay for anyone who gets supplies through.", RZ_Factions.GetName(subject));
			case RZ_ENeed.SECURITY: return string.Format("%1 has lost people lately. They're nervous. Don't make sudden moves near them.", RZ_Factions.GetName(subject));
			case RZ_ENeed.SAMPLES: return string.Format("%1 wants samples, more than ever. Fresh, from the big ones, in a cold box.", RZ_Factions.GetName(subject));
			case RZ_ENeed.PEOPLE: return string.Format("%1 is taking in anyone who can stand a watch. Bring them whoever you find hiding.", RZ_Factions.GetName(subject));
			case RZ_ENeed.REVENGE: return string.Format("%1 has a score to settle. Somebody's going to get hurt.", RZ_Factions.GetName(subject));
		}

		return "";
	}

	// ================================================================ Radio chatter (open channels)
	//------------------------------------------------------------------------------------------------
	static string ChatterTruck(RZ_EFaction f, string from, int minutes)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("Haven, open channel. The truck leaves %1 in about %2 minutes, with food for the safehouses. If you're on the road, give it room.", from, minutes);
			case RZ_EFaction.MILITARY: return string.Format("All stations, Bastion. Supply run departs %1 in figures %2 minutes. Escorts welcome. Bastion out.", from, minutes);
			case RZ_EFaction.BANDITS: return string.Format("Truck's going out of %1 in %2 minutes or so. Anyone touches it, we'll know.", from, minutes);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon logistics. Next delivery leaves %1 in approximately %2 minutes. Cold boxes aboard. Please keep the road clear.", from, minutes);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string ChatterTruckOnRoad(RZ_EFaction f, string dest)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("Haven truck's on its way to %1. Keep the door open, love.", dest);
			case RZ_EFaction.MILITARY: return string.Format("Bastion, convoy en route to %1. Report contacts on that road.", dest);
			case RZ_EFaction.BANDITS: return string.Format("Our truck's headed for %1. Bastion, don't wave.", dest);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon. Delivery en route to %1, reagents and cold boxes. Please keep the road clear.", dest);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string ChatterShift(RZ_EFaction f, string place, string clock)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return string.Format("Post %1, Bastion. Night watch rotates at %2. Acknowledge.", place, clock);
			case RZ_EFaction.SURVIVORS: return string.Format("Night watch at %1 starts at %2. Bring a coat, it gets cold on the roof.", place, clock);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon, field station %1. Overnight readings start at %2. Generator to the freezers from then. Do not disturb.", place, clock);
		}

		return string.Format("Crew at %1 swaps at %2. Don't be late or don't come back.", place, clock);
	}

	//------------------------------------------------------------------------------------------------
	static string ChatterToll(string place, bool night)
	{
		if (night)
			return string.Format("Toll's up on the road out of %1 tonight. Pay or walk round. Walking round is slower. And it's dark.", place);

		return string.Format("Road past %1 is tolled till dusk. One tin a car, or fuel, or something worth knowing. We prefer the tins.", place);
	}

	//------------------------------------------------------------------------------------------------
	static string ChatterSamples(int percent, string labPlace)
	{
		return string.Format("Halcyon, open channel. Research stands at %1 percent. Samples from special infected are wanted at the lab in %2. Fresh ones, please.", percent, labPlace);
	}

	//------------------------------------------------------------------------------------------------
	static string ChatterShort(RZ_EFaction f, string place)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("Haven, open channel. %1 is short. Anything in a tin, anything at all.", place);
			case RZ_EFaction.MILITARY: return string.Format("Bastion. Post %1 is on short supply. Requisitions suspended there until resupply.", place);
			case RZ_EFaction.BANDITS: return string.Format("%1's dry. Prices are up. Blame whoever hit our truck.", place);
			case RZ_EFaction.INSTITUTE: return string.Format("Halcyon. %1 is short of supplies until the next van. Some stock won't be available till then. We apologise.", place);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string ChannelTitle(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "Radio: the Haven";
			case RZ_EFaction.MILITARY: return "Radio: Bastion net";
			case RZ_EFaction.BANDITS: return "Radio: open channel";
			case RZ_EFaction.INSTITUTE: return "Radio: Halcyon";
		}

		return "Radio";
	}

	// ================================================================ Retaliation
	//------------------------------------------------------------------------------------------------
	//! Told when a faction's people near you turn on you for shooting at them.
	static string Retaliation(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "Somebody shouts that you're shooting at the house. The Haven's people nearby take cover, and the ones with guns point them at you. Keep away for a few minutes.";
			case RZ_EFaction.MILITARY: return "Contact, hostile civilian: the call goes down the line, and every Bastion rifle nearby is on you now. Get clear and give them a few minutes.";
			case RZ_EFaction.BANDITS: return "Oh, you've done it now, somebody laughs. The Red Hand's people nearby are coming for you. It'll blow over. Probably.";
			case RZ_EFaction.INSTITUTE: return "Halcyon's guards nearby open fire on you. They were told the lab comes first. They'll calm down in a few minutes, if you're gone.";
		}

		return "The people nearby turn on you. Give them a few minutes, and give them room.";
	}

	// ================================================================ Camps (RZ_Checkpoints)
	//------------------------------------------------------------------------------------------------
	//! A camp warns off someone the faction watches.
	static string CampWarning(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "That's close enough. You're on a list here. Turn around, or come back when you've made it right.";
			case RZ_EFaction.BANDITS: return "Oi. We know you, and so does the book. You're not welcome here till it balances. Keep walking.";
			case RZ_EFaction.INSTITUTE: return "Stop there, please. The director has closed the lab to you, and the guards have your description. Please don't make this difficult.";
		}

		return "That's close enough. Whatever you did here, they remember it.";
	}

	//------------------------------------------------------------------------------------------------
	//! Coming into a stronghold or the lab: the rule of the place.
	static string SafeZoneRule(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "A stencilled sign on the gate: WEAPONS SLUNG. NO FIRING INSIDE THE WIRE. Somebody has added, in pencil: we mean it.";
			case RZ_EFaction.BANDITS: return "Guns stay quiet in here. You want to shoot someone, do it outside. House rule.";
			case RZ_EFaction.INSTITUTE: return "A laminated notice: NO FIREARMS DISCHARGED WITHIN THE SITE. They will close the doors on you, and worse.";
		}

		return "No shooting here. Shots bring the infected, and everyone here has to live with them.";
	}
}
