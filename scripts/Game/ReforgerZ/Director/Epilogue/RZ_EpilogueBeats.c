//! The epilogue's beats (docs/EPILOGUE.md, Beats): chosen once when the story commits, frozen
//! with their names, and checked again every time a carrier shows one. A beat's home is a mark at
//! its place (RZ_Marks, the epilogue pool: a prop when Epilogue > Board prefab is set, else an
//! approach line); leaders, wanderers, the radio and the journal echo it.
//!
//! Slots: the cure (the lab gate and its stand board), the hubs (the queue, the Names board, each
//! stronghold's place, each safehouse), outposts, towns from the town log (Epilogue > Towns at
//! most), packs still calling (4), the Long Table (only once it's exposed), the named dead (6),
//! side stories from the world records with an epilogue line (6), wanted posters and graves, and
//! since-the-cure beats added in the week (Epilogue > Since-the-cure beats).
class RZ_EpiBeats
{
	protected static const int PACKS = 4;
	protected static const int PEOPLE = 6;
	protected static const int STORIES = 6;
	protected static const int GRAVES = 12;
	protected static const float MET_RADIUS = 40;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Epilogue m_Epi;
	protected float m_fMetCheck;

	//------------------------------------------------------------------------------------------------
	void RZ_EpiBeats(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		m_GM = gm;
		m_Epi = epi;
	}

	//------------------------------------------------------------------------------------------------
	protected array<ref RZ_EpiBeat> List()
	{
		return m_Epi.Data().beats;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_EpiBeat Add(string key, string slot, string place, vector pos, string text, string echo)
	{
		RZ_EpiBeat b = new RZ_EpiBeat();
		b.key = key;
		b.slot = slot;
		b.place = place;
		b.x = pos[0];
		b.z = pos[2];
		b.texts.Insert(text);
		b.echo = echo;
		b.weight = 1;
		List().Insert(b);
		RZ_Log.Info(string.Format("Epilogue: beat %1 at %2 (weight %3)", key, place, b.weight));
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! A beat from the fuller record (RZ_EpiRecord): the roads, the stations, the characters, one a
	//! faction. Never twice.
	RZ_EpiBeat AddRecord(string key, string slot, string place, vector pos, string text, string echo, int faction)
	{
		if (Find(key))
			return null;

		RZ_EpiBeat b = Add(key, slot, place, pos, text, echo);
		b.faction = faction;
		return b;
	}

	//------------------------------------------------------------------------------------------------
	RZ_EpiBeat Find(string key)
	{
		foreach (RZ_EpiBeat b : List())
		{
			if (b.key == key)
				return b;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector Pos(RZ_EpiBeat b)
	{
		return Vector(b.x, 0, b.z);
	}

	// ================================================================ Choosing
	//------------------------------------------------------------------------------------------------
	//! Every slot, in the table's order. The season's seed breaks ties.
	void Build()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		RZ_EpiSpots spots = m_Epi.GetSpots();
		int route = m_Epi.Route();
		int terms = s.terms;

		// The cure: the lab gate and the stand board (never on the Unsigned last day)
		if (m_GM.HasLab() && !s.lastDay)
		{
			vector gate = spots.LabGatePos();
			Add("lab", "cure", "the lab gate", gate, RZ_EpiText.LabMark(route, terms, s.faction), "There's a board by the lab gate. Worth reading.");
			string board = RZ_EpiText.StandBoard(s.how, s.cureDay, RZ_CureFinale.RouteName(route));
			RZ_EpiBeat stand = Add("stand", "cure", "the lab gate", gate + Vector(6, 0, 0), board, "There's a board by the lab gate with names on it.");
			foreach (string n : s.standNames)
			{
				if (stand.names.Count() < 8)
					stand.names.Insert(n);
			}
		}

		if (s.lastDay)
		{
			BuildTallies();
		}
		else
		{
			vector queue = spots.QueuePos();
			if (queue != vector.Zero)
			{
				Add("queue", "hub", "the north road", queue, "", "There's a queue on the verge of the north road. It hasn't moved.");
				Add("names", "hub", "the bend on the north road", spots.NamesPos(), "", "There's a board at the bend on the north road. Names on it.");
			}
		}

		BuildStrongholds();
		BuildHouses();
		BuildPosts();
		BuildTowns();
		BuildPacks();
		BuildTable();
		BuildDead();
		BuildStories();
		BuildWanted();
		BuildGraves();
		Weigh();
		m_Epi.GetRecord().OnBeats(this);
	}

	//------------------------------------------------------------------------------------------------
	//! The Unsigned last day: chalk tallies 150 m up from each barrier, counting to the day.
	protected void BuildTallies()
	{
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (!perimeter)
			return;

		array<vector> points = {};
		perimeter.GetCheckpoints(points);
		int day = RZ_WorldFacts.Get().Day();
		foreach (int i, vector c : points)
		{
			vector at = c + perimeter.InwardAt(c) * 150;
			Add("tally." + i.ToString(), "hub", "a barrier", at, string.Format("Chalk tallies on a board, counting to %1. Somebody's crossed them in fives.", day), "Somebody's counting the days on a board by the barrier.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildStrongholds()
	{
		RZ_EpiSpots spots = m_Epi.GetSpots();
		RZ_Leader co = RZ_MajorHelp.Top(m_GM, RZ_EFaction.MILITARY);
		if (co)
		{
			RZ_EpiBeat roster = Add("roster", "hub", co.m_sPlace, RZ_MajorHelp.NearSeat(co.m_vPos), "", "Bastion's roster board has a new column: NO REPLY.");
			roster.subj = "leader";
			roster.subjId = co.m_iId;
			roster.subjVal = co.m_iGeneration;
			roster.faction = RZ_EFaction.MILITARY;
		}

		vector cross = spots.CrossroadsPos();
		if (cross != vector.Zero)
		{
			RZ_EpiBeat book = Add("crossroads", "hub", "Rudi's crossroads", cross, "", "There's a price board at Rudi's crossroads. Read it before you pay.");
			book.faction = RZ_EFaction.BANDITS;
		}

		RZ_Leader chair = RZ_MajorHelp.Top(m_GM, RZ_EFaction.SURVIVORS);
		if (chair)
		{
			RZ_EpiBeat supper = Add("supper", "hub", chair.m_sPlace, RZ_MajorHelp.NearSeat(chair.m_vPos), "", "The Haven reads its dead at supper. The names are pinned up after.");
			supper.faction = RZ_EFaction.SURVIVORS;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One beat per safehouse: candles, the rota, beds in the yard, how the dose reaches the house.
	protected void BuildHouses()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			RZ_EpiBeat b = Add("house." + i.ToString(), "hub", h.m_sName, RZ_Mission.LandPointNear(h.m_vPos, 8, 14), "", string.Format("There are candles on the step at %1.", h.m_sName));
			b.subj = "house";
			b.subjId = i;
			b.subjVal = h.m_iPopulation;
			b.faction = RZ_EFaction.SURVIVORS;
			b.weight = 1 + Math.Min(4, h.m_iSeasonDied) * 0.5;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One beat per outpost: its owner's board at the gate.
	protected void BuildPosts()
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return;

		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h.m_bStronghold)
				continue;

			string text = string.Format("%1's board at the gate of %2, repainted. The old coats show through at the edges.", RZ_Factions.GetShortName(h.m_eFaction), h.m_sName);
			RZ_EpiBeat b = Add("post." + h.m_iId.ToString(), "place", h.m_sName, RZ_Mission.LandPointNear(h.m_vPos, 20, 35), text, string.Format("The board at %1's gate has been repainted.", h.m_sName));
			b.subj = "owner";
			b.subjId = h.m_iId;
			b.subjVal = h.m_eFaction;
			b.faction = h.m_eFaction;
			b.since = string.Format("The board at %1's gate has a new coat of paint over the old one. Different colours.", h.m_sName);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Towns from the town log: the best slot first, then the worst; Epilogue > Towns at most.
	protected void BuildTowns()
	{
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		array<int> towns = {};
		array<float> scores = {};
		foreach (int idx, RZ_Settlement s : m_GM.GetSettlements())
		{
			RZ_TownEntry best = mem.TownBest(idx);
			RZ_TownEntry worst = mem.TownWorst(idx);
			float score = 0;
			if (best)
				score = best.salience + 0.5;

			if (worst && worst.salience > score)
				score = worst.salience;

			if (score <= 0)
				continue;

			towns.Insert(idx);
			scores.Insert(score + 0.001 * idx);
		}

		int cap = Math.Max(0, m_Epi.GetSettings().m_iTowns);
		int placed = 0;
		while (placed < cap && !towns.IsEmpty())
		{
			int pick = 0;
			for (int j = 1; j < towns.Count(); j++)
			{
				if (scores[j] > scores[pick])
					pick = j;
			}

			int town = towns[pick];
			towns.Remove(pick);
			scores.Remove(pick);
			RZ_Settlement st = m_GM.GetSettlement(town);
			RZ_TownEntry e = mem.TownBest(town);
			RZ_TownEntry other = mem.TownWorst(town);
			if (!e || (other && other.salience > e.salience))
			{
				e = other;
				other = mem.TownBest(town);
			}

			string echo = string.Format("There's something on the road into %1.", st.m_sName);
			if (other && other != e)
				echo = string.Format("There's something on the road into %1. The town behind it has another story.", st.m_sName);

			vector at = RZ_Util.GroundPos(st.m_vCenter + Vector(st.m_fRadius * 0.6, 0, 0));
			// Traces' census of the town's doors after the town log's line (docs/TRACES.md)
			RZ_EpiBeat b = Add("town." + town.ToString(), "place", st.m_sName, at, RZ_Traces.WithCensus(e.text, town), echo);
			b.weight = e.salience;
			foreach (int k, string id : e.idents)
			{
				if (k < e.names.Count() && !e.names[k].IsEmpty())
				{
					b.idents.Insert(id);
					b.names.Insert(e.names[k]);
				}
			}

			placed++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Packs still calling at the cure: their faction marks where a road enters the range.
	protected void BuildPacks()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes)
			return;

		int n = 0;
		foreach (int i, int id : s.packIds)
		{
			if (n >= PACKS)
				break;

			RZ_Tribe t = tribes.Find(id);
			if (!t)
				continue;

			string place = s.packPlaces[i];
			RZ_EpiBeat b = Add("pack." + id.ToString(), "place", place, RZ_Mission.LandPointNear(t.m_vLair, 500, 700), string.Format("Chalk on a post: the lot at %1 still call every evening. Fewer answer now. NOT AFTER 7.", place), string.Format("The lot at %1 still call at dusk. Fewer answer.", place));
			b.subj = "pack";
			b.subjId = id;
			b.since = string.Format("The chalk on the post about the lot at %1 has a line through it.", place);
			n++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Long Table: never described, and nothing at all until it's exposed.
	protected void BuildTable()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		RZ_LongTable table = m_GM.GetLongTable();
		if (!table || facts.Flag("table.exposed") == 0)
			return;

		string text = "An amber ribbon on a farm gate. Inside, a long table laid for one.";
		if (facts.Flag("table.burnt") != 0)
			text = "A Bastion stencil board at a farm lane: CLEARED.";
		else if (facts.Flag("table.broken") != 0)
			text = "A farmhouse with the chairs turned to the wall.";

		RZ_EpiBeat b = Add("table", "place", "a farm", RZ_Mission.LandPointNear(table.GetHousePos(), 60, 90), text, "There's a farm nobody goes near now.");
		b.subj = "table";
	}

	//------------------------------------------------------------------------------------------------
	//! The named dead (wanderers): their name chalked on the last door they rested at.
	protected void BuildDead()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		int n = 0;
		foreach (int i, RZ_EpiName d : m_Epi.Data().names)
		{
			if (n >= PEOPLE || !d.ident.IsEmpty() || d.faction >= 0)
				continue;

			vector at = Vector(d.x, 0, d.z);
			string door = d.lies;
			if (houses)
			{
				RZ_Safehouse h = houses.Get(houses.FindNearest(at));
				if (h)
				{
					at = RZ_Mission.LandPointNear(h.m_vPos, 10, 16);
					door = h.m_sName;
				}
			}

			Add("dead." + i.ToString(), "people", door, at, string.Format("A name chalked on a door at %1: %2. %3.", door, d.name, RZ_Util.Capitalize(d.role)), string.Format("Somebody chalked a name on a door at %1.", door));
			RZ_Traces.ChalkName(at, d.name, "dead." + i.ToString());
			n++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Side stories: world records with an epilogue line, by importance, at their subject's place.
	protected void BuildStories()
	{
		array<RZ_WorldRecord> records = {};
		RZ_WorldFacts.Get().Memory().AllRecords(records);
		int n = 0;
		for (int pass = 12; pass >= 0 && n < STORIES; pass--)
		{
			foreach (RZ_WorldRecord r : records)
			{
				if (n >= STORIES || r.epilogue.IsEmpty() || r.importance != pass)
					continue;

				vector at;
				string place;
				if (!SubjectPlace(r.subject, at, place))
					continue;

				RZ_EpiBeat b = Add("story." + r.key + "." + r.subject, "side", place, at, r.epilogue, string.Format("Something worth seeing at %1.", place));
				b.weight = r.importance;
				foreach (int k, string id : r.idents)
				{
					if (k < r.names.Count() && !r.names[k].IsEmpty())
					{
						b.idents.Insert(id);
						b.names.Insert(r.names[k]);
					}
				}

				n++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "town:7" and "safehouse:2" have a place; other subjects have none (no beat).
	protected bool SubjectPlace(string subject, out vector at, out string place)
	{
		at = vector.Zero;
		place = "";
		if (subject.StartsWith("town:"))
		{
			int t = subject.Substring(5, subject.Length() - 5).ToInt();
			RZ_Settlement s = m_GM.GetSettlement(t);
			if (!s)
				return false;

			at = RZ_Mission.LandPointNear(s.m_vCenter, s.m_fRadius * 0.4, s.m_fRadius * 0.7);
			place = s.m_sName;
			return true;
		}

		if (subject.StartsWith("safehouse:") && m_GM.GetSafehouses())
		{
			int idx = subject.Substring(10, subject.Length() - 10).ToInt();
			RZ_Safehouse h = m_GM.GetSafehouses().Get(idx);
			if (!h)
				return false;

			at = RZ_Mission.LandPointNear(h.m_vPos, 12, 20);
			place = h.m_sName;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Wanted posters at the hunting faction's stronghold (or its nearest holding).
	protected void BuildWanted()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		foreach (int i, string ident : s.wantIdents)
		{
			RZ_Leader top = RZ_MajorHelp.Top(m_GM, s.wantFaction[i]);
			if (!top)
				continue;

			vector at = RZ_Mission.LandPointNear(top.m_vPos, 25, 40);
			RZ_EpiBeat b = Add("wanted." + i.ToString(), "people", top.m_sPlace, at, "", string.Format("There's a wanted poster up at %1.", top.m_sPlace));
			b.subj = "wanted";
			b.subjId = i;
			b.idents.Insert(ident);
			b.names.Insert(s.wantNames[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Notable players' dead characters: a grave, 100 m from any other, 150 m from a safehouse.
	protected void BuildGraves()
	{
		array<vector> placed = {};
		RZ_Safehouses houses = m_GM.GetSafehouses();
		foreach (int i, RZ_EpiName d : m_Epi.Data().names)
		{
			if (d.ident.IsEmpty() || placed.Count() >= GRAVES)
				continue;

			vector at = Vector(d.x, 0, d.z);
			bool crowded = false;
			foreach (vector p : placed)
			{
				if (RZ_Util.DistXZ(p, at) < 100)
					crowded = true;
			}

			if (houses && houses.FindNearest(at, 150) >= 0)
				at = RZ_Mission.LandPointNear(at, 150, 300);

			if (crowded)
				continue;

			placed.Insert(at);
			RZ_EpiBeat b = Add("grave." + i.ToString(), "people", d.lies, at, "", string.Format("There's a grave on the verge %1. Two fence palings and a name.", d.lies));
			b.idents.Insert(d.ident);
			b.names.Insert(d.name);
			string role = d.role;
			role.ToUpper();
			b.extra = string.Format("%1. DAY %2", role, d.day);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The weight: base x people (1 + 0.25 a named identity, x2 at most) x known (3 or more
	//! visitors, x1.25). Lists (the stand board) are exempt from the name caps.
	protected void Weigh()
	{
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		foreach (RZ_EpiBeat b : List())
		{
			float people = Math.Min(2, 1 + 0.25 * b.idents.Count());
			b.weight = b.weight * people;
			if (b.key.StartsWith("town."))
			{
				int t = b.key.Substring(5, b.key.Length() - 5).ToInt();
				int visits = 0;
				foreach (string id : m_Epi.Data().playerIdents)
				{
					if (mem.Visited(t, id))
						visits++;
				}

				if (visits >= 3)
					b.weight = b.weight * 1.25;
			}
		}
	}

	// ================================================================ Since the cure
	//------------------------------------------------------------------------------------------------
	//! A new beat in the week (a town taken back, a name brought home, a closing stage done): up to
	//! Epilogue > Since-the-cure beats.
	void AddSince(string key, string place, vector pos, string text, string echo, string ident = "", string name = "")
	{
		RZ_EpilogueSave s = m_Epi.Data();
		if (s.sinceCount >= m_Epi.GetSettings().m_iSinceBeats || Find("since." + key))
			return;

		s.sinceCount++;
		RZ_EpiBeat b = Add("since." + key, "since", place, pos, text, echo);
		if (!ident.IsEmpty())
		{
			b.idents.Insert(ident);
			b.names.Insert(name);
		}

		Place(b);
	}

	// ================================================================ Showing
	//------------------------------------------------------------------------------------------------
	//! Its subjects checked again: the since version when one changed, or dropped (lookups only).
	protected void Check(RZ_EpiBeat b)
	{
		if (b.dropped || b.sinceOn || b.subj.IsEmpty())
			return;

		bool changed = false;
		string why = "";
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (b.subj == "leader")
		{
			changed = facts.LeaderGeneration(b.subjId) != b.subjVal;
			why = "a new leader";
		}
		else if (b.subj == "owner")
		{
			RZ_Territory territory = m_GM.GetTerritory();
			RZ_Holding h;
			if (territory)
				h = territory.GetHolding(b.subjId);

			changed = !h || h.m_eFaction != b.subjVal;
			why = "a new owner";
		}
		else if (b.subj == "pack")
		{
			RZ_Tribes tribes = m_GM.GetTribes();
			RZ_Tribe t;
			if (tribes)
				t = tribes.Find(b.subjId);

			changed = !t || !t.m_bHasAlpha;
			why = "it went quiet";
		}
		else if (b.subj == "table")
		{
			changed = facts.Flag("table.exposed") == 0;
			why = "the farm changed";
		}

		if (!changed)
			return;

		if (b.since.IsEmpty())
		{
			b.dropped = true;
			RZ_Log.Info(string.Format("Epilogue: beat %1 dropped (%2)", b.key, why));
		}
		else
		{
			b.sinceOn = true;
			RZ_Log.Info(string.Format("Epilogue: beat %1 since (%2)", b.key, why));
		}

		Place(b);
	}

	//------------------------------------------------------------------------------------------------
	//! What the beat says now: checked, its version for the step, live numbers, names filtered.
	string Text(RZ_EpiBeat b)
	{
		Check(b);
		if (b.dropped)
			return "";

		if (b.sinceOn)
			return b.since;

		RZ_EpilogueSave s = m_Epi.Data();
		int step = m_Epi.Step();
		int route = m_Epi.Route();
		string text = "";
		if (!b.texts.IsEmpty())
			text = b.texts[0];

		if (b.key == "queue")
			return RZ_EpiText.QueueSign(step, m_Epi.GetSpots().QueueWaiting(), RZ_EpiText.QueueTable(route, s.terms)) + m_Epi.GetSpots().QueueExtra();

		if (b.key == "names")
			return m_Epi.GetClosing().WallText(false);

		if (b.key == "roster")
		{
			string net = string.Format("NO REPLY: one stroke for every 20:30 since the cure. %1 so far.", s.watchStrokes);
			if (s.choices[1] == 2)
				net = net + " NET CLOSED.";

			return "The roster board at Bastion's stronghold. " + net;
		}

		if (b.key == "crossroads")
		{
			if (route == RZ_CureFinale.REDHAND && s.terms == 1)
				return RZ_EpiText.PriceBoard(step, s.choices[2] == 2);

			if (RZ_MajorHelp.World(m_GM, "book.coda") != 0)
				return "The lorry at the crossroads, with the book nailed open at the last page. PAID beside some of the lines.";

			return "The lorry at the crossroads. The boss keeps the book on a knee.";
		}

		if (b.key == "supper")
		{
			if (s.choices[0] == 2)
				return "The house board: the names read at supper, and under them, the ones the Haven put out.";

			return "The house board: the names read at supper, then a gap for the ones we don't say.";
		}

		if (b.key.StartsWith("house."))
			return HouseText(b, step) + b.more;

		if (b.key.StartsWith("wanted."))
		{
			int w = b.subjId;
			return RZ_EpiText.Poster(Named(b, 0), s.wantFaction[w], s.wantState[w], RZ_WorldFacts.Get().Day());
		}

		if (b.key.StartsWith("grave."))
		{
			string who = Named(b, 0);
			who.ToUpper();
			return string.Format("Two fence palings and a name in biro: %1. %2.", who, b.extra);
		}

		if (b.key == "stand")
		{
			string names = "";
			for (int i = 0; i < b.names.Count(); i++)
			{
				if (i > 0)
					names += ", ";

				names += RZ_EpiNames.Show("", b.names[i]);
			}

			if (!names.IsEmpty())
				text += " " + names + ".";

			return text;
		}

		return text + b.more;
	}

	//------------------------------------------------------------------------------------------------
	//! A beat's i-th name, as the names policy shows it.
	protected string Named(RZ_EpiBeat b, int i)
	{
		if (i >= b.names.Count())
			return "someone";

		string ident = "";
		if (i < b.idents.Count())
			ident = b.idents[i];

		return RZ_EpiNames.Show(ident, b.names[i]);
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse: candles and the number, the rota or the beds in the yard, how the dose comes.
	protected string HouseText(RZ_EpiBeat b, int step)
	{
		RZ_Safehouse h = m_GM.GetSafehouses().Get(b.subjId);
		if (!h)
			return "";

		RZ_EpilogueSave s = m_Epi.Data();
		string text = RZ_EpiText.Candles(step, h.m_iSeasonDied);
		int tierAt = 0;
		if (b.subjId < s.houseTier.Count())
			tierAt = s.houseTier[b.subjId];

		string more = "";
		if (h.m_iPopulation <= 0)
			more = "Beds stacked in the yard, and nobody to sleep in them.";
		else if (h.GetTier() > tierAt)
			more = string.Format("A new rota on the door: ROTA. %1 NAMES. NO EXCUSES.", h.m_iPopulation);
		else if (h.GetTier() < tierAt || h.m_bShort)
			more = "Beds stacked in the yard. By the pot: half portions until the gates open.";

		if (!more.IsEmpty())
			text = (text + " " + more).Trim();

		if (step >= 3)
		{
			int gone = m_Epi.GetClosing().GoneHome(b.subjId);
			if (gone > 0)
			{
				text = (text + string.Format(" Chalked by the door: GONE HOME: %1.", gone)).Trim();
				RZ_Traces.GoneHome(b.subjId, gone);
			}
		}

		string door = m_Epi.GetOutlets().HouseNotice(b.subjId);
		if (!door.IsEmpty())
			text = (text + " " + door).Trim();

		return text;
	}

	// ================================================================ Marks
	//------------------------------------------------------------------------------------------------
	//! Every beat's home mark (after the commit, after a restart, at each step).
	void PlaceAll()
	{
		// The cure's, the lab's and the queue's home marks first, then by weight; at most 3 in any
		// 300 m (the rest stay text: approach lines). The marks pool keeps the server and cluster caps.
		array<RZ_EpiBeat> order = {};
		foreach (RZ_EpiBeat b : List())
		{
			int at = order.Count();
			for (int i = 0; i < order.Count(); i++)
			{
				if (Rank(b) > Rank(order[i]))
				{
					at = i;
					break;
				}
			}

			order.InsertAt(b, at);
		}

		array<vector> up = {};
		RZ_Marks marks = RZ_Marks.Get();
		foreach (RZ_EpiBeat o : order)
		{
			vector p = Pos(o);
			int near = 0;
			foreach (vector u : up)
			{
				if (RZ_Util.DistXZ(u, p) <= 300)
					near++;
			}

			if (near >= 3 && o.slot != "cure" && o.slot != "hub")
			{
				if (marks)
					marks.Remove("epi." + o.key);

				continue;
			}

			Place(o);
			up.Insert(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static float Rank(RZ_EpiBeat b)
	{
		if (b.slot == "cure")
			return 1000 + b.weight;

		if (b.key == "queue" || b.key == "names" || b.slot == "hub")
			return 500 + b.weight;

		return b.weight;
	}

	//------------------------------------------------------------------------------------------------
	protected void Place(RZ_EpiBeat b)
	{
		RZ_Marks marks = RZ_Marks.Get();
		if (!marks)
			return;

		string key = "epi." + b.key;
		string text = Text(b);
		if (text.IsEmpty())
		{
			marks.Remove(key);
			return;
		}

		ResourceName prefab = m_Epi.GetSettings().m_sBoardPrefab;
		if (b.key.StartsWith("since.town") || b.key.StartsWith("house."))
		{
			if (!m_Epi.GetSettings().m_sCandlePrefab.IsEmpty())
				prefab = m_Epi.GetSettings().m_sCandlePrefab;
		}

		prefab = m_Epi.GetRest().PrefabFor(b, prefab);
		int salience = RZ_Util.RoundToInt(Math.Clamp(b.weight, 1, 5));
		bool protect = b.slot == "cure" || b.slot == "hub";
		marks.Place(RZ_EMarkPool.EPILOGUE, key, Pos(b), prefab, text, salience, -1, protect, b.slot != "cure");
	}

	//------------------------------------------------------------------------------------------------
	//! The evolving step moved (or a dusk came): the marks show their new versions.
	void Refresh()
	{
		PlaceAll();
	}

	//------------------------------------------------------------------------------------------------
	//! The first dusk of the week: the losing routes' marks come down (none are built: the winner's
	//! are the lab gate's), the candles are lit.
	void OnFirstDusk()
	{
		PlaceAll();
	}

	//------------------------------------------------------------------------------------------------
	//! Every few seconds: who has met which beat (within 40 m of its place).
	void TickPlayers(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			RZ_EpiBeat b = Nearest(p.GetOrigin(), MET_RADIUS);
			if (b)
				m_Epi.Met(ids[i], b.key);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_EpiBeat Nearest(vector pos, float radius)
	{
		RZ_EpiBeat best;
		float bestDist = radius;
		foreach (RZ_EpiBeat b : List())
		{
			if (b.dropped)
				continue;

			float d = RZ_Util.DistXZ(pos, Pos(b));
			if (d <= bestDist)
			{
				bestDist = d;
				best = b;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The echo of the nearest beat within `radius` (a place and one detail), "" if none.
	string NearestEcho(vector pos, float radius)
	{
		RZ_EpiBeat b = Nearest(pos, radius);
		if (!b || Text(b).IsEmpty())
			return "";

		return b.echo;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest beat's text within `radius` (a leader's "This place"), "" if none.
	string PlaceLine(vector pos, float radius)
	{
		RZ_EpiBeat b = Nearest(pos, radius);
		if (!b)
			return "";

		return Text(b);
	}

	//------------------------------------------------------------------------------------------------
	//! "Where to look": two places with beats the player hasn't met yet, nearest first, no markers.
	string WhereToLook(int playerId, vector pos)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		array<RZ_EpiBeat> picks = {};
		for (int n = 0; n < 2; n++)
		{
			RZ_EpiBeat best;
			float bestDist = float.MAX;
			foreach (RZ_EpiBeat b : List())
			{
				if (b.dropped || picks.Contains(b) || m_Epi.Data().met.Contains(ident + "|" + b.key) || b.slot == "people")
					continue;

				float d = RZ_Util.DistXZ(pos, Pos(b));
				if (d < bestDist)
				{
					bestDist = d;
					best = b;
				}
			}

			if (best)
				picks.Insert(best);
		}

		if (picks.IsEmpty())
			return "";

		if (picks.Count() == 1)
			return picks[0].echo;

		return picks[0].echo + " " + picks[1].echo;
	}

	//------------------------------------------------------------------------------------------------
	//! The journal's "As the Zone tells it": every beat met (all of them in processing), 12 at most.
	string MetText(string ident, bool all, int most = 12)
	{
		string text = "";
		int n = 0;
		foreach (RZ_EpiBeat b : List())
		{
			if (n >= most)
				break;

			if (!all && !m_Epi.Data().met.Contains(ident + "|" + b.key))
				continue;

			string line = Text(b);
			if (line.IsEmpty())
				continue;

			text += string.Format("%1: %2\n", RZ_Util.Capitalize(b.place), line);
			n++;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! An admin strike: a beat key is dropped; a name is shown as a title from now on.
	void Strike(string what)
	{
		RZ_EpiBeat b = Find(what);
		if (b)
		{
			b.dropped = true;
			RZ_Log.Info(string.Format("Epilogue: beat %1 dropped (struck)", b.key));
		}

		PlaceAll();
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return List().Count();
	}
}
