//! Bastion's word at the wire, said in a conversation (docs/DIALOGUE.md, "Claims in someone else's
//! kit"): three words are offered, shuffled daily, and one is today's (RZ_Disguise.WordToday, changed
//! at the 05:30 shift and read on Bastion's secure net). The right one is a cover detail at the next
//! look (+1); a wrong one fails at once. A friend can tell you the word over voice. Server only.
class RZ_DialogueWire
{
	//------------------------------------------------------------------------------------------------
	//! Today's three offered words, the right one among them, the same for everyone all day.
	static void Offered(RZ_GameModeComponent gm, notnull array<string> result)
	{
		result.Clear();
		RZ_Hostility hostility = gm.GetHostility();
		if (!hostility || !RZ_WorldFacts.Get())
			return;

		string today = hostility.GetDisguise().WordToday();
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + RZ_SeasonClock.Day() * 977 + 31);
		// The words a decoy is drawn from: the same list as RZ_Disguise.WordToday's (a decoy is never
		// today's word, whichever list that is)
		array<string> all = {"KETTLE", "LANTERN", "HARROW", "PIKESTAFF", "MILLRACE", "FLINT", "ORCHARD", "CANDLE", "BRIDLE", "SPARROW", "TALLOW", "HOLLY"};
		array<string> decoys = {};
		foreach (string w : all)
		{
			if (w != today)
				decoys.Insert(w);
		}

		while (result.Count() < 2 && !decoys.IsEmpty())
		{
			int i = rng.Int(0, decoys.Count());
			result.Insert(decoys[i]);
			decoys.Remove(i);
		}

		result.InsertAt(today, rng.Int(0, result.Count() + 1));
	}

	//------------------------------------------------------------------------------------------------
	//! {wire:1} to {wire:3}: the offered words, in Bastion's capitals.
	static string Fill(RZ_GameModeComponent gm, string text)
	{
		array<string> words = {};
		Offered(gm, words);
		string s = text;
		for (int i = 0; i < 3; i++)
		{
			string word = "";
			if (i < words.Count())
				word = words[i];

			s.Replace("{wire:" + (i + 1).ToString() + "}", word);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! wire:N, the player says the Nth word: the right one counts at the next look today; a wrong
	//! one is caught at once. True if it was right.
	static bool Say(RZ_GameModeComponent gm, int playerId, int n, IEntity user)
	{
		array<string> words = {};
		Offered(gm, words);
		RZ_Hostility hostility = gm.GetHostility();
		if (!hostility || n < 1 || n > words.Count())
			return false;

		RZ_Disguise d = hostility.GetDisguise();
		if (words[n - 1] == d.WordToday())
		{
			d.LearnWord(playerId);
			return true;
		}

		if (user && d.InTheirKit(playerId, RZ_EFaction.MILITARY))
			d.Caught(playerId, RZ_EFaction.MILITARY, user, "Wrong word. Hands on the barrier.");

		return false;
	}
}
