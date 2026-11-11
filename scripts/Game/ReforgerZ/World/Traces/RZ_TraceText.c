//! Every line Traces says (docs/TRACES.md, "Reading"; docs/WRITING.md, "Notices and boards"):
//! the door lines by level, the town summary, residents' words, and the player traces' lines.
//!
//! Three levels. Glance: what anyone sees. Read: what the writer meant, for someone who knows that
//! writer (by background, or Known with them). Know: one more true detail only training or a life
//! gives. Every written mark carries its day: "D17" on the mark, "Day 17" in prose.
//! Server only.

//------------------------------------------------------------------------------------------------
//! Who is reading: their background and standing, once per reading.
class RZ_TraceReader
{
	int m_iPlayer = -1;
	string m_sBackground;
	bool m_bKnownBastion;
	bool m_bKnownHaven;
	bool m_bKnownRedHand;
	bool m_bKnownHalcyon;

	//------------------------------------------------------------------------------------------------
	static RZ_TraceReader For(RZ_GameModeComponent gm, int playerId)
	{
		RZ_TraceReader r = new RZ_TraceReader();
		r.m_iPlayer = playerId;
		if (!gm || playerId <= 0)
			return r;

		RZ_Background bg = gm.GetPlayerBackground(playerId);
		if (bg)
			r.m_sBackground = bg.m_sId;

		r.m_bKnownBastion = RZ_Reputation.GetAccessRank(gm.GetTier(playerId, RZ_EFaction.MILITARY)) >= 1;
		r.m_bKnownHaven = RZ_Reputation.GetAccessRank(gm.GetTier(playerId, RZ_EFaction.SURVIVORS)) >= 1;
		r.m_bKnownRedHand = RZ_Reputation.GetAccessRank(gm.GetTier(playerId, RZ_EFaction.BANDITS)) >= 1;
		r.m_bKnownHalcyon = RZ_Reputation.GetAccessRank(gm.GetTier(playerId, RZ_EFaction.INSTITUTE)) >= 1;
		return r;
	}

	//------------------------------------------------------------------------------------------------
	bool Is(string id)
	{
		return m_sBackground == id;
	}

	//------------------------------------------------------------------------------------------------
	//! 0 glance, 1 read, 2 know, for marks by `author` (the reading table).
	int Level(int author)
	{
		switch (author)
		{
			case RZ_EDoorAuthor.BASTION:
			{
				if (Is("police") || Is("soldier"))
					return 2;

				if (m_bKnownBastion)
					return 1;

				return 0;
			}
			case RZ_EDoorAuthor.HAVEN:
			{
				if (Is("nurse") || Is("elder"))
					return 2;

				if (Is("police") || m_bKnownHaven)
					return 1;

				return 0;
			}
			case RZ_EDoorAuthor.REDHAND:
			{
				if (Is("excon"))
					return 2;

				if (Is("raider") || Is("police") || m_bKnownRedHand)
					return 1;

				return 0;
			}
			case RZ_EDoorAuthor.HALCYON:
			{
				if (Is("labtech"))
					return 2;

				if (Is("doctor") || Is("police") || m_bKnownHalcyon)
					return 1;

				return 0;
			}
			case RZ_EDoorAuthor.INFECTED:
			{
				if (Is("hunter"))
					return 2;

				return 0;
			}
		}

		// Residents' words: everyone reads them; a Police Officer knows whether the day fits
		if (Is("police"))
			return 2;

		return 1;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_TraceText
{
	// ================================================================ Bastion's search code
	//------------------------------------------------------------------------------------------------
	static string Unit(int i)
	{
		array<string> units = {"1A", "1B", "2A", "2B", "3C", "HQ"};
		return units[Math.ClampInt(i, 0, units.Count() - 1)];
	}

	//------------------------------------------------------------------------------------------------
	//! The right quadrant. 0 none, 1 INF, 2 BIT, 3 SEAL, 4 NE, 5 DOG.
	static string HazardCode(int h)
	{
		array<string> codes = {"–", "INF", "BIT", "SEAL", "NE", "DOG"};
		return codes[Math.ClampInt(h, 0, codes.Count() - 1)];
	}

	//------------------------------------------------------------------------------------------------
	protected static string HazardWords(int h)
	{
		switch (h)
		{
			case 1: return " Infected inside.";
			case 2: return " A bitten person inside.";
			case 3: return " Sealed.";
			case 4: return " Barricaded from inside.";
			case 5: return " A dog.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Where the living were sent: 0 nowhere, 1 the north road, 2 the gym, 3 the church.
	protected static string DestCode(int d)
	{
		array<string> codes = {"", "NR", "GYM", "CH"};
		return codes[Math.ClampInt(d, 0, codes.Count() - 1)];
	}

	//------------------------------------------------------------------------------------------------
	protected static string DestWords(int d)
	{
		array<string> words = {"away", "the north road", "the gym", "the church"};
		return words[Math.ClampInt(d, 0, words.Count() - 1)];
	}

	//------------------------------------------------------------------------------------------------
	//! The X's fields packed in C: hazard * 1000 + living * 100 + dead * 10 + where the living went.
	static int PackFound(int hazard, int living, int dead, int dest)
	{
		return hazard * 1000 + living * 100 + dead * 10 + dest;
	}

	//------------------------------------------------------------------------------------------------
	//! "D2 0740 · 1A · – · 3L→NR"
	protected static string XCode(RZ_DoorMark m, bool locked)
	{
		int hazard = m.m_iC / 1000;
		if (locked)
			hazard = 3;

		int living = (m.m_iC / 100) % 10;
		int dead = (m.m_iC / 10) % 10;
		int dest = m.m_iC % 10;
		string found = "0";
		if (living > 0)
			found = string.Format("%1L→%2", living, DestCode(dest));

		if (dead > 0)
		{
			if (living > 0)
				found += string.Format(" %1D", dead);
			else
				found = string.Format("%1D", dead);
		}

		return string.Format("D%1 %2 · %3 · %4 · %5", m.m_iDay, Clock(m.m_iA), Unit(m.m_iB), HazardCode(hazard), found);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Clock(int hhmm)
	{
		string s = hhmm.ToString();
		while (s.Length() < 4)
		{
			s = "0" + s;
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected static string ClockWords(int hhmm)
	{
		string s = Clock(hhmm);
		return s.Substring(0, 2) + ":" + s.Substring(2, 2);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Num(int n)
	{
		return RZ_WorldFacts.NumberWords(n);
	}

	//------------------------------------------------------------------------------------------------
	protected static string CapNum(int n)
	{
		return RZ_Util.Capitalize(RZ_WorldFacts.NumberWords(n));
	}

	//------------------------------------------------------------------------------------------------
	protected static string TownName(int idx)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return "";

		RZ_Settlement s = gm.GetSettlement(idx);
		if (!s)
			return "";

		return s.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! "→ M": the first letter of the town the arrow points to (a + 1 in the fields; 0 none).
	protected static string ArrowLetter(int townPlusOne)
	{
		string name = TownName(townPlusOne - 1);
		if (name.IsEmpty())
			return "?";

		string first = name.Substring(0, 1);
		first.ToUpper();
		return first;
	}

	// ================================================================ One layer
	//------------------------------------------------------------------------------------------------
	//! A layer's words at `level` (the reader's level for its author). sealed: the door's building
	//! is one the lock system locked (the X says SEAL then, always true).
	static string DoorLayer(RZ_DoorMark m, int level, RZ_TraceReader r, RZ_MarkedDoor d, float ageDays, int stage)
	{
		string s = DoorGlance(m, d);
		if (level >= 1)
		{
			string read = DoorRead(m, d);
			if (!read.IsEmpty())
				s = read;
		}

		if (level >= 2)
		{
			string know = DoorKnow(m, r, d, ageDays);
			if (!know.IsEmpty())
				s += " " + know;
		}

		if (m.Has(RZ_DoorMark.F_OVER))
			return "Under the paint, still readable: " + s;

		return s + AgeWords(m, stage);
	}

	//------------------------------------------------------------------------------------------------
	//! What anyone sees.
	static string DoorGlance(RZ_DoorMark m, RZ_MarkedDoor d)
	{
		switch (m.m_iSymbol)
		{
			case RZ_EDoorSymbol.X:
			{
				if (m.Has(RZ_DoorMark.F_RING))
					return string.Format("An orange X on the door with writing in its corners, and a ring round it. Day %1.", m.m_iDay);

				return string.Format("An orange X on the door, with writing in its corners. Day %1.", m.m_iDay);
			}
			case RZ_EDoorSymbol.SLASH: return string.Format("One orange stroke on the door, where an X would be: \"D%1 / %2\".", m.m_iDay, Unit(m.m_iB));
			case RZ_EDoorSymbol.POSTCODE: return string.Format("A black stencil: B-%1 · D%2.", m.m_iA, m.m_iDay);
			case RZ_EDoorSymbol.CONTRIB: return string.Format("A small black stencil: C · D%1.", m.m_iDay);
			case RZ_EDoorSymbol.ATT: return "A black stencil: ATT · DO NOT REQUISITION.";
			case RZ_EDoorSymbol.CLEARED:
			{
				if (m.Has(RZ_DoorMark.F_GREY))
					return string.Format("Grey paint over something red, and a black stencil on top: CLEARED · %1 · D%2.", Clock(m.m_iA), m.m_iDay);

				return string.Format("A black stencil: CLEARED · %1 · D%2.", Clock(m.m_iA), m.m_iDay);
			}
			case RZ_EDoorSymbol.BEAR:
			{
				if (m.Has(RZ_DoorMark.F_TICK) && m.m_iA > 0)
					return string.Format("A bear on the door with a tick through it, and an arrow: → %1.", ArrowLetter(m.m_iA));

				if (m.Has(RZ_DoorMark.F_TICK))
					return string.Format("A bear on the door with a tick through it. D%1.", m.m_iDay);

				if (m.Has(RZ_DoorMark.F_RING) && m.m_iA > 0)
					return string.Format("A bear in white paint, with a chalk ring round it and an arrow: → %1.", ArrowLetter(m.m_iA));

				if (m.Has(RZ_DoorMark.F_RING))
					return string.Format("A bear in white paint, with a chalk ring round it. D%1.", m.m_iDay);

				return string.Format("A bear in white paint. D%1.", m.m_iDay);
			}
			case RZ_EDoorSymbol.PATCHED_BEAR:
			{
				if (m.Has(RZ_DoorMark.F_RING) && m.m_iA > 0)
					return string.Format("A bear with a patch on its belly, a chalk ring round it, and an arrow: → %1.", ArrowLetter(m.m_iA));

				if (m.Has(RZ_DoorMark.F_RING))
					return string.Format("A bear with a patch on its belly and a chalk ring round it. D%1.", m.m_iDay);

				return string.Format("A bear with a patch on its belly and one eye stitched. D%1.", m.m_iDay);
			}
			case RZ_EDoorSymbol.CANDLE:
			{
				if (m.Has(RZ_DoorMark.F_SHOWN) && !m.m_sText.IsEmpty())
					return string.Format("A candle in chalk, and small underneath: \"%1. D%2.\"", m.m_sText, m.m_iDay);

				return string.Format("A candle in chalk, and a %1 under it.", m.m_iA);
			}
			case RZ_EDoorSymbol.HATCH: return string.Format("Hatching across the door frame, like a gate. D%1.", m.m_iDay);
			case RZ_EDoorSymbol.RING_DOT: return string.Format("A chalk ring with a dot in it. D%1.", m.m_iDay);
			case RZ_EDoorSymbol.ARROW:
			{
				if (m.Has(RZ_DoorMark.F_SHOWN) && !m.m_sText.IsEmpty())
					return string.Format("Chalk by the door: %1.", m.m_sText);

				return string.Format("A chalk arrow and a letter: → %1.", ArrowLetter(m.m_iA));
			}
			case RZ_EDoorSymbol.DROP: return string.Format("A drop in chalk on the post by the pump. D%1.", m.m_iDay);
			case RZ_EDoorSymbol.HAND:
			{
				if (m.Has(RZ_DoorMark.F_STRUCK))
					return string.Format("A red hand with a stroke through it. D%1.", m.m_iDay);

				return string.Format("A red hand, flat, and %1 strokes beside it.", m.m_iA);
			}
			case RZ_EDoorSymbol.TALLY:
			{
				if (m.Has(RZ_DoorMark.F_PAID))
					return "Red strokes in gates of five, and across them in red: PAID.";

				return string.Format("Red strokes in gates of five: %1 of them.", m.m_iA);
			}
			case RZ_EDoorSymbol.BEAK:
			{
				if (m.Has(RZ_DoorMark.F_BAR))
					return string.Format("A plague doctor stencilled on white primer, with a bar through it. d%1.", m.m_iDay);

				if (m.Has(RZ_DoorMark.F_RING))
					return string.Format("A plague doctor stencilled on the door, with a ring round it. Underneath: PLEASE DO NOT BURN. d%1.", m.m_iDay);

				return string.Format("A plague doctor stencilled on white primer: S-%1 · d%2 · %3.", Series(m.m_iA), m.m_iDay, m.m_iB);
			}
			case RZ_EDoorSymbol.SCRATCH_OUT:
			{
				if (m.Has(RZ_DoorMark.F_YOUNG))
					return "Scratches on the door with the bear. Fresh.";

				return "Gouges in the wood at hand height, on the outside.";
			}
			case RZ_EDoorSymbol.SCRATCH_IN: return "Gouges on the inside of the open door.";
			case RZ_EDoorSymbol.SMEARS: return "Low grey smears along the sill.";
			case RZ_EDoorSymbol.THUMB: return "A small red print, low on the frame.";
			case RZ_EDoorSymbol.WORDS: return Words(m);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! What the writer meant. "" when Read adds nothing to the glance.
	static string DoorRead(RZ_DoorMark m, RZ_MarkedDoor d)
	{
		switch (m.m_iSymbol)
		{
			case RZ_EDoorSymbol.X:
			{
				int living = (m.m_iC / 100) % 10;
				int dead = (m.m_iC / 10) % 10;
				int hazard = m.m_iC / 1000;
				if (d && d.m_bLocked)
					hazard = 3;

				string found = " Nobody found.";
				if (living > 0)
					found = string.Format(" %1 living sent to %2.", CapNum(living), DestWords(m.m_iC % 10));

				if (dead > 0)
					found += string.Format(" %1 dead.", CapNum(dead));

				string ring = "";
				if (m.Has(RZ_DoorMark.F_RING))
					ring = " Checked again on Day 5.";

				return string.Format("Bastion's search code, %1. Day %2, %3, section %4.%5%6%7", XCode(m, d && d.m_bLocked), m.m_iDay, ClockWords(m.m_iA), Unit(m.m_iB), HazardWords(hazard), found, ring);
			}
			case RZ_EDoorSymbol.SLASH: return string.Format("Bastion's code: section %1 went in on Day %2 and never marked it done.", Unit(m.m_iB), m.m_iDay);
			case RZ_EDoorSymbol.POSTCODE: return string.Format("A Bastion post's street: post %1, since Day %2.", m.m_iA, m.m_iDay);
			case RZ_EDoorSymbol.CONTRIB: return string.Format("Bastion took a contribution from here on Day %1.", m.m_iDay);
			case RZ_EDoorSymbol.ATT: return "Bastion's mark for attached civilians: the family here lives under the post's protection.";
			case RZ_EDoorSymbol.CLEARED: return string.Format("Bastion cleared this on Day %1 at %2: culled, shelled or retaken.", m.m_iDay, ClockWords(m.m_iA));
			case RZ_EDoorSymbol.BEAR:
			{
				if (m.Has(RZ_DoorMark.F_TICK) && m.m_iA > 0)
					return string.Format("Everyone got out. The Haven ticked it on Day %1; they went to %2.", m.m_iDay, TownName(m.m_iA - 1));

				if (m.Has(RZ_DoorMark.F_TICK))
					return string.Format("Everyone got out. The Haven ticked it on Day %1.", m.m_iDay);

				if (m.Has(RZ_DoorMark.F_RING) && m.m_iA > 0)
					return string.Format("Children were here. They went to %1.", TownName(m.m_iA - 1));

				if (m.Has(RZ_DoorMark.F_RING))
					return "Children were here. They moved on, safe.";

				return string.Format("The Haven's bear: children inside, as of Day %1.", m.m_iDay);
			}
			case RZ_EDoorSymbol.PATCHED_BEAR:
			{
				if (m.Has(RZ_DoorMark.F_RING) && m.m_iA > 0)
					return string.Format("A Haven house, emptied. The people went to %1.", TownName(m.m_iA - 1));

				if (m.Has(RZ_DoorMark.F_RING))
					return string.Format("A Haven house, emptied on Day %1.", m.m_iDay);

				return string.Format("A Haven house. It opened its door on Day %1.", m.m_iDay);
			}
			case RZ_EDoorSymbol.CANDLE:
			{
				if (m.Has(RZ_DoorMark.F_SHOWN))
					return "";

				return string.Format("The Haven's candle: %1 lost from this house, the last on Day %2.", Num(m.m_iA), m.m_iDay);
			}
			case RZ_EDoorSymbol.HATCH: return string.Format("Haven chalk, Day %1. They never write what the hatching is for.", m.m_iDay);
			case RZ_EDoorSymbol.RING_DOT: return string.Format("Haven chalk: checked on Day %1, nobody here.", m.m_iDay);
			case RZ_EDoorSymbol.ARROW:
			{
				if (m.Has(RZ_DoorMark.F_SHOWN))
					return string.Format("Haven chalk: how many from this house went home through the gates. %1.", m.m_sText);

				return string.Format("Haven chalk: gone to the house at %1.", TownName(m.m_iA - 1));
			}
			case RZ_EDoorSymbol.DROP: return string.Format("Haven chalk: the pump here works. Checked on Day %1.", m.m_iDay);
			case RZ_EDoorSymbol.HAND:
			{
				if (m.Has(RZ_DoorMark.F_STRUCK))
					return string.Format("The crews' hand, struck through on Day %1: this door owes.", m.m_iDay);

				return string.Format("A red hand and %1 strokes: %2 weeks paid. The crews take their cut here, and nobody else does.", m.m_iA, Num(m.m_iA));
			}
			case RZ_EDoorSymbol.TALLY:
			{
				if (m.Has(RZ_DoorMark.F_PAID))
					return string.Format("The line is cleared, in the book's hand, on Day %1.", m.m_iDay);

				return string.Format("The crews' tally: %1 tins owed.", Num(m.m_iA));
			}
			case RZ_EDoorSymbol.BEAK:
			{
				if (m.Has(RZ_DoorMark.F_BAR))
					return string.Format("Halcyon finished here on Day %1.", m.m_iDay);

				if (m.Has(RZ_DoorMark.F_RING))
					return string.Format("Halcyon asks that the slow ones here be left for study. Day %1.", m.m_iDay);

				return string.Format("S-%1, day %2, %3 samples. Halcyon's stencil.", Series(m.m_iA), m.m_iDay, Num(m.m_iB));
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The one more true detail. "" when there's none for this reader.
	static string DoorKnow(RZ_DoorMark m, RZ_TraceReader r, RZ_MarkedDoor d, float ageDays)
	{
		switch (m.m_iSymbol)
		{
			case RZ_EDoorSymbol.X:
			{
				if (m.Has(RZ_DoorMark.F_RING))
					return "The ring means someone came back two days later to look again. You don't do that unless you're looking for someone.";

				if (m.m_iDay == 2 && m.m_iA < 1400)
					return "Before the column left. Whoever was found walked to the north road and turned back that afternoon.";

				if (m.m_iDay == 1)
					return "The cordon detail's sweep, the first night.";

				if (m.m_iDay >= 4)
					return "One of the last sweeps. After Day 5 they stopped looking for anyone but their own.";

				return "The stragglers' sweep, after the north road shut.";
			}
			case RZ_EDoorSymbol.SLASH: return "The second stroke goes on as the section comes out. Nobody put it on.";
			case RZ_EDoorSymbol.CONTRIB: return "C is what the posts call requisition when they're being polite.";
			case RZ_EDoorSymbol.ATT: return "Attached means counted, fed a little, and expected to be useful.";
			case RZ_EDoorSymbol.CLEARED: return "CLEARED goes on afterwards. It never says who was inside.";
			case RZ_EDoorSymbol.CANDLE:
			{
				if (r.Is("elder") && !m.m_sText.IsEmpty() && !m.Has(RZ_DoorMark.F_SHOWN))
					return string.Format("Under it, small: %1.", m.m_sText);

				return "";
			}
			case RZ_EDoorSymbol.HATCH:
			{
				if (r.Is("nurse"))
					return "Somebody here said it out loud at supper, and was walked out at dusk.";

				return "";
			}
			case RZ_EDoorSymbol.HAND:
			{
				if (m.Has(RZ_DoorMark.F_STRUCK))
					return "Nobody has written what happens next. Nobody needs to.";

				return "";
			}
			case RZ_EDoorSymbol.TALLY:
			{
				if (m.Has(RZ_DoorMark.F_PAID) && d && !RZ_WorldFacts.Get().IsLivedIn(d.m_iTown))
					return "Paid, in the book's hand. Nobody lives here now.";

				if (m.Has(RZ_DoorMark.F_PAID))
					return "Nobody asks who paid it.";

				return "";
			}
			case RZ_EDoorSymbol.BEAK: return string.Format("S-%1 is the lab's own series, not a field team's.", Series(m.m_iA));
			case RZ_EDoorSymbol.SCRATCH_OUT:
			{
				// The young come home (L4): never who
				if (m.Has(RZ_DoorMark.F_YOUNG))
					return "One of them. Not tall. It stood here a long time.";

				string many = "One of them";
				if (m.m_iA > 1)
					many = CapNum(m.m_iA) + " of them";

				string tall = "";
				if (m.m_iB > 0)
					tall = ", one tall";

				return string.Format("Outside, at hand height. %1%2. %3", many, tall, ScratchAge(ageDays));
			}
			case RZ_EDoorSymbol.SCRATCH_IN: return "Someone turned in that room. " + ScratchAge(ageDays);
			case RZ_EDoorSymbol.THUMB:
			{
				if (r.Is("excon"))
					return "A crew's thumb. Someone the Haven put out sleeps here, and the crews know where.";

				return "";
			}
			case RZ_EDoorSymbol.SMEARS: return "The slow ones lay against it for a long time. " + ScratchAge(ageDays);
			case RZ_EDoorSymbol.WORDS: return WordsFit(m, d);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Series(int n)
	{
		string s = n.ToString();
		while (s.Length() < 3)
		{
			s = "0" + s;
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The Hunter's read of the infected's marks: they don't fade, they grey.
	protected static string ScratchAge(float days)
	{
		if (days < 1)
			return "Fresh: pale wood, today.";

		if (days < 2)
			return "A day old.";

		if (days < 6)
			return string.Format("%1 days old.", CapNum(RZ_Util.RoundToInt(Math.Floor(days))));

		if (days < 10)
			return "Grey already. A week old, about.";

		return "Grey right through. Weeks.";
	}

	//------------------------------------------------------------------------------------------------
	//! What weathering adds at a glance.
	protected static string AgeWords(RZ_DoorMark m, int stage)
	{
		if (stage <= 0 || m.m_iAuthor == RZ_EDoorAuthor.INFECTED)
			return "";

		if (m.IsChalk())
		{
			if (stage == 1)
				return " Rained on.";

			return " Washed almost to nothing.";
		}

		if (stage == 2)
			return " Faded.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The Hunter: how old any written mark is.
	static string WrittenAge(RZ_DoorMark m, float days)
	{
		if (m.m_iAuthor == RZ_EDoorAuthor.INFECTED || m.m_iAuthor == RZ_EDoorAuthor.RESIDENTS)
			return "";

		string what = "paint";
		if (m.IsChalk())
			what = "chalk";

		if (days < 1)
			return string.Format("The %1 is from today.", what);

		if (days < 2)
			return string.Format("The %1's a day old.", what);

		return string.Format("The %1's %2 days old.", what, Num(RZ_Util.RoundToInt(Math.Floor(days))));
	}

	// ================================================================ Residents' words
	//------------------------------------------------------------------------------------------------
	//! Residents' words: line a of the pool (b: the town in it, + 1).
	static string Words(RZ_DoorMark m)
	{
		switch (m.m_iA)
		{
			case 0: return string.Format("In lipstick on the glass: \"GONE TO MUM'S IN %1. D1\"", TownName(m.m_iB - 1));
			case 1: return "In marker on the door: \"WE'RE AT THE GYM. D2\"";
			case 2: return "In pencil, pressed hard: \"DON'T COME IN. TELL ANNA I'M SORRY. D2\"";
			case 3: return "In chalk, small: \"BACK SOON. KEY WHERE IT WAS\"";
			case 4: return "In marker across the door: \"THE RADIO SAYS STAY INDOORS. WE ARE INDOORS.\"";
			case 5: return "On card, pinned up: \"CHURCH OPEN. ASK FOR THE FATHER. D4\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The Police Officer: whether the day fits the rest of the door.
	protected static string WordsFit(RZ_DoorMark m, RZ_MarkedDoor d)
	{
		if (!d)
			return "";

		RZ_DoorMark x = d.Find(RZ_EDoorSymbol.X);
		if (!x)
			return "";

		int wrote = m.m_iDay;
		if (m.m_iA == 3 || m.m_iA == 4)
			wrote = 1;

		if (x.m_iDay > wrote)
			return string.Format("Written before the search came through on Day %1.", x.m_iDay);

		if (x.m_iDay < wrote)
			return "Written after the search. Somebody came back.";

		return "Written the same day the search came through.";
	}

	// ================================================================ The town
	//------------------------------------------------------------------------------------------------
	//! "Doors in Levie: three bears, two of them ringed. A red hand on the shop." At a glance, the
	//! three biggest things only.
	static string TownSummary(string town, array<RZ_MarkedDoor> doors)
	{
		int xs = 0;
		int bears = 0;
		int ringed = 0;
		int candles = 0;
		int hands = 0;
		bool handShop = false;
		int beaks = 0;
		int scratches = 0;
		int words = 0;
		int stencils = 0;
		int chalk = 0;
		foreach (RZ_MarkedDoor d : doors)
		{
			RZ_DoorMark top = d.Top();
			if (!top)
				continue;

			switch (top.m_iSymbol)
			{
				case RZ_EDoorSymbol.X: xs++; break;
				case RZ_EDoorSymbol.SLASH: xs++; break;
				case RZ_EDoorSymbol.BEAR:
				{
					bears++;
					if (top.Has(RZ_DoorMark.F_RING) || top.Has(RZ_DoorMark.F_TICK))
						ringed++;
					break;
				}
				case RZ_EDoorSymbol.PATCHED_BEAR: bears++; break;
				case RZ_EDoorSymbol.CANDLE: candles++; break;
				case RZ_EDoorSymbol.HAND:
				{
					hands++;
					if (d.m_iKind == RZ_EDoorWant.SHOP || d.m_iWant == RZ_EDoorWant.SHOP)
						handShop = true;
					break;
				}
				case RZ_EDoorSymbol.TALLY: hands++; break;
				case RZ_EDoorSymbol.BEAK: beaks++; break;
				case RZ_EDoorSymbol.SCRATCH_OUT: scratches++; break;
				case RZ_EDoorSymbol.SCRATCH_IN: scratches++; break;
				case RZ_EDoorSymbol.SMEARS: scratches++; break;
				case RZ_EDoorSymbol.WORDS: words++; break;
				case RZ_EDoorSymbol.HATCH: chalk++; break;
				case RZ_EDoorSymbol.RING_DOT: chalk++; break;
				case RZ_EDoorSymbol.ARROW: chalk++; break;
				case RZ_EDoorSymbol.DROP: chalk++; break;
				case RZ_EDoorSymbol.THUMB: break;	// too small to see from the street
				default: stencils++; break;
			}
		}

		array<string> parts = {};
		if (bears > 0)
		{
			string b = Count(bears, "a bear", "bears");
			if (ringed > 0 && ringed == bears && bears > 1)
				b += ", all of them ringed";
			else if (ringed > 0 && bears > 1)
				b += string.Format(", %1 of them ringed", Num(ringed));
			else if (ringed > 0)
				b += " with a ring round it";

			parts.Insert(b);
		}

		if (candles > 0)
			parts.Insert(Count(candles, "a candle in chalk", "candles in chalk"));

		if (hands > 0)
		{
			string h = Count(hands, "a red hand", "red hands");
			if (handShop)
				h += ", one on the shop";

			parts.Insert(h);
		}

		if (xs > 0)
			parts.Insert(Count(xs, "an orange X", "orange Xs"));

		if (beaks > 0)
			parts.Insert(Count(beaks, "a plague doctor stencil", "plague doctor stencils"));

		if (words > 0)
			parts.Insert(Count(words, "writing on a door", "doors with writing on them"));

		if (scratches > 0)
			parts.Insert(Count(scratches, "a door gouged", "doors gouged"));

		if (stencils > 0)
			parts.Insert(Count(stencils, "a black stencil", "black stencils"));

		if (chalk > 0)
			parts.Insert(Count(chalk, "some chalk", "chalk on more doors"));

		if (parts.IsEmpty())
			return "";

		string s = "";
		for (int i = 0; i < parts.Count() && i < 3; i++)
		{
			if (i == 0)
				s = RZ_Util.Capitalize(parts[i]);
			else
				s += ". " + RZ_Util.Capitalize(parts[i]);
		}

		return string.Format("Doors in %1: %2.", town, s);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Count(int n, string one, string many)
	{
		if (n == 1)
			return one;

		return Num(n) + " " + many;
	}

	// ================================================================ The log
	//------------------------------------------------------------------------------------------------
	static string SymbolKey(int symbol)
	{
		array<string> keys = {"x", "slash", "postcode", "c", "att", "cleared", "bear", "patched_bear", "candle", "hatch",
			"ring_dot", "arrow", "drop", "hand", "tally", "beak", "scratch_out", "scratch_in", "smears", "thumb", "words"};
		if (symbol < 0 || symbol >= keys.Count())
			return "?";

		return keys[symbol];
	}

	//------------------------------------------------------------------------------------------------
	//! A symbol from its log key (the admin command), -1 if none.
	static int SymbolFromKey(string key)
	{
		for (int i = 0; i <= RZ_EDoorSymbol.WORDS; i++)
		{
			if (SymbolKey(i) == key)
				return i;
		}

		return -1;
	}

	// ================================================================ Player traces
	//------------------------------------------------------------------------------------------------
	//! A grave (P3): a name only where the naming rules allow it, else the description.
	static string Grave(string who, bool named, int day)
	{
		if (named)
			return string.Format("Two fence palings and a mound, and a name in biro: %1. Day %2.", who, day);

		return string.Format("Two fence palings and a mound. In biro: %1. Day %2.", who, day);
	}

	//------------------------------------------------------------------------------------------------
	//! Ash where the bodies were burned (P2).
	static string Pyre(int count, int day)
	{
		return string.Format("A black patch in the grass, and a count on the post beside it: %1. D%2.", count, day);
	}

	//------------------------------------------------------------------------------------------------
	//! The memorial candles at every safehouse (P3b).
	static string Candles()
	{
		return "Candles in jars on the step. The house read a name at supper.";
	}

	//------------------------------------------------------------------------------------------------
	//! The town log's season mark, with its look by kind (P5).
	static string SeasonMark(string kind, string line)
	{
		switch (kind)
		{
			case "rescue": return "A sheet on two posts by the road, in marker: " + line;
			case "burned": return "A board by the road, the words burned into it with a hot nail: " + line;
			case "taken": return "A stencilled board by the road: " + line;
			case "emptied": return "Flowers tied to a post by the road, and a card: " + line;
			case "sealed": return "Tape between two posts by the road, and a card on it: " + line;
		}

		return "A board by the road, painted by hand: " + line;
	}
}
