//! The loom under a fixable car's steering column: the repair screen's hotwire, a wire to follow
//! (docs/DESIGN.md, "Hotwiring: the loom"; the screen: UI/RZ_CarMenu.c).
//!
//! Four plugs along the top, four sockets along the bottom, one socket lit: the ignition. Four
//! wires run from the plugs down to the sockets through a tangle, and you cut the one that ends in
//! the lit socket. The tangle is a few turns (Loom turns), each a shuffle of the four columns: in
//! a turn every wire drops, crosses at its own height to the column it leaves by, and drops again.
//! So wires only ever cross at right angles and never run along each other: it can always be
//! followed by eye, and there's nothing to read.
//!
//! Data only, kept on the car (RZ_CarProject.m_Loom) and decided on the server: RZ_Cars rolls it
//! when someone first picks Hotwire on the screen, checks each cut and sends the tangle (Records:
//! the paths are what the player sees, so nothing is hidden but which plug is right). Not saved:
//! after a restart a car has a fresh loom.

//------------------------------------------------------------------------------------------------
class RZ_CarLoom
{
	static const int PLUGS = 4;
	static const int MAX_TURNS = 4;

	//! What a cut did (Cut).
	static const int RESULT_NOTHING = 0;	// no such plug, or it's dead already
	static const int RESULT_DONE = 1;		// the ignition's wire: the car is hotwired
	static const int RESULT_WRONG = 2;		// another: a spark, and that plug is dead

	protected static const int TRIES = 30;	// tangles rolled before any one will do

	ref array<int> m_aNext = {};	// turn by turn, by the column a wire comes in at: the column it leaves by
	ref array<int> m_aLane = {};	// turn by turn, by the column a wire comes in at: the height it crosses at (0 highest)
	ref array<int> m_aDead = {};	// by plug: 1 = cut wrong, dead
	int m_iTurns;
	int m_iGoal;					// the lit socket
	int m_iSpare = -1;				// a wrong plug a Mechanic sees dead from the start
	int m_iStrikes;					// wrong cuts so far

	//------------------------------------------------------------------------------------------------
	//! A fresh loom of `turns` turns (1 to MAX_TURNS). The right wire never runs straight down:
	//! its plug isn't over the lit socket.
	static RZ_CarLoom Make(int turns)
	{
		RZ_CarLoom loom = new RZ_CarLoom();
		loom.m_iTurns = Math.ClampInt(turns, 1, MAX_TURNS);
		for (int attempt = 0; attempt < TRIES; attempt++)
		{
			loom.Tangle();
			if (loom.PickGoal(false))
				return loom;
		}

		loom.PickGoal(true);
		return loom;
	}

	//------------------------------------------------------------------------------------------------
	//! The socket the wire from `plug` ends in.
	int End(int plug)
	{
		int col = plug;
		for (int t = 0; t < m_iTurns; t++)
		{
			col = m_aNext[t * PLUGS + col];
		}

		return col;
	}

	//------------------------------------------------------------------------------------------------
	//! A plug that can't be cut: cut wrong already, or the one a Mechanic knows is wrong.
	bool IsDead(int plug, bool mechanic)
	{
		if (plug < 0 || plug >= m_aDead.Count())
			return true;

		return m_aDead[plug] == 1 || (mechanic && plug == m_iSpare);
	}

	//------------------------------------------------------------------------------------------------
	//! Cuts the wire at `plug`: what it did (RESULT_*). A wrong cut is a strike and kills the plug.
	int Cut(int plug, bool mechanic)
	{
		if (IsDead(plug, mechanic))
			return RESULT_NOTHING;

		if (End(plug) == m_iGoal)
			return RESULT_DONE;

		m_aDead[plug] = 1;
		m_iStrikes++;
		return RESULT_WRONG;
	}

	//------------------------------------------------------------------------------------------------
	//! The loom as the repair screen's records: L|wrong cuts|allowed|lit socket|turns, P|dead flag
	//! per plug, then per turn T|turn|leaves by (per column)|crosses at (per column).
	string Records(int allowed, bool mechanic)
	{
		string text = string.Format("\nL|%1|%2|%3|%4", m_iStrikes, allowed, m_iGoal, m_iTurns);
		text += "\nP";
		for (int p = 0; p < PLUGS; p++)
		{
			int dead = 0;
			if (IsDead(p, mechanic))
				dead = 1;

			text += "|" + dead.ToString();
		}

		for (int t = 0; t < m_iTurns; t++)
		{
			int at = t * PLUGS;
			text += string.Format("\nT|%1|%2|%3|%4|%5|%6|%7|%8|%9", t, m_aNext[at], m_aNext[at + 1], m_aNext[at + 2], m_aNext[at + 3],
				m_aLane[at], m_aLane[at + 1], m_aLane[at + 2], m_aLane[at + 3]);
		}

		return text;
	}

	// ================================================================ Rolling one
	//------------------------------------------------------------------------------------------------
	//! Every turn a shuffle that moves at least three of the four wires and doesn't just undo the
	//! turn above it, with the heights they cross at shuffled too.
	protected void Tangle()
	{
		m_aNext.Clear();
		m_aLane.Clear();
		m_aDead.Clear();
		m_iStrikes = 0;
		for (int p = 0; p < PLUGS; p++)
		{
			m_aDead.Insert(0);
		}

		array<int> last = {};
		for (int t = 0; t < m_iTurns; t++)
		{
			array<int> perm = {};
			for (int roll = 0; roll < 12; roll++)
			{
				Fill(perm);
				ShuffleInts(perm);
				if (Moves(perm) >= 3 && !Undoes(last, perm))
					break;
			}

			array<int> lanes = {};
			Fill(lanes);
			ShuffleInts(lanes);
			foreach (int n : perm)
			{
				m_aNext.Insert(n);
			}

			foreach (int l : lanes)
			{
				m_aLane.Insert(l);
			}

			last.Copy(perm);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Lights the socket of a wire that doesn't end under its own plug (any wire, when `any`), and
	//! picks the Mechanic's dead plug among the wrong ones. False when every wire ends where it began.
	protected bool PickGoal(bool any)
	{
		array<int> plugs = {};
		for (int p = 0; p < PLUGS; p++)
		{
			if (any || End(p) != p)
				plugs.Insert(p);
		}

		if (plugs.IsEmpty())
			return false;

		int right = plugs.GetRandomElement();
		m_iGoal = End(right);
		array<int> wrong = {};
		for (int w = 0; w < PLUGS; w++)
		{
			if (w != right)
				wrong.Insert(w);
		}

		m_iSpare = wrong.GetRandomElement();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Fill(notnull array<int> list)
	{
		list.Clear();
		for (int i = 0; i < PLUGS; i++)
		{
			list.Insert(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! How many wires a turn moves to another column.
	protected static int Moves(notnull array<int> perm)
	{
		int moved = 0;
		foreach (int i, int n : perm)
		{
			if (n != i)
				moved++;
		}

		return moved;
	}

	//------------------------------------------------------------------------------------------------
	//! Whether turn `perm` puts every wire back where turn `last` took it from.
	protected static bool Undoes(notnull array<int> last, notnull array<int> perm)
	{
		if (last.Count() != perm.Count())
			return false;

		foreach (int i, int n : last)
		{
			if (perm[n] != i)
				return false;
		}

		return true;
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
}
