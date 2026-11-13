//! Writing a duty's beats a line at a time (docs/ERRANDS.md, "Things that happen"): T() starts a
//! beat, O() adds a thing you can do at it, with its outcome as a few short codes. G() starts an
//! interaction: a group of beats whose flags are its own, so many can be written into a pool and a
//! few drawn for one run (Draw). Used by the duties' content files. Server only.
class RZ_DutyScript
{
	protected static ref array<ref array<ref RZ_DutyBeat>> s_aGroups;
	protected static ref array<ref RZ_DutyBeat> s_aInto;
	protected static ref RZ_DutyBeat s_Beat;
	protected static string s_sGroup;
	protected static int s_iGroups;

	//------------------------------------------------------------------------------------------------
	//! Beats go straight into this list, in order.
	static void Into(notnull array<ref RZ_DutyBeat> beats)
	{
		s_aInto = beats;
		s_aGroups = null;
		s_sGroup = "";
		s_Beat = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Beats go into a pool of interactions (G starts each), to draw from.
	static void Pool()
	{
		s_aGroups = new array<ref array<ref RZ_DutyBeat>>();
		s_aInto = null;
		s_sGroup = "";
		s_Beat = null;
	}

	//------------------------------------------------------------------------------------------------
	//! A new interaction in the pool, if `when` holds (false: what's written until the next G is
	//! dropped).
	static void G(string key, bool when = true)
	{
		s_iGroups++;
		s_sGroup = key + s_iGroups.ToString() + ".";
		s_aInto = null;
		if (!when || !s_aGroups)
			return;

		s_aInto = new array<ref RZ_DutyBeat>();
		s_aGroups.Insert(s_aInto);
	}

	//------------------------------------------------------------------------------------------------
	//! A beat: its key, the line said as you reach it, where it is (RZ_DutyBeat.AT_*), and the flag
	//! of its own interaction it needs ("": always).
	static RZ_DutyBeat T(string key, string line, int where = 5, string needs = "")
	{
		s_Beat = new RZ_DutyBeat(key, line, where);
		s_Beat.m_sGroup = s_sGroup;
		s_Beat.m_sIf = needs;
		if (s_aInto)
			s_aInto.Insert(s_Beat);

		return s_Beat;
	}

	//------------------------------------------------------------------------------------------------
	//! Something you can do at the beat: its prompt, what follows, and its outcome in codes
	//! separated by spaces: +N or -N Favor; fame1; shady1 or shady2 (Infamy if seen); infamy2 (at
	//! once); right, wrong (a scored beat); food1, fuel1, meat1, med1 (taken from your pack);
	//! flag:x (sets it for this interaction); next:key; ends (a beat with several picks moves on);
	//! out:8 (its own spot, 8 m from the beat); hold:6; dig, write, fix, fill, burn, care, search
	//! (the work it is); and any effect of RZ_DutyScenes.Apply (report, regard+, regard-, feed, feed2,
	//! morale+, morale-, infect+, infect-, local, later, rumour, record, heat, hook, meat, give,
	//! watch). `who` and `later` are for local, later and rumour; `told` is what the giver says when
	//! you report.
	static RZ_DutyOption O(string label, string said, string codes = "", string who = "", string later = "", string told = "")
	{
		if (!s_Beat)
			return null;

		RZ_DutyOption o = s_Beat.Opt(label, said);
		o.m_sWho = who;
		o.m_sLater = later;
		o.m_sTold = told;
		array<string> parts = {};
		codes.Split(" ", parts, true);
		string fx = "";
		foreach (string c : parts)
		{
			if (c.StartsWith("+") || c.StartsWith("-"))
				o.m_iFavor = c.ToInt();
			else if (c.StartsWith("fame"))
				o.m_iFame = c.Substring(4, c.Length() - 4).ToInt();
			else if (c.StartsWith("shady"))
				o.Shady(c.Substring(5, c.Length() - 5).ToInt());
			else if (c.StartsWith("infamy"))
				o.m_iInfamy = c.Substring(6, c.Length() - 6).ToInt();
			else if (c == "right")
				o.m_iRight = 1;
			else if (c == "wrong")
				o.m_iRight = -1;
			else if (c.StartsWith("food"))
				o.Needs(RZ_EErrandItem.FOOD, c.Substring(4, c.Length() - 4).ToInt());
			else if (c.StartsWith("fuel"))
				o.Needs(RZ_EErrandItem.FUEL, c.Substring(4, c.Length() - 4).ToInt());
			else if (c.StartsWith("meat") && c.Length() > 4)
				o.Needs(RZ_EErrandItem.MEAT, c.Substring(4, c.Length() - 4).ToInt());
			else if (c.StartsWith("med"))
				o.Needs(RZ_EErrandItem.MEDICAL, c.Substring(3, c.Length() - 3).ToInt());
			else if (c.StartsWith("flag:"))
				o.m_sFlag = c.Substring(5, c.Length() - 5);
			else if (c.StartsWith("next:"))
				o.m_sNext = c.Substring(5, c.Length() - 5);
			else if (c == "ends")
				o.m_bEnds = true;
			else if (c.StartsWith("out:"))
				o.m_fOut = c.Substring(4, c.Length() - 4).ToInt();
			else if (c.StartsWith("hold:"))
				o.m_fHold = c.Substring(5, c.Length() - 5).ToInt();
			else if (c == "dig")
				o.m_eWork = RZ_EWork.DIG;
			else if (c == "write")
				o.m_eWork = RZ_EWork.WRITE;
			else if (c == "fix")
				o.m_eWork = RZ_EWork.REPAIR;
			else if (c == "fill")
				o.m_eWork = RZ_EWork.FILL;
			else if (c == "burn")
				o.m_eWork = RZ_EWork.BURN;
			else if (c == "care")
				o.m_eWork = RZ_EWork.CARE;
			else if (c == "search")
				o.m_eWork = RZ_EWork.SEARCH;
			else if (fx.IsEmpty())
				fx = c;
			else
				fx += "," + c;
		}

		o.m_sFx = fx;
		if (o.m_iRight != 0)
			s_Beat.m_bScored = true;

		return o;
	}

	//------------------------------------------------------------------------------------------------
	static array<string> A(string a, string b, string c)
	{
		array<string> list = {a, b, c};
		return list;
	}

	//------------------------------------------------------------------------------------------------
	//! A case to work out and call: its line; three things to look at (`looks`: their prompts,
	//! `shows`: what each shows), of which `allowed` may be done; then three calls (`calls`: their
	//! prompts, `follows`: what follows each, `codes`: each one's outcome codes), `right` the one
	//! that's right (0-2). The right call is scored right, the others wrong. `who` and `later` go
	//! with the right call (a local, a later line), `told` is what the giver says of it.
	static RZ_DutyBeat Case(string key, string line, int where, int allowed, array<string> looks, array<string> shows, array<string> calls, array<string> follows, array<string> codes, int right, string who = "", string later = "", string told = "")
	{
		RZ_DutyBeat b = T(key, line, where);
		b.m_iPicks = allowed;
		for (int i = 0; i < 3; i++)
		{
			O(looks[i], shows[i], "hold:2");
		}

		for (int c = 0; c < 3; c++)
		{
			string code = "ends wrong " + codes[c];
			if (c == right)
				O(calls[c], follows[c], "ends right " + codes[c], who, later, told);
			else
				O(calls[c], follows[c], code);
		}

		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! `count` interactions drawn from the pool, in a random order, appended to `beats`. How many
	//! were in the pool.
	static int Draw(int count, notnull array<ref RZ_DutyBeat> beats)
	{
		if (!s_aGroups)
			return 0;

		int pool = s_aGroups.Count();
		for (int i = 0; i < count && !s_aGroups.IsEmpty(); i++)
		{
			int pick = Math.RandomInt(0, s_aGroups.Count());
			foreach (RZ_DutyBeat b : s_aGroups[pick])
			{
				beats.Insert(b);
			}

			s_aGroups.Remove(pick);
		}

		s_aGroups = null;
		s_aInto = null;
		return pool;
	}
}
