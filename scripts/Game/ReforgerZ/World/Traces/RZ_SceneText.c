//! Every line Traces' scenes say (docs/TRACES.md, "Scenes"; docs/WRITING.md, the Traces row): the
//! approach line anyone gets once a session within a scene's radius, the one more line a reader's
//! background or standing gives (the ★ reads), and the paper a scene holds (opened in the reader
//! when you stand at it). One plain sentence and one detail; no gore, no child hurt, nothing about
//! the Long Table. Server only.
class RZ_SceneText
{
	//------------------------------------------------------------------------------------------------
	//! The notice's title for a scene's approach line.
	static string Title(string kind)
	{
		switch (kind)
		{
			case "F1": return "On the verge";
			case "F2": return "On the verge";
			case "F3": return "At the gym";
			case "F4": return "At the church";
			case "F5": return "On the verge";
			case "F6": return "In the square";
			case "F7": return "At the crossroads";
			case "F8": return "On the hill";
			case "N1": return "Near the line";
			case "N2": return "Near the line";
			case "V1": return "In a garden";
			case "V2": return "At a house";
			case "V3": return "At the edge of the village";
			case "V4": return "At the shop";
			case "V5": return "In the ditch";
			case "V6": return "At the clinic";
			case "V7": return "Where the post was";
			case "V8": return "At the pump";
		}

		return "By the road";
	}

	//------------------------------------------------------------------------------------------------
	//! What anyone sees, by the scene's stage (0 fresh, 1 weathered, 2 old; the first days by the
	//! outbreak's stage, 0 Early to 3 Aftermath).
	static string Approach(RZ_Scene s, int stage)
	{
		switch (s.kind)
		{
			case "F1":
			{
				if (stage >= 2)
					return "Cars on the verge, all pointing the same way, grass up through the wheels.";

				if (stage == 1)
					return "Cars on the verge, all pointing the same way. One of them is burnt out, and the suitcases are gone.";

				return "Cars on the verge, all pointing the same way, all the doors open. Nobody came back for them.";
			}
			case "F2":
			{
				if (stage >= 1)
					return "Suitcases across the verge, two of them opened. Somebody's been through them. Nothing worth taking.";

				return "Suitcases across the verge, set down neatly, then left.";
			}
			case "F3": return "A bear on the gym doors. It's been painted over so many times it has a patch on its belly.";
			case "F4": return "A chair by the church door, a blanket folded on it. Somebody sat up here all night once.";
			case "F5":
			{
				if (stage >= 2)
					return "A sandbag half ring on the verge, a barrier pole in the ditch, and a field radio on a crate, dead.";

				return "A sandbag half ring on the verge, a barrier pole in the ditch, and a field radio on a crate, still hissing.";
			}
			case "F6": return "A board by the square, with notices pasted over each other, day by day.";
			case "F7": return "A garage shutter with one red hand on it. Under the paint, smears, a lot of them, at hand height. The crews call it Rudi's.";
			case "F8": return "Somebody lay up here for a long time watching the road. On the post, cut with a knife: 05:41.";
			case "N1": return Crosses(s.a);
			case "N2": return "Sheets of paper on the verge and in the hedge, soaked and dried and soaked again.";
			case "V1":
			{
				if (stage >= 2)
					return "A table laid for four in a garden, the chairs fallen over. The soup never got served.";

				if (stage == 1)
					return "A table laid for four in a garden, the jug knocked over. The cloth has been rained on.";

				return "A table laid for four in a garden. Nobody got as far as the soup.";
			}
			case "V2": return "Planks across the downstairs windows, casings by the step, and a tally of days chalked on the door: 19.";
			case "V3": return "A chair at the edge of the village, facing out. Somebody sat with somebody here.";
			case "V4":
			{
				if (s.b > 0)
					return "IOUs on the shop door, a dozen hands, and across them all, in red: PAID.";

				return string.Format("IOUs on the shop door, a dozen hands: \"Two tins, bleach. Will pay when this is over. M., %1.\"", s.text);
			}
			case "V5": return "A sample case in the ditch, with the plague doctor on its lid. It's empty. The cold packs are warm.";
			case "V6": return "Six chairs in a row outside the clinic. A sign: WAIT HERE. DOCTOR BACK 1400. The last ticket on the roll says 31.";
			case "V7": return "A roster board, left behind. The names are still on it. Every one has a mark against it but six.";
			case "V8": return "Buckets in a line at the pump, each with a name on it. Nobody's queueing. The names are.";
			case "L1":
			{
				if (s.faction == RZ_EFaction.MILITARY)
					return string.Format("A stencil on a post: CONTACT P-%1 · 1 LOST · D%2.", s.a, s.b);

				return string.Format("A candle in chalk on the gatepost, and under it: \"%1. D%2.\"", s.text, s.b);
			}
			case "L2":
			{
				if (s.a > 0)
					return string.Format("Burned from the doorway out. Somebody chalked on the step: BURNED D%1. NOBODY INSIDE (WE CHECKED).", s.b);

				return "Burned from the doorway out. Black up the wall, and ash on the step.";
			}
			case "L5": return Truck(s);
			case "L6": return "Sandbags on the verge and a HALT sign on its side. Nobody's stood here for days.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Crosses(int n)
	{
		if (n <= 1)
			return "A cross of fence palings on the verge, facing the barrier. No name on it.";

		return string.Format("%1 crosses of fence palings on the verge. No names on most. One says TRIED. D4.", RZ_Util.Capitalize(RZ_WorldFacts.NumberWords(n)));
	}

	//------------------------------------------------------------------------------------------------
	//! L5: the truck, by its faction; a red hand if the crews robbed it, scratches if a pack took it.
	protected static string Truck(RZ_Scene s)
	{
		string whose = RZ_Factions.GetShortName(s.faction);
		string line = string.Format("A %1 truck on its side in the verge, stencil just readable: %2. The crates are empty.", whose, s.text);
		if (s.a == 1)
			line += " A red hand on the driver's door.";
		else if (s.a == 2)
			line += " Gouges down the cab, low.";

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! The one more line for this reader, "" if their background or standing gives none.
	static string More(RZ_Scene s, RZ_TraceReader r)
	{
		switch (s.kind)
		{
			case "F1":
			{
				if (r.Is("police"))
					return "The numbers on the dashboards run 18 to 40. Somebody was handing out tickets.";

				if (r.Is("hunter"))
					return "Old scratches down the side of the last car, low. Day 2 or 3.";

				return "";
			}
			case "F2":
			{
				if (r.Is("soldier"))
					return "From here you'd see the barrier. And the men behind it, in our badge.";

				return "";
			}
			case "F3":
			{
				if (r.Is("elder"))
					return "That's the teacher's writing. They all made it.";

				return "";
			}
			case "F4":
			{
				if (r.Is("nurse"))
					return "The chair faces the door, not the road. Whoever sat here was watching somebody inside.";

				return "";
			}
			case "F5":
			{
				if (r.Is("soldier"))
					return "\"Leaving the pole down\" means they meant to come back.";

				if (r.Is("radioham"))
					return "Still on the cordon net. Nothing on it but carrier.";

				return "";
			}
			case "F6":
			{
				if (r.Is("police"))
					return "The order underneath is signed Lt H. Sedlak. The name means something at Bastion's stronghold.";

				return "";
			}
			case "F7":
			{
				if (r.Is("excon"))
					return "Nobody parks in front of it.";

				return "";
			}
			case "F8":
			{
				if (r.Is("soldier"))
					return "An observation post. Two men, two days, by the tins.";

				if (r.m_bKnownBastion)
					return "Every post knows what happened at 05:41.";

				return "";
			}
			case "V1":
			{
				if (r.Is("hunter"))
					return "They left in a hurry, all at once, and nobody came back for the coats.";

				return "";
			}
			case "V2":
			{
				if (r.Is("police"))
					return "The casings are all one calibre. One gun, one person.";

				return "";
			}
			case "V3":
			{
				if (r.Is("nurse"))
					return "They sit with you until the end, in one house. In another, they walk you here at dusk.";

				return "";
			}
			case "V4":
			{
				if (r.Is("excon") && s.b > 0)
					return "That's the book's hand. The shop's in the book now.";

				return "";
			}
			case "V5":
			{
				if (r.Is("labtech"))
					return "They'll have written the case off by now. They'll have written the courier up.";

				return "";
			}
			case "V6":
			{
				if (r.Is("doctor"))
					return "1400 on which day, it doesn't say.";

				return "";
			}
			case "V7":
			{
				if (r.Is("soldier"))
					return "They took the tags with them. They'll read the names tonight anyway.";

				return "";
			}
			case "V8":
			{
				if (r.Is("elder"))
					return QueueNames(s);

				return "";
			}
			case "L1":
			{
				if (r.Is("hunter"))
					return "Drag marks in the grass toward the trees, two days old. Nobody followed them.";

				return "";
			}
			case "L2":
			{
				if (r.Is("police"))
					return "Started at the door, with fuel. Not an accident.";

				return "";
			}
			case "L5":
			{
				if (r.Is("mechanic") && s.a == 1)
					return "Shot through the radiator. It stopped here on purpose.";

				if (r.Is("mechanic") && s.a == 2)
					return "The doors are open. Nobody shot at anything.";

				return "";
			}
			case "L6":
			{
				if (r.Is("soldier"))
					return "They pulled the post. They took the kettle.";

				return "";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! V8, the Elder: two of the names on the buckets, seeded.
	protected static string QueueNames(RZ_Scene s)
	{
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + s.town * 53 + 7);
		string one = RZ_NoteText.FirstName(rng);
		string two = RZ_NoteText.FirstName(rng);
		if (two == one)
			two = RZ_NoteText.FirstName(rng);

		return string.Format("%1 and %2 on two of them. They take turns at the pump after dark.", one, two);
	}

	//------------------------------------------------------------------------------------------------
	//! The paper a scene holds, "" for none. It opens in the reader for someone standing at it.
	static string Paper(RZ_Scene s)
	{
		switch (s.kind)
		{
			case "F1": return "NUMBER 31. PROCEED TO NORTH ROAD CHECKPOINT FOR PROCESSING. BRING IDENTIFICATION. D2.\n\nIn biro, along the bottom: Mirror shot off the bus. Turned round 1400. Walking to the gym.";
			case "F3": return GymList();
			case "F4": return "CHURCH OPEN. ALL WELCOME. DECLARE YOUR BITE.";
			case "F5": return "D1 0410. Incident roadblock established.\n0600 traffic stopped, 14 vehicles turned.\n2210 two civilians attempted to pass on foot. Returned.\nD2 0600 ordered to the north road. Leaving the pole down.";
			case "F6": return Notices(s);
			case "N2": return "BY ORDER. THIS AREA IS UNDER QUARANTINE. REMAIN INDOORS. DO NOT APPROACH THE PERIMETER. D1.";
			case "V2": return "D19. Out of water. Going for the pump. Back by dark.";
			case "V5": return "Courier 4. Two samples, Pack P-02. Left 1700.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The paper's heading in the reader.
	static string PaperHow(string kind)
	{
		switch (kind)
		{
			case "F1": return "A sheet on a dashboard, sun-faded";
			case "F3": return "A list on the gym door";
			case "F4": return "A board leaning on the church wall";
			case "F5": return "The log sheet, under a ration tin";
			case "F6": return "Notices, newest on top";
			case "N2": return "A sheet out of the hedge";
			case "V2": return "A note on the step";
			case "V5": return "A page out of a notebook";
		}

		return "A paper";
	}

	//------------------------------------------------------------------------------------------------
	//! F3: eleven first names in a teacher's hand, each ticked; seeded, the same all season.
	protected static string GymList()
	{
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + 1102);
		string names = "";
		for (int i = 0; i < 11; i++)
		{
			names += RZ_NoteText.FirstName(rng) + "\n";
		}

		return "Eleven first names in a teacher's hand, a tick beside each:\n\n" + names + "\nAt the bottom: All at the house. Lock up after you.";
	}

	//------------------------------------------------------------------------------------------------
	//! F6: the square's notices, newest on top. A layer shows when its fact is true for this town
	//! (a: bits, worked out when the scene is made: 1 near the north road, 2 near the gym, 4 near
	//! the lab, 8 near the crossroads).
	protected static string Notices(RZ_Scene s)
	{
		string text = "On top, a government sheet: AID IS ON ITS WAY. Somebody has underlined AID twice, in another pen.";
		if ((s.a & 8) != 0)
			text += "\n\nIn red, half covered: DIESEL. ASK AT THE CROSSROADS.";

		if ((s.a & 4) != 0)
			text += "\n\nA printed flyer with a plague doctor in the corner: INFECTED SAMPLES WANTED.";

		if ((s.a & 2) != 0)
			text += "\n\nIn felt pen: GYM OPEN. BRING BLANKETS.";

		text += "\n\nALL RESIDENTS TO THE NORTH ROAD CHECKPOINT FOR PROCESSING. 0600. D2.";
		if ((s.a & 1) != 0)
			text += " Across it in marker: TURNED BACK 1400.";

		text += "\n\nAt the bottom, stuck to the board by the rain: INCIDENT IN THE HILLS. REMAIN INDOORS. D1.";
		return text;
	}

	// ================================================================ Sound and light
	//------------------------------------------------------------------------------------------------
	//! S3: the radio in an empty house, within 12 m: the broadcast's current line.
	static string Radio(string broadcast)
	{
		return "A radio somewhere in the house, under the hiss: " + broadcast;
	}

	//------------------------------------------------------------------------------------------------
	//! S3, the Radio Operator.
	static string RadioMore()
	{
		return "It's on the government frequency. Somebody left it on so the house wouldn't be quiet.";
	}

	//------------------------------------------------------------------------------------------------
	//! S4, a Hunter within 800 m.
	static string Dogs(string town)
	{
		return string.Format("The dogs in %1 have stopped.", town);
	}

	//------------------------------------------------------------------------------------------------
	//! S6, everyone within 1.2 km of the church.
	static string Bell()
	{
		return RZ_WorldFacts.Get().Fill("One stroke of a church bell, from the direction of {canon_town:day4.church.town}.");
	}

	//------------------------------------------------------------------------------------------------
	//! S7, the town loudspeaker, by Bastion's presence on the roads.
	static string Loudspeaker(bool presence)
	{
		if (presence)
			return "Remain indoors. Relief is on its way.";

		return "Remain indoors.";
	}

	//------------------------------------------------------------------------------------------------
	//! S7, a Soldier who hears it.
	static string LoudspeakerMore()
	{
		return "The man reading it has stopped believing it.";
	}

	// ================================================================ The epilogue
	//------------------------------------------------------------------------------------------------
	//! The census of a town's doors for the epilogue's town beats: "Four candles on the doors in
	//! Brod, and one bear." "" with nothing to count.
	static string Census(string town, array<RZ_MarkedDoor> doors)
	{
		int candles = 0;
		int bears = 0;
		int hands = 0;
		int beaks = 0;
		int scratches = 0;
		foreach (RZ_MarkedDoor d : doors)
		{
			foreach (RZ_DoorMark m : d.m_aLayers)
			{
				if (m.Has(RZ_DoorMark.F_OVER))
					continue;

				switch (m.m_iSymbol)
				{
					case RZ_EDoorSymbol.CANDLE: candles += Math.Max(1, m.m_iA); break;
					case RZ_EDoorSymbol.BEAR: bears++; break;
					case RZ_EDoorSymbol.PATCHED_BEAR: bears++; break;
					case RZ_EDoorSymbol.HAND: hands++; break;
					case RZ_EDoorSymbol.TALLY: hands++; break;
					case RZ_EDoorSymbol.BEAK: beaks++; break;
					case RZ_EDoorSymbol.SCRATCH_OUT: scratches++; break;
					case RZ_EDoorSymbol.SCRATCH_IN: scratches++; break;
				}
			}
		}

		array<string> parts = {};
		if (candles > 0)
			parts.Insert(CountOf(candles, "a candle", "candles"));

		if (bears > 0)
			parts.Insert(CountOf(bears, "a bear", "bears"));

		if (hands > 0)
			parts.Insert(CountOf(hands, "a red hand", "red hands"));

		if (beaks > 0)
			parts.Insert(CountOf(beaks, "a plague doctor", "plague doctors"));

		if (scratches > 0)
			parts.Insert(CountOf(scratches, "a door gouged", "doors gouged"));

		if (parts.IsEmpty())
			return "";

		string line = RZ_Util.Capitalize(parts[0]) + " on the doors in " + town;
		if (parts.Count() == 2)
			line += ", and " + parts[1];
		else if (parts.Count() > 2)
			line += ", " + parts[1] + ", and " + parts[2];

		return line + ".";
	}

	//------------------------------------------------------------------------------------------------
	protected static string CountOf(int n, string one, string many)
	{
		if (n == 1)
			return one;

		return RZ_WorldFacts.NumberWords(n) + " " + many;
	}
}
