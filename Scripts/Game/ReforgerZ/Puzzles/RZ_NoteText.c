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
		return (Next() % 1000000) / 1000000.0;
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

//! Templated text in the LORE voice: personal, specific, a little wrong.
//! Everything takes an RZ_Rng so a mission's notes stay consistent with each other.
class RZ_NoteText
{
	//------------------------------------------------------------------------------------------------
	static string FirstName(RZ_Rng rng)
	{
		array<string> names = {"Marta", "Pavel", "Henri", "Jana", "Tomas", "Lucie", "Emil", "Ruth", "Anton", "Irena",
			"Viktor", "Elise", "Karel", "Nadia", "Oskar", "Vera", "Milan", "Sabine", "Jakub", "Ilse", "Bruno", "Dana", "Leon", "Agata"};
		return rng.Pick(names);
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
			"Tell my sister I'm sorry about the car.",
			"If you find the dog, his name is Bruno. He doesn't bite.",
			"The radio still says aid is coming. I stopped listening on Day 4.",
			"We had soup today. Real soup.",
			"I keep counting the bullets. The number doesn't change.",
			"I left the key under the blue pot, for whoever comes after.",
			"My hands won't stop shaking. Must be the cold.",
			"Nobody answers at the school any more.",
			"If this reaches the Haven: we tried.",
			"Don't drink from the stream past the bridge."
		};

		string line = rng.Pick(lines);
		if (!place.IsEmpty() && rng.Chance(0.4))
			line = string.Format("%1 is quiet tonight. Too quiet. %2", place, line);

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
