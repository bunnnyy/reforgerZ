//! A small seeded random generator (Park-Miller, Schrage's method, no overflow)
//! so a mission's names, dates and puzzle layout come from one seed.
class RZ_Rng
{
	protected int m_iState = 1;

	protected static const int A = 16807;
	protected static const int M = 2147483647;
	protected static const int Q = 127773;
	protected static const int R = 2836;

	//------------------------------------------------------------------------------------------------
	void RZ_Rng(int seed)
	{
		m_iState = seed % (M - 1);
		if (m_iState <= 0)
			m_iState += M - 1;
	}

	//------------------------------------------------------------------------------------------------
	int Next()
	{
		int hi = m_iState / Q;
		int lo = m_iState % Q;
		int s = A * lo - R * hi;
		if (s <= 0)
			s += M;

		m_iState = s;
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! min inclusive, max exclusive. Returns min if the range is empty.
	int Int(int min, int max)
	{
		if (max <= min)
			return min;

		return min + Next() % (max - min);
	}

	//------------------------------------------------------------------------------------------------
	float Float01()
	{
		int r = Next() % 1000000;
		return r / 1000000.0;
	}

	//------------------------------------------------------------------------------------------------
	float Range(float min, float max)
	{
		return min + (max - min) * Float01();
	}

	//------------------------------------------------------------------------------------------------
	bool Chance(float p)
	{
		return Float01() < p;
	}

	//------------------------------------------------------------------------------------------------
	string Pick(array<string> list)
	{
		if (list.IsEmpty())
			return "";

		return list[Int(0, list.Count())];
	}

	//------------------------------------------------------------------------------------------------
	void ShuffleInts(notnull array<int> list)
	{
		for (int i = list.Count() - 1; i > 0; i--)
		{
			int j = Int(0, i + 1);
			int tmp = list[i];
			list[i] = list[j];
			list[j] = tmp;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShuffleStrings(notnull array<string> list)
	{
		for (int i = list.Count() - 1; i > 0; i--)
		{
			int j = Int(0, i + 1);
			string tmp = list[i];
			list[i] = list[j];
			list[j] = tmp;
		}
	}
}

//! Templated text in the notes voice (docs/WRITING.md): personal, specific, a little wrong.
//! Everything takes an RZ_Rng so a mission's notes stay consistent with each other.
class RZ_NoteText
{
	//------------------------------------------------------------------------------------------------
	//! Everyone in the Zone is a man (the stack has only male bodies, docs/MISSION_RULES.md rule 10),
	//! and a note's name may be met later, so every first name is a man's.
	static string FirstName(RZ_Rng rng)
	{
		return ManName(rng);
	}

	//------------------------------------------------------------------------------------------------
	//! A man's first name, for someone who stands in the world: the base game's people are all men
	//! (its identity list has no women, SCR_EIdentityGender), so a body's name matches it
	//! (docs/MISSION_RULES.md, rule 10). FirstName is the same list.
	static string ManName(RZ_Rng rng)
	{
		array<string> names = {"Pavel", "Henri", "Radim", "Emil", "Anton", "Viktor", "Karel", "Oskar", "Milan", "Jakub",
			"Bruno", "Leon", "Rudolf", "Patrik", "Lukas", "Vilem", "Felix", "Otto", "Konrad", "Erik", "Stanislav", "Hugo", "Dominik", "Jan"};
		return rng.Pick(names);
	}

	//------------------------------------------------------------------------------------------------
	//! A saved name from before everyone met was a man (an older save): a woman's first name from
	//! the old lists becomes its man's name, the rest of the text as it was ("Marek Weber" becomes
	//! "Marek Weber", "Old Marek" "Old Marek"). Any other text comes back unchanged.
	static string ManFromOld(string text)
	{
		if (text.IsEmpty())
			return text;

		array<string> old = {"Marta", "Jana", "Lucie", "Ruth", "Irena", "Elise", "Nadia", "Vera", "Sabine", "Ilse", "Dana", "Agata",
			"Ada", "Hana", "Olga", "Edith", "Agnes", "Zofia", "Greta", "Mila"};
		array<string> men = {"Marek", "Jan", "Lukas", "Rudolf", "Ivo", "Emil", "Nikola", "Vaclav", "Samuel", "Ivan", "Dalibor", "Augustin",
			"Adam", "Hanus", "Oldrich", "Edmund", "Albin", "Zdenek", "Gregor", "Milos"};
		array<string> words = {};
		text.Split(" ", words, false);
		bool changed = false;
		for (int i = 0; i < words.Count(); i++)
		{
			int at = old.Find(words[i]);
			if (at < 0)
				continue;

			words[i] = men[at];
			changed = true;
		}

		if (!changed)
			return text;

		string joined = words[0];
		for (int k = 1; k < words.Count(); k++)
		{
			joined += " " + words[k];
		}

		return joined;
	}

	//------------------------------------------------------------------------------------------------
	static string Surname(RZ_Rng rng)
	{
		array<string> names = {"Novak", "Laurent", "Brandt", "Kowal", "Duval", "Mercer", "Horak", "Petit", "Vogel", "Sorel",
			"Kral", "Marchand", "Weber", "Dvorak", "Rousseau", "Lindqvist", "Benes", "Fournier", "Hale", "Moreau"};
		return rng.Pick(names);
	}

	//------------------------------------------------------------------------------------------------
	static string FullName(RZ_Rng rng)
	{
		return FirstName(rng) + " " + Surname(rng);
	}

	//------------------------------------------------------------------------------------------------
	//! Distinct first names (no repeats), for suspects, patients, researchers.
	static void DistinctFirstNames(RZ_Rng rng, int count, notnull array<string> result)
	{
		int guard = 0;
		while (result.Count() < count && guard < 200)
		{
			guard++;
			string n = FirstName(rng);
			if (!result.Contains(n))
				result.Insert(n);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Days are counted from the day the gates closed, like the broadcasts do.
	static string Day(int day)
	{
		return string.Format("Day %1", day);
	}

	//------------------------------------------------------------------------------------------------
	//! A line that makes a note feel like a person wrote it.
	static string PersonalLine(RZ_Rng rng, string place)
	{
		array<string> lines = {
			"Tell my sister I'm sorry about the car. I left it at the north road barrier on Day 2, keys in.",
			"If you find a brown dog, his name is Bruno. He doesn't bite, and he comes to a whistle. Take him to whoever's feeding people.",
			"The radio still says aid is on its way. I stopped listening on Day 4. The batteries are for the torch now.",
			"We had soup at the safehouse today. Real soup, potatoes in it, and nobody counted the bowls.",
			"I keep counting the bullets. The number doesn't change, and neither does the scratching at the back door.",
			"I left the key under the blue pot, for whoever comes after. The tins in the cellar are yours. Leave one jar of the jam: it was my mother's.",
			"My hands won't stop shaking since the scratch on Day 6. Must be the cold.",
			"Nobody answers at the school any more. Last week someone still waved a torch from the gym window at nine.",
			"If this reaches the Haven: we tried to bring the children to you. The bridge was down, and we went back.",
			"Don't drink from the stream past the bridge. Something has been lying in it upstream since Day 3."
		};

		string line = rng.Pick(lines);
		if (!place.IsEmpty() && rng.Chance(0.4))
			line = string.Format("No calls from the hills round %1 tonight, the first time in a week. %2", place, line);

		return line;
	}

	//------------------------------------------------------------------------------------------------
	static string Signoff(RZ_Rng rng, string name)
	{
		array<string> forms = {"- %1", "%1, if anyone asks", "(%1)", "- %1. Burn this."};
		return string.Format(rng.Pick(forms), name);
	}

	//------------------------------------------------------------------------------------------------
	//! Numbers as words, for spoken broadcasts ("four, seven, one").
	static string DigitWord(int d)
	{
		array<string> words = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
		if (d < 0 || d > 9)
			return "static";

		return words[d];
	}

	//------------------------------------------------------------------------------------------------
	//! "047" for 47 with width 3.
	static string Pad(int value, int width)
	{
		string s = value.ToString();
		while (s.Length() < width)
		{
			s = "0" + s;
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! "a dozen", "the fingers on one hand"... for small riddles; the plain number if there is no phrase.
	static string NumberPhrase(int n)
	{
		switch (n)
		{
			case 2: return "the eyes in your head";
			case 3: return "the wise men";
			case 4: return "the legs of a dog";
			case 5: return "the fingers on one hand";
			case 6: return "half a dozen";
			case 7: return "the days in a week";
			case 8: return "the legs of a spider";
			case 9: return "the lives of a cat";
			case 10: return "the fingers on both hands";
			case 12: return "a dozen";
			case 20: return "a score";
		}

		return n.ToString();
	}
}
