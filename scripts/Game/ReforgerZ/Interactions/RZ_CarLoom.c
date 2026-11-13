//! The loom under a fixable car's steering column: the repair screen's hotwire, cut by hand
//! (docs/DESIGN.md, "Cars: found, rare and hard to fix"; the screen: UI/RZ_CarMenu.c).
//!
//! Five wires, left to right. One is the battery feed (the thick one: never cut), two start the
//! car (the ignition wire and the starter wire), one is the horn and one the dash lamps. Each has a
//! colour, and two of the four thin ones carry a stripe. Two lines on the sticker under the column
//! are enough to work out which two to cut: the loom is rolled until a pair of true lines settles
//! it (Settles), so it's a puzzle to think through, never one to be quick at.
//!
//! Data only, kept on the car (RZ_CarProject.m_Loom) and decided on the server: RZ_Cars rolls it
//! when someone first picks Hotwire on the screen, checks each cut and sends what can be seen
//! (Records). Not saved: after a restart a car has a fresh loom.

//------------------------------------------------------------------------------------------------
class RZ_CarLoom
{
	static const int WIRES = 5;

	static const int ROLE_FEED = 0;
	static const int ROLE_IGNITION = 1;
	static const int ROLE_STARTER = 2;
	static const int ROLE_HORN = 3;
	static const int ROLE_LAMPS = 4;

	static const int CUT_NONE = 0;
	static const int CUT_GOOD = 1;		// one of the two: stripped and bared
	static const int CUT_BAD = 2;

	//! What a cut did (Cut).
	static const int RESULT_NOTHING = 0;	// no such wire, or it's cut already
	static const int RESULT_GOOD = 1;		// one of the two
	static const int RESULT_DONE = 2;		// the second of the two: the car is hotwired
	static const int RESULT_LAMPS = 3;		// a wrong wire: a spark
	static const int RESULT_HORN = 4;		// a wrong wire: the horn sounds
	static const int RESULT_FEED = 5;		// the feed: the loom shorts at once

	protected static const int CLUES = 7;	// the kinds of line the sticker can carry
	protected static const int TRIES = 40;	// looms rolled before one is said straight

	ref array<int> m_aRole = {};		// by place, left to right
	ref array<int> m_aColour = {};		// by place: an index for ColourWord
	ref array<int> m_aStriped = {};		// by place: 1 = it carries a stripe
	ref array<int> m_aCut = {};			// by place: CUT_*
	ref array<string> m_aClues = {};	// the sticker's lines
	int m_iStrikes;						// wrong cuts so far

	//------------------------------------------------------------------------------------------------
	//! A fresh loom whose two sticker lines settle which wires to cut.
	static RZ_CarLoom Make()
	{
		RZ_CarLoom loom;
		for (int attempt = 0; attempt < TRIES; attempt++)
		{
			loom = new RZ_CarLoom();
			loom.Shuffle();
			if (loom.FindClues())
				return loom;
		}

		// No pair of lines settles this one (it shouldn't come to this): the sticker names them
		loom.m_aClues.Clear();
		loom.m_aClues.Insert(string.Format("The ignition wire is the %1 one.", ColourWord(loom.m_aColour[loom.Find(ROLE_IGNITION)])));
		loom.m_aClues.Insert(string.Format("The starter wire is the %1 one.", ColourWord(loom.m_aColour[loom.Find(ROLE_STARTER)])));
		return loom;
	}

	//------------------------------------------------------------------------------------------------
	static string ColourWord(int colour)
	{
		switch (colour)
		{
			case 0: return "red";
			case 1: return "yellow";
			case 2: return "green";
			case 3: return "blue";
		}

		return "white";
	}

	//------------------------------------------------------------------------------------------------
	//! The place of the wire with this role.
	int Find(int role)
	{
		for (int i = 0; i < m_aRole.Count(); i++)
		{
			if (m_aRole[i] == role)
				return i;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	int GoodCuts()
	{
		int good = 0;
		foreach (int cut : m_aCut)
		{
			if (cut == CUT_GOOD)
				good++;
		}

		return good;
	}

	//------------------------------------------------------------------------------------------------
	//! Cuts the wire at `index`: what it did (RESULT_*). A wrong cut is a strike.
	int Cut(int index)
	{
		if (index < 0 || index >= m_aCut.Count() || m_aCut[index] != CUT_NONE)
			return RESULT_NOTHING;

		int role = m_aRole[index];
		if (role == ROLE_IGNITION || role == ROLE_STARTER)
		{
			m_aCut[index] = CUT_GOOD;
			if (GoodCuts() >= 2)
				return RESULT_DONE;

			return RESULT_GOOD;
		}

		m_aCut[index] = CUT_BAD;
		m_iStrikes++;
		if (role == ROLE_FEED)
			return RESULT_FEED;

		if (role == ROLE_HORN)
			return RESULT_HORN;

		return RESULT_LAMPS;
	}

	//------------------------------------------------------------------------------------------------
	//! What a wire is known to be: the feed always (it's the thick one), a cut wire once it's cut,
	//! and for a Mechanic the horn. "" while nobody could know.
	string Label(int index, bool mechanic)
	{
		if (index < 0 || index >= m_aRole.Count())
			return "";

		int role = m_aRole[index];
		if (role == ROLE_FEED)
			return "FEED";

		if (m_aCut[index] == CUT_NONE && !(mechanic && role == ROLE_HORN))
			return "";

		switch (role)
		{
			case ROLE_IGNITION: return "IGNITION";
			case ROLE_STARTER: return "STARTER";
			case ROLE_HORN: return "HORN";
		}

		return "LAMPS";
	}

	//------------------------------------------------------------------------------------------------
	//! The loom as the repair screen's records: L|wrong cuts|allowed|right cuts, then
	//! K|place|colour|thick|striped|cut|label per wire, then Q|line per sticker line. A Mechanic
	//! knows the horn wire: it's labelled, and a line says so.
	string Records(int allowed, bool mechanic)
	{
		string text = string.Format("\nL|%1|%2|%3", m_iStrikes, allowed, GoodCuts());
		for (int i = 0; i < m_aRole.Count(); i++)
		{
			int thick = 0;
			if (m_aRole[i] == ROLE_FEED)
				thick = 1;

			text += string.Format("\nK|%1|%2|%3|%4|%5|%6", i, m_aColour[i], thick, m_aStriped[i], m_aCut[i], Label(i, mechanic));
		}

		text += "\nQ|The thick one is the battery feed. Never cut it.";
		foreach (string clue : m_aClues)
		{
			text += "\nQ|" + clue;
		}

		int horn = Find(ROLE_HORN);
		if (mechanic && m_aCut[horn] == CUT_NONE)
			text += string.Format("\nQ|You know this make: the %1 one is the horn.", ColourWord(m_aColour[horn]));

		return text;
	}

	// ================================================================ Rolling one
	//------------------------------------------------------------------------------------------------
	protected void Shuffle()
	{
		m_aRole.Clear();
		m_aColour.Clear();
		m_aStriped.Clear();
		m_aCut.Clear();
		m_aClues.Clear();
		m_iStrikes = 0;
		for (int i = 0; i < WIRES; i++)
		{
			m_aRole.Insert(i);
			m_aColour.Insert(i);
			m_aStriped.Insert(0);
			m_aCut.Insert(CUT_NONE);
		}

		ShuffleInts(m_aRole);
		ShuffleInts(m_aColour);

		// Two of the four thin wires carry a stripe; the feed is plain
		array<int> thin = {};
		for (int p = 0; p < WIRES; p++)
		{
			if (m_aRole[p] != ROLE_FEED)
				thin.Insert(p);
		}

		ShuffleInts(thin);
		m_aStriped[thin[0]] = 1;
		m_aStriped[thin[1]] = 1;
	}

	//------------------------------------------------------------------------------------------------
	protected static void ShuffleInts(notnull array<int> list)
	{
		for (int i = list.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			int a = list[i];
			list[i] = list[j];
			list[j] = a;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Picks two true lines that together leave only the real two wires, neither enough alone.
	//! The two stripe lines together ("cut the striped ones") are the easy loom: used only when no
	//! other pair will do. False when no pair settles this loom.
	protected bool FindClues()
	{
		array<int> firsts = {};
		array<int> seconds = {};
		for (int a = 0; a < CLUES; a++)
		{
			if (Settles(a, -1))
				continue;

			for (int b = a + 1; b < CLUES; b++)
			{
				if (a == 0 && b == 1)
					continue;

				if (Settles(b, -1) || !Settles(a, b))
					continue;

				firsts.Insert(a);
				seconds.Insert(b);
			}
		}

		if (firsts.IsEmpty())
		{
			if (Settles(0, -1) || Settles(1, -1) || !Settles(0, 1))
				return false;

			firsts.Insert(0);
			seconds.Insert(1);
		}

		int pick = Math.RandomInt(0, firsts.Count());
		int ign = Find(ROLE_IGNITION);
		int starter = Find(ROLE_STARTER);
		m_aClues.Insert(ClueText(firsts[pick], Holds(firsts[pick], ign, starter)));
		m_aClues.Insert(ClueText(seconds[pick], Holds(seconds[pick], ign, starter)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Whether lines a and b (b -1: a alone), read as they are true of this loom, leave only the
	//! real pair of wires to cut. Every way of placing the ignition and starter wires is tried.
	protected bool Settles(int a, int b)
	{
		int ign = Find(ROLE_IGNITION);
		int starter = Find(ROLE_STARTER);
		int feed = Find(ROLE_FEED);
		bool wantA = Holds(a, ign, starter);
		bool wantB = true;
		if (b >= 0)
			wantB = Holds(b, ign, starter);

		int real = PairKey(ign, starter);
		for (int i = 0; i < WIRES; i++)
		{
			for (int s = 0; s < WIRES; s++)
			{
				if (i == s || i == feed || s == feed)
					continue;

				if (Holds(a, i, s) != wantA)
					continue;

				if (b >= 0 && Holds(b, i, s) != wantB)
					continue;

				if (PairKey(i, s) != real)
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The two places as one number, whichever way round.
	protected static int PairKey(int a, int b)
	{
		if (a < b)
			return a * 10 + b;

		return b * 10 + a;
	}

	//------------------------------------------------------------------------------------------------
	//! Whether line `kind` is true with the ignition wire at `ign` and the starter wire at `starter`.
	protected bool Holds(int kind, int ign, int starter)
	{
		int feed = Find(ROLE_FEED);
		switch (kind)
		{
			case 0: return m_aStriped[ign] == 1;
			case 1: return m_aStriped[starter] == 1;
			case 2: return Math.AbsInt(starter - feed) == 1;
			case 3: return Math.AbsInt(ign - feed) == 1;
			case 4: return ign < starter;
			case 5: return Math.AbsInt(ign - starter) == 1;
			case 6: return starter == 0 || starter == WIRES - 1;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Line `kind` as the sticker puts it, when it's true (yes) or when its opposite is.
	protected static string ClueText(int kind, bool yes)
	{
		switch (kind)
		{
			case 0:
			{
				if (yes)
					return "The ignition wire is one of the two with a stripe.";

				return "The ignition wire is a plain one.";
			}
			case 1:
			{
				if (yes)
					return "The starter wire has a stripe.";

				return "The starter wire is a plain one.";
			}
			case 2:
			{
				if (yes)
					return "The starter wire runs right beside the feed.";

				return "The starter wire never sits next to the feed.";
			}
			case 3:
			{
				if (yes)
					return "The ignition wire runs right beside the feed.";

				return "The ignition wire keeps clear of the feed.";
			}
			case 4:
			{
				if (yes)
					return "The ignition wire is left of the starter wire.";

				return "The ignition wire is right of the starter wire.";
			}
			case 5:
			{
				if (yes)
					return "The two you want sit side by side.";

				return "The two you want never touch.";
			}
		}

		if (yes)
			return "The starter wire is an outside one.";

		return "The starter wire isn't an outside one.";
	}
}
