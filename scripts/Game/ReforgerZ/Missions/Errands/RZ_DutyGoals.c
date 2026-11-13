//! A scene duty's steps for the tracker (docs/ERRANDS.md, "The duties"; docs/WRITING.md, "Objective
//! lines"): one objective for each thing done at one place, in the order the scene plays them. Each
//! goal is put on the beat that starts its step (RZ_DutyBeat.m_sGoal), once the scene is built, so
//! the scenes' own files stay as they are. RZ_DutyScenes.Objectives turns them into the tracker's
//! lines: the steps behind ticked, the one under way bright at its beat, the rest dim, then the way
//! back to the giver. A duty that's one thing at one place (a watch, the toll bar, a table of
//! things to sort) has no goals: its step line is the objective.

//------------------------------------------------------------------------------------------------
class RZ_DutyGoals
{
	//------------------------------------------------------------------------------------------------
	//! Put the goals of the duty `duty` (its scene key; "names" by faction) on its built scene.
	static void Set(string duty, int faction, RZ_DutyScene s)
	{
		if (!s)
			return;

		switch (duty)
		{
			case "school":
			{
				On(s, "count", "Count the heads in the school room at {here}.");
				On(s, "look", "Find the child missing from the benches at {here}.");
				return;
			}

			case "doorround":
			{
				On(s, "round", "Knock and listen at each of the four doors on the round from {here}.");
				On(s, "chalk", "Chalk the door at {here} where somebody lives unannounced.");
				return;
			}

			case "rollcall":
			{
				On(s, "looks", "Find out who of {here} is on the list and wasn't fed.");
				On(s, "who", "Write why against the missing name on the list at {here}.");
				On(s, "found", "Look along the wood road from {here} for the missing one.");
				return;
			}

			case "samples":
			{
				On(s, "round", "Take a swab at each of the four marked spots out from {here}.");
				On(s, "source", "Mark the infection's source on the station's map at {here}.");
				return;
			}

			case "decon":
			{
				On(s, "", "Check each of the five who come through the decon line at {here}.");
				On(s, "close", "Close the decon line at the door of {here}.");
				return;
			}

			case "incinerator":
			{
				On(s, "", "Burn, report or open each of the four sealed bags at the pit behind {here}.");
				On(s, "log", "Settle the burn log at the pit behind {here}.");
				return;
			}

			case "ward":
			{
				On(s, "", "Read the signs at each of the three beds on the ward at {here}.");
				On(s, "chart", "Write up the ward's charts at the door of the ward at {here}.");
				return;
			}

			case "coldchain":
			{
				On(s, "take", "Take the cold box from the cold room at {here}.");
				On(s, "@1", "Carry the cold box to the far door, keeping to its rule on the road.");
				On(s, "dark", "Hand over the cold box at the far door.");
				On(s, "nobody", "Hand over the cold box at the far door.");
				On(s, "short", "Hand over the cold box at the far door.");
				On(s, "sign", "Sign the cold box's manifest at the far door.");
				return;
			}

			case "wreck":
			{
				On(s, "wheels", "Strip the wreck in the ditch beyond {here} by hand.");
				On(s, "load", "Load the handcart at the wreck beyond {here}.");
				return;
			}

			case "still":
			{
				On(s, "mash", "Load the still behind {here} and fire it.");
				On(s, "gauge", "Keep the still's needle in its band behind {here}.");
				On(s, "cut", "Pour off the still's first cup behind {here}.");
				On(s, "bottle", "Bottle the rest of the batch at the still behind {here}.");
				return;
			}

			case "count":
			{
				On(s, "", "Sort each of the four things off the tarp at {here}.");
				On(s, "short", "Name to the boss at {here} whoever shorted the tally.");
				return;
			}

			case "spotter":
			{
				On(s, "climb", "Lie up with the glass on the hill beyond {here} and watch.");
				On(s, "board", "Mark on the slate at the hill beyond {here} what the crew can use.");
				return;
			}

			case "tape":
			{
				On(s, "pole", "Go out to the loudspeaker on its pole beyond {here}.");
				On(s, "swap", "Change the tape in the loudspeaker beyond {here}.");
				On(s, "test", "Test the loudspeaker on its pole beyond {here}.");
				return;
			}

			case "kits":
			{
				On(s, "", "Pass, write up or deal with each of the three kits at {here}.");
				On(s, "rifle", "Clean the spare rifle at {here}.");
				return;
			}

			case "telephone":
			{
				On(s, "start", "Test the field telephone at the gate of {here}.");
				On(s, "@1", "Walk the telephone wire out from {here} to its break.");
				On(s, "splice", "Splice the telephone wire at its break.");
				On(s, "ring", "Ring through on the telephone at the splice.");
				return;
			}

			case "list":
			{
				On(s, "chart0", "Read the chart at each of the three beds at {here}.");
				On(s, "list", "Write one name on the trial list by the door of {here}.");
				return;
			}

			case "dose":
			{
				On(s, "", "Read each of the three beds at {here}.");
				On(s, "dose", "Write one name on the card in the dose case at {here}.");
				return;
			}

			case "animals":
			{
				On(s, "", "Read each of the three cages in the animal house at {here}.");
				On(s, "log", "Sign the animal house log at {here}.");
				return;
			}

			case "names":
			{
				if (faction == RZ_EFaction.MILITARY)
				{
					On(s, "", "Read what was left at each of the three bunks at {here}.");
					On(s, "sign", "Sign the casualty return at {here}.");
					return;
				}

				if (faction == RZ_EFaction.BANDITS)
				{
					On(s, "", "Go to the door of each of the three in the boss's debt book near {here}.");
					On(s, "book", "Press your thumb under the lines in the boss's debt book at {here}.");
					return;
				}

				if (faction == RZ_EFaction.INSTITUTE)
				{
					On(s, "", "Read each of the three desks at {here} for the week's report.");
					On(s, "report", "Initial the week's report on the director's tray at {here}.");
					return;
				}

				On(s, "", "Tell the people at the door of each of the dead of {here}.");
				On(s, "book", "Write the names in the book by the stove at {here}.");
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The goal on the beat named `key`: "" the first beat, "@n" the beat at index n, else the
	//! first beat with that key. Nothing if there's no such beat (a beat the draw left out).
	protected static void On(RZ_DutyScene s, string key, string goal)
	{
		int at = -1;
		if (key.IsEmpty())
		{
			at = 0;
		}
		else if (key.StartsWith("@"))
		{
			at = key.Substring(1, key.Length() - 1).ToInt();
		}
		else
		{
			foreach (int i, RZ_DutyBeat b : s.m_aBeats)
			{
				if (b.m_sKey == key)
				{
					at = i;
					break;
				}
			}
		}

		if (at < 0 || at >= s.m_aBeats.Count())
			return;

		if (s.m_aBeats[at].m_sGoal.IsEmpty())
			s.m_aBeats[at].m_sGoal = goal;
	}
}
