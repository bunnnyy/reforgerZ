//! Session headlines (docs/DESIGN.md, "Story arcs and headlines"). The news itself is kept where it
//! always was (RZ_Leaders.AddHeadline: leader news, territory, faction life, story arcs, the
//! power grid, burned towns). This adds what the headlines didn't have yet and builds the
//! 3-4 lines a player gets on their first spawn of a session ("Word on the roads"):
//!
//! - the latest headline (outposts changing hands, leaders, arcs...),
//! - the last wanderer lost on the roads,
//! - the cure percentage,
//! - the biggest rescue so far this season,
//! - now and then a dead character's journal (town memory) or a line of Haven gossip:
//!   a hungry safehouse going to half portions, or "borrowing" from another.
//!
//! Plain data, saved. Never broadcast: players see it when they join.

//------------------------------------------------------------------------------------------------
class RZ_HeadlineSave
{
	string wandererLost;
	int rescueCount;
	string rescuePlace;
}

//------------------------------------------------------------------------------------------------
class RZ_Headlines
{
	protected RZ_GameModeComponent m_GM;
	protected string m_sWandererLost;
	protected int m_iRescueCount;
	protected string m_sRescuePlace;

	//------------------------------------------------------------------------------------------------
	void RZ_Headlines(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_HeadlineSave saved)
	{
		if (!saved)
			return;

		m_sWandererLost = saved.wandererLost;
		m_iRescueCount = saved.rescueCount;
		m_sRescuePlace = saved.rescuePlace;
	}

	//------------------------------------------------------------------------------------------------
	RZ_HeadlineSave Save()
	{
		RZ_HeadlineSave data = new RZ_HeadlineSave();
		data.wandererLost = m_sWandererLost;
		data.rescueCount = m_iRescueCount;
		data.rescuePlace = m_sRescuePlace;
		return data;
	}

	// ================================================================ Hooks
	//------------------------------------------------------------------------------------------------
	//! A named wanderer died (RZ_Wanderers.Die): "Old Marta the trader was killed near Morton."
	void OnWandererLost(string text)
	{
		m_sWandererLost = text;
	}

	//------------------------------------------------------------------------------------------------
	//! People rescued into a safehouse (RZ_Safehouses.AddRescued). Keeps the biggest.
	void OnRescue(int count, string place)
	{
		if (count <= m_iRescueCount)
			return;

		m_iRescueCount = count;
		m_sRescuePlace = place;
	}

	// ================================================================ Haven flavour
	//------------------------------------------------------------------------------------------------
	//! A line of Haven gossip for the headlines or a wanderer: a hungry safehouse on half
	//! portions, or "borrowing" from another. "" if every safehouse is fed.
	string HavenFlavour()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || houses.Count() == 0)
			return "";

		RZ_Safehouse hungry;
		RZ_Safehouse fed;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h)
				continue;

			if (h.m_bShort && !hungry)
				hungry = h;
			else if (!h.m_bShort && (!fed || h.m_iFood > fed.m_iFood))
				fed = h;
		}

		if (!hungry)
			return "";

		if (fed && Math.RandomFloat01() < 0.5)
			return string.Format("The safehouse in %1 \"borrowed\" two sacks of flour from %2. Nobody's calling it stealing. Yet.", hungry.m_sName, fed.m_sName);

		return string.Format("The safehouse in %1 has gone to half portions. The children eat first.", hungry.m_sName);
	}

	// ================================================================ The session news
	//------------------------------------------------------------------------------------------------
	//! 3-4 lines of server news for a player's first spawn of a session.
	string Build()
	{
		array<string> lines = {};

		// 1. The latest thing that happened (outposts, leaders, arcs, power...)
		RZ_Leaders leaders = m_GM.GetLeaders();
		array<string> headlines;
		if (leaders)
			headlines = leaders.GetHeadlines();

		if (headlines && !headlines.IsEmpty())
			lines.Insert(headlines[headlines.Count() - 1]);

		// 2. Lost on the roads
		if (!m_sWandererLost.IsEmpty())
			lines.Insert("Lost on the roads: " + m_sWandererLost);

		// 3. The biggest rescue so far
		if (m_iRescueCount > 0)
			lines.Insert(string.Format("Biggest rescue so far: %1 people brought into %2.", m_iRescueCount, m_sRescuePlace));

		// 4. Now and then: a journal, or Haven gossip; else an older headline
		string extra = "";
		RZ_TownMemory mem = m_GM.GetTownMemory();
		if (mem && Math.RandomFloat01() < 0.5)
			extra = mem.LatestJournal();

		if (extra.IsEmpty())
			extra = HavenFlavour();

		if (extra.IsEmpty() && headlines && headlines.Count() >= 2)
			extra = headlines[headlines.Count() - 2];

		if (!extra.IsEmpty())
			lines.Insert(extra);

		// Three at most from the above, then the cure, so it's 3-4 lines with the cure always in
		while (lines.Count() > 3)
		{
			lines.Remove(lines.Count() - 1);
		}

		RZ_CureResearch cure = m_GM.GetCure();
		if (cure && cure.IsCured())
			lines.Insert("The cure is out. The gates are opening.");
		else if (cure)
			lines.Insert(string.Format("Halcyon says the cure is at %1 percent.", RZ_Util.RoundToInt(cure.m_fProgress)));

		string text = "";
		foreach (string line : lines)
		{
			text += line + "\n";
		}

		return text;
	}
}
