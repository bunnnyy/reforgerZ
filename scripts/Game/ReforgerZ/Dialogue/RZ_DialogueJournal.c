//! The notebook's Journal rows from talk (docs/UI.md, "The notebook"; docs/DIALOGUE.md, "Promises
//! and callbacks"): one **Promises** row while you have a promise open (each with its play left), and
//! one **What you know** row once a lie you were told has turned out false and you know it ("The
//! Scavenger said the chemist's was clean. It wasn't."). Journal records like the others (RZ_UIFeed's
//! M and D records), so the notebook needs nothing new. Server only.
class RZ_DialogueJournal
{
	static const int PROMISES_ID = 400001;
	static const int KNOWN_ID = 400002;
	protected static const int KNOWN_ROWS = 12;
	protected static const int DETAIL_MAX = 1500;

	//------------------------------------------------------------------------------------------------
	static bool IsOurs(int id)
	{
		return id == -PROMISES_ID || id == -KNOWN_ID;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_DialogueMemory Memory()
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!trees || !trees.IsOn())
			return null;

		return trees.GetMemory();
	}

	//------------------------------------------------------------------------------------------------
	//! The journal's M records (RZ_UIFeed.BuildJournal).
	static string Rows(int playerId)
	{
		RZ_DialogueMemory mem = Memory();
		if (!mem)
			return "";

		string s = "";
		array<string> what = {};
		array<string> left = {};
		mem.OpenPromises(playerId, what, left);
		if (!what.IsEmpty())
		{
			string brief = string.Format("You promised %1. %2 of play left.", what[0], left[0]);
			RZ_UILine ln = new RZ_UILine("M");
			ln.AddInt(-PROMISES_ID).AddInt(RZ_EMissionKindUI.ERRAND).AddInt(-1).Add("Promises").Add(string.Format("%1 open", what.Count())).Add("").Add("");
			ln.AddInt(-1).Add("").AddInt(0).AddInt(0).Add(brief).Add("1");
			s += ln.End();
		}

		array<string> lines = {};
		Known(playerId, lines);
		if (!lines.IsEmpty())
		{
			RZ_UILine kn = new RZ_UILine("M");
			kn.AddInt(-KNOWN_ID).AddInt(RZ_EMissionKindUI.DOORS).AddInt(-1).Add("What you know").Add(string.Format("%1 found out", lines.Count())).Add("").Add("");
			kn.AddInt(-1).Add("").AddInt(0).AddInt(0).Add(lines[0]).Add("0");
			s += kn.End();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! A row's detail: every open promise with its play left, or every lie you've seen through.
	static string Detail(int playerId, int id)
	{
		RZ_DialogueMemory mem = Memory();
		if (!mem)
			return "";

		string told = "";
		if (id == -PROMISES_ID)
		{
			array<string> what = {};
			array<string> left = {};
			mem.OpenPromises(playerId, what, left);
			foreach (int i, string w : what)
			{
				told += string.Format("You promised %1. %2 of play left.\n\n", w, left[i]);
			}

			if (!what.IsEmpty())
				told += "A promise pays nothing of its own. Keep it, and they remember; break it, and they remember that.";
		}
		else
		{
			array<string> lines = {};
			Known(playerId, lines);
			foreach (string line : lines)
			{
				told += line + "\n\n";
			}
		}

		RZ_UILine ln = new RZ_UILine("D");
		string payload = ln.AddInt(id).Add(told.Trim()).Add("").Add("").End();
		if (payload.Length() > DETAIL_MAX)
			payload = payload.Substring(0, DETAIL_MAX);

		return payload;
	}

	//------------------------------------------------------------------------------------------------
	//! "<the lie, as it was said>" It wasn't true. Newest first.
	protected static void Known(int playerId, notnull array<string> lines)
	{
		RZ_DialogueMemory mem = Memory();
		array<string> keys = {};
		mem.FoundLies(playerId, keys, KNOWN_ROWS);
		foreach (string key : keys)
		{
			string said = LieLine(key);
			if (said.IsEmpty())
				continue;

			lines.Insert(string.Format("They told you: %1 It wasn't true.", said));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The line that told a lie (its key, "@here" bound to a speaker or not), tokens left out.
	protected static string LieLine(string key)
	{
		string bare = key;
		int at = bare.IndexOf("@");
		if (at > 0)
			bare = bare.Substring(0, at) + "@here";

		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		string found = trees.GetDB().LieText(bare);
		if (found.IsEmpty())
			found = trees.GetDB().LieText(key);

		if (RZ_WorldFacts.Get())
			found = RZ_WorldFacts.Get().Fill(found);

		return found;
	}
}
