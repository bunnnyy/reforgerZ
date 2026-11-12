//! The epilogue's own evening calendar (docs/EPILOGUE.md, "The evening calendar"), after Last Rounds:
//! - the Haven's supper reading: once the Gate has run, at 18:30 each game day the chair reads the
//!   season's dead on the Haven's open channel (the house boards keep the names as its standing
//!   version);
//! - Bastion's 20:30 watch: once the last call has run, and unless the CO closed the net, five words
//!   on Bastion's open channel each evening and a stroke on the roster board;
//! - second airings: each coda again, words only, on its faction's open channel, on the first evening
//!   at least 8 real hours (x Test scale) after it ran.
//! Radio and words only: nothing here takes the set-piece calendar's evening. Read on the epilogue's
//! check with players online. Server only.
class RZ_EpiCalendar
{
	protected static const int HOUR = 3600;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Epilogue m_Epi;

	//------------------------------------------------------------------------------------------------
	void RZ_EpiCalendar(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		m_GM = gm;
		m_Epi = epi;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_EpilogueSave S()
	{
		return m_Epi.Data();
	}

	//------------------------------------------------------------------------------------------------
	void Tick()
	{
		if (!RZ_GameTime.IsCalibrated() || !m_GM.GetFactionLife())
			return;

		float hour = RZ_GameTime.GetHour();
		int day = RZ_WorldFacts.Get().Day();
		if (hour >= 18.5 && hour < 19.5 && S().supperDay != day)
			Supper(day);

		if (hour >= 20.5 && hour < 21.5 && S().watchDay != day)
			Watch(day);

		if (hour >= 18 && hour < 22)
			Airings();
	}

	//------------------------------------------------------------------------------------------------
	//! The chair reads the season's dead at supper, six names a night, round the list.
	protected void Supper(int day)
	{
		if (RZ_MajorHelp.World(m_GM, "every_name.coda") == 0)
			return;

		S().supperDay = day;
		array<string> names = {};
		foreach (RZ_EpiName n : S().names)
		{
			if (!n.name.IsEmpty() && !m_Epi.IsStruck(n.name))
				names.Insert(n.name);
		}

		string line = "Supper, and the names. Tonight we read the ones we never had names for.";
		if (!names.IsEmpty())
		{
			string read = "";
			int count = Math.Min(6, names.Count());
			int start = (day * 6) % names.Count();
			for (int i = 0; i < count; i++)
			{
				if (i > 0)
					read += ", ";

				read += names[(start + i) % names.Count()];
			}

			line = string.Format("Supper, and the names. %1. And the ones we never had names for.", read);
		}

		if (S().choices[0] == 2)
			line += " And the ones we put out. Them too.";

		// In processing the clerk reads a few of the Last Entries after the names
		string entries = m_Epi.GetRest().SupperEntries();
		if (!entries.IsEmpty())
			line += " From the Names board. " + entries;

		m_GM.GetFactionLife().OpenChannelLine(RZ_EFaction.SURVIVORS, line);
		RZ_Log.Info("Epilogue: the supper reading.");
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's 20:30 watch: five minutes on the net, the same three sentences, a stroke on the board.
	protected void Watch(int day)
	{
		if (RZ_MajorHelp.World(m_GM, "orders.coda") == 0 || S().choices[1] == 2)
			return;

		S().watchDay = day;
		S().watchStrokes = S().watchStrokes + 1;
		m_GM.GetFactionLife().OpenChannelLine(RZ_EFaction.MILITARY, "Bastion listening. Nothing heard. Bastion out.");
		RZ_Log.Info(string.Format("Epilogue: the 20:30 watch (%1 strokes).", S().watchStrokes));
	}

	//------------------------------------------------------------------------------------------------
	//! Each coda once more, words only, at least 8 real hours after it ran, one an evening.
	protected void Airings()
	{
		int now = m_Epi.Now();
		int gap = RZ_Util.RoundToInt(8 * HOUR * m_Epi.Scale());
		array<string> keys = {"every_name", "orders", "proof", "book"};
		foreach (string key : keys)
		{
			if (RZ_MajorHelp.World(m_GM, key + ".coda") == 0 || S().aired.Contains(key))
				continue;

			int i = S().codaKeys.Find(key);
			if (i < 0)
			{
				S().codaKeys.Insert(key);
				S().codaUnix.Insert(now);
				continue;
			}

			// One an evening, unless phase B holds fewer evenings than there are codas
			bool sameDay = S().airedDay == RZ_WorldFacts.Get().Day() && !m_Epi.GetRest().TightEvenings();
			if (now - S().codaUnix[i] < gap || sameDay)
				continue;

			RZ_StoryRecord r = Story(key);
			if (!r)
			{
				S().aired.Insert(key);
				continue;
			}

			string line = RZ_Majors.LineFor(r, S().faction);
			S().aired.Insert(key);
			S().airedDay = RZ_WorldFacts.Get().Day();
			if (line.IsEmpty())
				continue;

			m_GM.GetFactionLife().OpenChannelLine(r.faction, "Again, for anyone who missed it. " + line);
			RZ_Log.Info("Epilogue: second airing of " + key);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_StoryRecord Story(string key)
	{
		if (!RZ_Majors.Get())
			return null;

		foreach (RZ_StoryRecord r : RZ_Majors.Get().GetStories())
		{
			if (r.major == key)
				return r;
		}

		return null;
	}
}
