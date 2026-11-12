//! The epilogue's fuller record and the beats in full (docs/EPILOGUE.md, "The season's record" and
//! "Beats"). All season, as data:
//! - the ledger: per faction trucks delivered and lost, outposts won and lost, leaders lost (a count
//!   and the last three names); per outpost its owners this season (three at most, in order) and its
//!   commander generations; the wanderers who died (name, role, where); each identity's longest
//!   life; hours each life spent Kill on Sight before the cure (Wanted means 4 or more). Each counter
//!   keeps the season day it started, and a line that reads one started late leaves the numbers out.
//! At the snapshot and after the beats are built:
//! - the change factor (x1.5 where a place ended up different) and the choice factor (x1.25 for a
//!   beat someone chose), one beat per faction, hubs at most 3 beats and strongholds 2, the roads'
//!   wrecks (the last 4 lost trucks), the stations (towns with lights), the characters (the Kid, the
//!   Preacher, leaders who died), the places in full (coats of paint, a lost post's notice, a
//!   commander's grave, the rescued-in line, the house rule's notice, bunting, towns' marks by kind,
//!   the Long Table's states), route marks (the losing routes' down at the first dusk);
//! - notables: up to 12, scored, each in at most 3 beats (2 on a server under 6), at least half the
//!   beats naming nobody, and on a server under 24 identities with deeds at most half of them named;
//! - titles: the identity's best life this season with a faction ("Don't know the face. Know the
//!   name."), "From <town>" from its highest-salience town log entry.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_EpiRecordSave
{
	ref array<string> keys = {};		// "truck.ok.1", "truck.lost.1", "post.won.2", "post.lost.2", "leader.lost.0"
	ref array<int> values = {};
	ref array<int> since = {};			// the season day each counter started
	ref array<string> leaders = {};		// "faction|name", newest last
	ref array<string> owners = {};		// "holding|f,f,f"
	ref array<string> gens = {};		// "holding|generations"
	ref array<string> wanderers = {};	// "name|role|where"
	ref array<string> lives = {};		// "ident|seconds|name"
	ref array<string> kos = {};			// "ident|life|seconds"
	int startDay = -1;
}

//------------------------------------------------------------------------------------------------
class RZ_EpiRecord
{
	protected static const float WANTED_SECONDS = 14400;
	protected static const int NOTABLES = 12;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Epilogue m_Epi;
	protected float m_fKos;

	//------------------------------------------------------------------------------------------------
	void RZ_EpiRecord(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		m_GM = gm;
		m_Epi = epi;
	}

	//------------------------------------------------------------------------------------------------
	RZ_EpiRecordSave R()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		if (!s.record)
			s.record = new RZ_EpiRecordSave();

		if (s.record.startDay < 0)
			s.record.startDay = RZ_WorldFacts.Get().Day();

		return s.record;
	}

	// ================================================================ The ledger
	//------------------------------------------------------------------------------------------------
	//! A ledger event from the world (RZ_FactionLife): kind is truck.ok, truck.lost, post.won,
	//! post.lost or leader.lost; text a name for leader.lost; holding the outpost, or -1.
	static void Note(string kind, int faction, string text = "", int holding = -1)
	{
		RZ_Epilogue epi = RZ_Epilogue.Get();
		if (!epi || !RZ_Epilogue.IsOn() || epi.IsBuilt())
			return;

		epi.GetRecord().Count(string.Format("%1.%2", kind, faction));
		if (kind == "leader.lost" && !text.IsEmpty())
			epi.GetRecord().AddLeader(faction, text);

		if (kind == "post.won")
		{
			int hold = holding;
			if (hold < 0)
				hold = HoldingNamed(text);

			if (hold >= 0)
				epi.GetRecord().AddOwner(hold, faction);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static int HoldingNamed(string place)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetTerritory() || place.IsEmpty())
			return -1;

		foreach (RZ_Holding h : gm.GetTerritory().GetHoldings())
		{
			if (h.m_sName == place)
				return h.m_iId;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	void Count(string key)
	{
		RZ_EpiRecordSave r = R();
		int i = r.keys.Find(key);
		if (i < 0)
		{
			r.keys.Insert(key);
			r.values.Insert(1);
			r.since.Insert(RZ_WorldFacts.Get().Day());
			return;
		}

		r.values[i] = r.values[i] + 1;
	}

	//------------------------------------------------------------------------------------------------
	int Value(string key)
	{
		int i = R().keys.Find(key);
		if (i < 0)
			return 0;

		return R().values[i];
	}

	//------------------------------------------------------------------------------------------------
	//! Started on the season's first day of the ledger (so its numbers can be read out).
	bool FromTheStart(string key)
	{
		int i = R().keys.Find(key);
		return i >= 0 && R().since[i] <= R().startDay + 1;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddLeader(int faction, string name)
	{
		R().leaders.Insert(string.Format("%1|%2", faction, name));
		while (R().leaders.Count() > 24)
		{
			R().leaders.Remove(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddOwner(int holding, int faction)
	{
		string head = holding.ToString() + "|";
		foreach (int i, string o : R().owners)
		{
			if (!o.StartsWith(head))
				continue;

			array<string> list = {};
			o.Substring(head.Length(), o.Length() - head.Length()).Split(",", list, true);
			if (!list.IsEmpty() && list[list.Count() - 1] == faction.ToString())
				return;

			list.Insert(faction.ToString());
			while (list.Count() > 3)
			{
				list.Remove(0);
			}

			string joined = "";
			foreach (int k, string f : list)
			{
				if (k > 0)
					joined += ",";

				joined += f;
			}

			R().owners[i] = head + joined;
			return;
		}

		R().owners.Insert(head + faction.ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! An outpost's owners this season, oldest first.
	void Owners(int holding, notnull array<int> result)
	{
		string head = holding.ToString() + "|";
		foreach (string o : R().owners)
		{
			if (!o.StartsWith(head))
				continue;

			array<string> list = {};
			o.Substring(head.Length(), o.Length() - head.Length()).Split(",", list, true);
			foreach (string f : list)
			{
				result.Insert(f.ToInt());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The last three leaders a faction lost, newest first.
	void LostLeaders(int faction, notnull array<string> result)
	{
		string head = faction.ToString() + "|";
		for (int i = R().leaders.Count() - 1; i >= 0 && result.Count() < 3; i--)
		{
			string l = R().leaders[i];
			if (l.StartsWith(head))
				result.Insert(l.Substring(head.Length(), l.Length() - head.Length()));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! All season: seconds each life spends Kill on Sight with someone (before the cure only).
	void SeasonTick(float dt, array<int> ids)
	{
		if (m_Epi.IsBuilt() || ids.IsEmpty())
			return;

		m_fKos += dt;
		if (m_fKos < 30)
			return;

		float step = m_fKos;
		m_fKos = 0;
		foreach (int pid : ids)
		{
			RZ_PlayerProfile p = m_GM.GetProfile(pid);
			if (!p || p.m_sIdentity.IsEmpty())
				continue;

			bool kos = false;
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (p.GetTier(f) == RZ_ETier.KILL_ON_SIGHT)
					kos = true;
			}

			if (kos)
				AddKos(p.m_sIdentity, p.m_iLives, step);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddKos(string ident, int life, float secs)
	{
		string head = string.Format("%1|%2|", ident, life);
		foreach (int i, string k : R().kos)
		{
			if (k.StartsWith(head))
			{
				float had = k.Substring(head.Length(), k.Length() - head.Length()).ToFloat();
				R().kos[i] = head + (had + secs).ToString();
				return;
			}
		}

		R().kos.Insert(head + secs.ToString());
		while (R().kos.Count() > 200)
		{
			R().kos.Remove(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Wanted: Kill on Sight for 4 or more hours played in that life before the cure.
	bool LongWanted(string ident, int life)
	{
		string head = string.Format("%1|%2|", ident, life);
		foreach (string k : R().kos)
		{
			if (k.StartsWith(head))
				return k.Substring(head.Length(), k.Length() - head.Length()).ToFloat() >= WANTED_SECONDS * m_Epi.Scale();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A player died (all season): the identity's longest life is kept.
	void OnDied(int victim)
	{
		if (m_Epi.IsBuilt())
			return;

		RZ_PlayerProfile p = m_GM.GetProfile(victim);
		if (!p || p.m_sIdentity.IsEmpty())
			return;

		string head = p.m_sIdentity + "|";
		string entry = string.Format("%1%2|%3", head, RZ_Util.RoundToInt(p.m_fLifeSeconds), RZ_Util.PlayerName(victim));
		foreach (int i, string l : R().lives)
		{
			if (!l.StartsWith(head))
				continue;

			array<string> parts = {};
			l.Split("|", parts, false);
			if (parts.Count() >= 2 && parts[1].ToFloat() < p.m_fLifeSeconds)
				R().lives[i] = entry;

			return;
		}

		R().lives.Insert(entry);
	}

	//------------------------------------------------------------------------------------------------
	//! At the snapshot: the wanderers the season lost.
	void Snapshot()
	{
		RZ_Wanderers w = m_GM.GetWanderers();
		if (!w)
			return;

		R().wanderers.Clear();
		for (int id = 0; id < 400 && R().wanderers.Count() < 12; id++)
		{
			RZ_Wanderer dead = w.FindById(id);
			if (!dead || dead.m_bAlive || dead.m_sName.IsEmpty())
				continue;

			R().wanderers.Insert(string.Format("%1|%2|%3", dead.GetShortName(), dead.GetRoleName(), m_Epi.PlaceWords(dead.m_vPos)));
		}
	}

	// ================================================================ The record's lines
	//------------------------------------------------------------------------------------------------
	//! The season's record as lines for "As the Zone tells it": per faction, then the roads.
	void Lines(notnull array<string> result)
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			string name = RZ_Util.Capitalize(RZ_Factions.GetNameInText(f));
			int ok = Value(string.Format("truck.ok.%1", f));
			int lost = Value(string.Format("truck.lost.%1", f));
			if (ok + lost > 0)
			{
				if (FromTheStart(string.Format("truck.lost.%1", f)) || FromTheStart(string.Format("truck.ok.%1", f)))
					result.Insert(string.Format("%1: %2 trucks in, %3 lost on the road.", name, ok, lost));
				else
					result.Insert(string.Format("%1 lost trucks on the road this season.", name));
			}

			int won = Value(string.Format("post.won.%1", f));
			int gone = Value(string.Format("post.lost.%1", f));
			if (won + gone > 0)
				result.Insert(string.Format("%1: %2 outposts taken, %3 lost.", name, won, gone));

			array<string> dead = {};
			LostLeaders(f, dead);
			if (!dead.IsEmpty())
			{
				string list = "";
				foreach (int i, string d : dead)
				{
					if (i > 0)
						list += ", ";

					list += d;
				}

				result.Insert(string.Format("%1 lost %2 of its leaders. The last: %3.", name, Value(string.Format("leader.lost.%1", f)), list));
			}
		}

		if (!R().wanderers.IsEmpty())
		{
			string roads = "";
			foreach (int k, string w : R().wanderers)
			{
				array<string> parts = {};
				w.Split("|", parts, false);
				if (parts.Count() < 2 || k >= 4)
					continue;

				if (!roads.IsEmpty())
					roads += "; ";

				roads += parts[0];
			}

			result.Insert("The roads lost " + roads + ".");
		}

		string longest = Longest();
		if (!longest.IsEmpty())
			result.Insert(longest);
	}

	//------------------------------------------------------------------------------------------------
	//! The longest life on the server this season, "" without one.
	protected string Longest()
	{
		string best = "";
		float bestSecs = 0;
		foreach (string l : R().lives)
		{
			array<string> parts = {};
			l.Split("|", parts, false);
			if (parts.Count() < 3 || parts[1].ToFloat() <= bestSecs)
				continue;

			bestSecs = parts[1].ToFloat();
			best = RZ_EpiNames.Show(parts[0], parts[2]);
		}

		if (best.IsEmpty())
			return "";

		int days = Math.Max(1, RZ_Util.RoundToInt(bestSecs / Math.Max(60, RZ_SeasonClock.DayMinutes() * 60)));
		return string.Format("The longest anyone lasted in one go: %1, %2 days.", best, days);
	}

	// ================================================================ Titles
	//------------------------------------------------------------------------------------------------
	//! What `faction`'s people call this identity this week: its best life this season with them,
	//! and where it's from. "" if they never knew it.
	string Title(string ident, int faction)
	{
		RZ_RepSeason season = RZ_RepSeason.Get();
		string title = "";
		if (season && RZ_Factions.IsValid(faction))
			title = season.TitleFor(ident, faction);

		if (title.IsEmpty())
			return "";

		string from = FromTown(ident);
		if (!from.IsEmpty())
			return string.Format("\"Don't know the face. Know the name. The %1 from %2.\"", title, from);

		return string.Format("\"Don't know the face. Know the name. The %1.\"", title);
	}

	//------------------------------------------------------------------------------------------------
	//! The town whose log holds this identity at the highest salience, "" if none.
	protected string FromTown(string ident)
	{
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		if (!mem)
			return "";

		string best = "";
		int bestSal = 0;
		foreach (int idx, RZ_Settlement s : m_GM.GetSettlements())
		{
			RZ_TownEntry e = mem.TownBest(idx);
			if (e && e.idents.Contains(ident) && e.salience > bestSal)
			{
				bestSal = e.salience;
				best = s.m_sName;
			}
		}

		return best;
	}

	// ================================================================ The beats in full
	//------------------------------------------------------------------------------------------------
	//! After the beats are built (RZ_EpiBeats.Build): the roads, the stations, the characters, one
	//! beat per faction, the places in full, the factors, the hub caps and the notables.
	void OnBeats(RZ_EpiBeats beats)
	{
		RZ_EpilogueSave s = m_Epi.Data();
		if (s.lastDay)
			return;

		Roads(beats);
		Stations(beats);
		Characters(beats);
		Factions(beats);
		Places(s);
		Factors(s);
		HubCaps(s);
		Notables(s);
	}

	//------------------------------------------------------------------------------------------------
	//! The last four lost trucks, as wrecks on the verge.
	protected void Roads(RZ_EpiBeats beats)
	{
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		if (!mem)
			return;

		int made = 0;
		for (int f = 0; f < RZ_Factions.COUNT && made < 4; f++)
		{
			array<RZ_Signal> trucks = {};
			mem.RecentSignals("truck." + f.ToString(), vector.Zero, 1000000, 60, trucks);
			if (trucks.IsEmpty())
				continue;

			RZ_Signal sig = trucks[trucks.Count() - 1];
			vector at = RZ_Util.GroundPos(Vector(sig.x, 0, sig.z));
			string place = m_Epi.PlaceWords(at);
			beats.AddRecord(string.Format("wreck.%1", f), "road", place, at, string.Format("%1 truck on its side %2, still pointing at %3. Somebody has taken the wheels.", RZ_Util.Capitalize(RZ_Factions.GetNameInText(f)), place, sig.text), "There's a wreck on the verge nobody's moved.", f);
			made++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The stations: which towns have lights (Town memory's Powered).
	protected void Stations(RZ_EpiBeats beats)
	{
		RZ_TownMemory tm = m_GM.GetTownMemory();
		if (!tm || !tm.IsEnabled())
			return;

		string lit = "";
		int count = 0;
		vector at = vector.Zero;
		foreach (int idx, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (!tm.HasFlag(idx, RZ_TownMemory.FLAG_POWERED))
				continue;

			if (count < 4)
			{
				if (!lit.IsEmpty())
					lit += ", ";

				lit += s.m_sName;
			}

			if (at == vector.Zero)
				at = s.m_vCenter;

			count++;
		}

		if (count == 0)
			return;

		beats.AddRecord("station", "place", "the lit towns", RZ_Mission.LandPointNear(at, 30, 60), string.Format("A notice on the transformer: LIVE. %1 have lights tonight.", lit), "Some towns have lights again.", -1);
	}

	//------------------------------------------------------------------------------------------------
	//! The characters slot: the Kid and the Preacher (where they are, or that they're gone) and the
	//! leaders who died.
	protected void Characters(RZ_EpiBeats beats)
	{
		RZ_Wanderers w = m_GM.GetWanderers();
		if (w)
		{
			array<int> kids = {};
			w.GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.KID), kids);
			vector pos;
			if (!kids.IsEmpty() && w.GetPosition(kids[0], pos))
				beats.AddRecord("kid", "people", m_Epi.PlaceWords(pos), pos, "A child's chalk on a wall: a house, a road, a queue. Somebody has written NO underneath and somebody else rubbed it out.", "The Kid's been through here.", -1);

			array<int> preachers = {};
			w.GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.PREACHER), preachers);
			if (!preachers.IsEmpty() && w.GetPosition(preachers[0], pos))
				beats.AddRecord("preacher", "people", m_Epi.PlaceWords(pos), pos, "A ring of stones and a bell hung from a branch. The Preacher rests here and reads the dead at dusk.", "There's a bell hung up in a tree.", -1);
		}

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			array<string> dead = {};
			LostLeaders(f, dead);
			RZ_Leader top = RZ_MajorHelp.Top(m_GM, f);
			if (dead.IsEmpty() || !top)
				continue;

			vector at = RZ_Mission.LandPointNear(top.m_vPos, 40, 60);
			beats.AddRecord(string.Format("leaders.%1", f), "people", top.m_sPlace, at, GraveText(f, dead[0]), "Somebody important is buried near here.", f);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A leader's grave in the faction's way.
	protected static string GraveText(int f, string name)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("A grave with a jam jar of flowers on it and a card: %1. The house remembers.", name);
			case RZ_EFaction.MILITARY: return string.Format("A rifle upside down in the earth, a helmet on it, a stencilled board: %1.", name);
			case RZ_EFaction.BANDITS: return string.Format("No grave. A page torn from the book and nailed to a post: %1. PAID.", name);
			case RZ_EFaction.INSTITUTE: return string.Format("A numbered marker, filed: %1. Cause of death recorded.", name);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! One beat a faction: its major's ending, else its season in a line, at its stronghold.
	protected void Factions(RZ_EpiBeats beats)
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			bool has = false;
			foreach (RZ_EpiBeat b : m_Epi.Data().beats)
			{
				if (b.faction == f && b.slot != "since")
					has = true;
			}

			RZ_Leader top = RZ_MajorHelp.Top(m_GM, f);
			if (has || !top)
				continue;

			string line = "";
			if (RZ_Majors.Get())
			{
				foreach (RZ_StoryRecord r : RZ_Majors.Get().GetStories())
				{
					if (r.faction == f && line.IsEmpty())
						line = RZ_Majors.LineFor(r, m_Epi.Data().faction, !m_Epi.Data().lastDay);
				}
			}

			if (line.IsEmpty())
				line = string.Format("%1's season, written up on a board by the gate. Most of it is crossed out and written again.", RZ_Util.Capitalize(RZ_Factions.GetNameInText(f)));

			beats.AddRecord(string.Format("faction.%1", f), "faction", top.m_sPlace, RZ_MajorHelp.NearSeat(top.m_vPos), line, string.Format("%1 has written its season up.", RZ_Util.Capitalize(RZ_Factions.GetNameInText(f))), f);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The places in full, added to the beats' text.
	protected void Places(RZ_EpilogueSave s)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		foreach (RZ_EpiBeat b : s.beats)
		{
			if (b.key.StartsWith("post.") && territory)
			{
				array<int> owners = {};
				Owners(b.subjId, owners);
				b.more = Coats(owners);
				if (owners.Count() > 1)
					b.changed = true;
			}
			else if (b.key.StartsWith("house.") && houses)
			{
				RZ_Safehouse h = houses.Get(b.subjId);
				if (!h)
					continue;

				string more = "";
				if (h.m_iSeasonRescued > 0)
					more += string.Format(" On the board: the ones brought in this season, %1 of them, each with where from.", h.m_iSeasonRescued);

				string rule = RuleNotice(RZ_MajorHelp.World(m_GM, "haven.rule"));
				if (!rule.IsEmpty())
					more += " " + rule;

				if (b.subjId < s.houseTier.Count() && h.GetTier() > s.houseTier[b.subjId])
				{
					more += " Bunting across the yard, made from shirts.";
					b.changed = true;
				}

				b.more = more;
			}
			else if (b.key.StartsWith("town.") && mem)
			{
				int idx = b.key.Substring(5, b.key.Length() - 5).ToInt();
				RZ_TownEntry e = mem.TownBest(idx);
				RZ_TownEntry worst = mem.TownWorst(idx);
				if (worst && (!e || worst.salience > e.salience))
					e = worst;

				if (e)
				{
					b.more = TownMark(e.kind);
					if (e.kind == "burned" || e.kind == "shelled")
						b.changed = true;
				}
			}
			else if (b.key == "table")
			{
				int state = s.tableState;
				if (RZ_WorldFacts.Get().Flag("table.exposed") != 0)
				{
					if (RZ_WorldFacts.Get().Flag("table.boy_out") != 0)
						b.more = " A child's boot by the gate, the other one gone. The boy got out.";
					else if (state == 2)
						b.more = " The house is empty. The table is cleared. They went to the queue.";
					else
						b.more = " The door is chained from outside.";
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! An outpost's coats of paint, the oldest underneath.
	protected static string Coats(notnull array<int> owners)
	{
		if (owners.Count() < 2)
			return "";

		string text = "";
		for (int i = owners.Count() - 1; i >= 0; i--)
		{
			string mark = Paint(owners[i]);
			if (i == owners.Count() - 1)
				text = " On the gate, " + mark;
			else
				text += ", and under that, " + mark;
		}

		return text + ".";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Paint(int f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "the Haven's KEEP OUT in chalk";
			case RZ_EFaction.MILITARY: return "a Bastion stencil";
			case RZ_EFaction.BANDITS: return "a red hand";
			case RZ_EFaction.INSTITUTE: return "Halcyon's numbers";
		}

		return "something scraped off";
	}

	//------------------------------------------------------------------------------------------------
	//! The house rule's door notice (haven.rule).
	protected static string RuleNotice(int rule)
	{
		switch (rule)
		{
			case 1: return "On the door: EVERY NAME. SHARE THE POT.";
			case 2: return "On the door: WORKERS EAT FIRST. THE FAR HOUSE FOR THE REST.";
			case 3: return "On the door, in Bastion's stencil: COUNTED.";
			case 4: return "On the door, small: PAID.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A town's mark by its log's kind.
	protected static string TownMark(string kind)
	{
		switch (kind)
		{
			case "rescue": return " A sheet on two posts at the edge of town: WE GOT THEM OUT.";
			case "shelled": return " A Bastion stencil on the first wall: AREA CLEARED.";
			case "quarantine": return " Tape between two posts across the road, still there.";
			case "burned": return " A cross made of fence palings at the edge of the black.";
			case "mass grave": return " A cross made of fence palings, and a lot of turned earth.";
			case "puzzle": return " A note in a jar on a post: the riddle, and somebody's answer under it.";
			case "taken back": return " A candle in a jar on a doorstep.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The change factor (x1.5) and the choice factor (x1.25).
	protected void Factors(RZ_EpilogueSave s)
	{
		foreach (RZ_EpiBeat b : s.beats)
		{
			if (b.changed)
				b.weight = b.weight * 1.5;

			if (!b.idents.IsEmpty() || b.key.StartsWith("story."))
				b.weight = b.weight * 1.25;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Hubs (the lab, the queue, each safehouse) take up to 3 beats within 60 m; strongholds 2. The
	//! weakest go.
	protected void HubCaps(RZ_EpilogueSave s)
	{
		foreach (RZ_EpiBeat hub : s.beats)
		{
			if (hub.slot != "hub" && hub.slot != "cure")
				continue;

			int cap = 3;
			if (hub.key == "roster" || hub.key == "crossroads" || hub.key == "supper")
				cap = 2;

			array<RZ_EpiBeat> near = {};
			foreach (RZ_EpiBeat b : s.beats)
			{
				if (!b.dropped && RZ_Util.DistXZ(Vector(b.x, 0, b.z), Vector(hub.x, 0, hub.z)) <= 60)
					near.Insert(b);
			}

			while (near.Count() > cap)
			{
				int weakest = 0;
				for (int i = 1; i < near.Count(); i++)
				{
					if (near[i].weight < near[weakest].weight)
						weakest = i;
				}

				if (near[weakest].slot == "cure")
					break;

				near[weakest].dropped = true;
				near.Remove(weakest);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The notables: up to 12 named in the world, by score; each in at most 3 beats (2 on a server
	//! with fewer than 6), at least half the beats naming nobody; on a server with fewer than 24
	//! identities with deeds, at most half of them named. Everyone else's name comes off the beats
	//! (lists, the stand board, keep theirs).
	protected void Notables(RZ_EpilogueSave s)
	{
		array<string> idents = {};
		array<int> scores = {};
		foreach (string ident : s.playerIdents)
		{
			int score = Score(ident);
			if (score <= 0)
				continue;

			idents.Insert(ident);
			scores.Insert(score);
		}

		int cap = Math.Min(Math.Min(NOTABLES, m_Epi.GetSettings().m_iNotables), idents.Count());
		if (idents.Count() < 24)
			cap = Math.Min(cap, idents.Count() / 2);

		array<string> notable = {};
		while (notable.Count() < cap && !idents.IsEmpty())
		{
			int pick = 0;
			for (int i = 1; i < idents.Count(); i++)
			{
				if (scores[i] > scores[pick])
					pick = i;
			}

			notable.Insert(idents[pick]);
			idents.Remove(pick);
			scores.Remove(pick);
		}

		int perName = 3;
		if (s.playerIdents.Count() < 6)
			perName = 2;

		int named = 0;
		int total = 0;
		map<string, int> used = new map<string, int>();
		foreach (RZ_EpiBeat b : s.beats)
		{
			if (b.dropped)
				continue;

			total++;
			if (b.key == "stand" || b.idents.IsEmpty())
				continue;

			for (int k = b.idents.Count() - 1; k >= 0; k--)
			{
				string who = b.idents[k];
				int count = 0;
				used.Find(who, count);
				bool keep = notable.Contains(who) && count < perName;
				if (!keep && b.key.StartsWith("wanted."))
					keep = true;

				if (!keep)
				{
					b.idents.Remove(k);
					if (k < b.names.Count())
						b.names.Remove(k);

					continue;
				}

				used.Set(who, count + 1);
			}

			if (!b.idents.IsEmpty())
				named++;
		}

		// At least half the beats name nobody: the weakest named ones lose their names
		while (named * 2 > total && named > 0)
		{
			RZ_EpiBeat weakest;
			foreach (RZ_EpiBeat w : s.beats)
			{
				if (w.dropped || w.idents.IsEmpty() || w.key == "stand" || w.key.StartsWith("wanted."))
					continue;

				if (!weakest || w.weight < weakest.weight)
					weakest = w;
			}

			if (!weakest)
				break;

			weakest.idents.Clear();
			weakest.names.Clear();
			named--;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A notable's score: decided the route +5, a major +4 each (8 at most), held the stand +3,
	//! Champion +3 a faction (6 at most), Wanted +2, a questline +1 each (3 at most), a crew name +1.
	protected int Score(string ident)
	{
		int score = 0;
		RZ_EpilogueSave s = m_Epi.Data();
		if (s.deciderIdents.Contains(ident))
			score += 5;

		int majors = 0;
		if (RZ_Majors.Get())
		{
			foreach (RZ_StoryRecord r : RZ_Majors.Get().GetStories())
			{
				if (r.deciders.Contains(ident))
					majors += 4;
			}
		}

		score += Math.Min(8, majors);
		if (s.standNames.Contains(KnownNameOf(ident)))
			score += 3;

		if (s.wantIdents.Contains(ident))
			score += 2;

		int pi = m_GM.FindPlayerByIdentity(ident);
		if (pi > 0)
		{
			RZ_PlayerProfile p = m_GM.GetProfile(pi);
			int champs = 0;
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (p.GetTier(f) == RZ_ETier.CHAMPION)
					champs += 3;
			}

			score += Math.Min(6, champs);
			if (!RZ_RepVoice.CrewName(p).IsEmpty())
				score++;
		}

		return score;
	}

	//------------------------------------------------------------------------------------------------
	protected string KnownNameOf(string ident)
	{
		int i = m_Epi.Data().playerIdents.Find(ident);
		if (i < 0)
			return "";

		return m_Epi.Data().playerNames[i];
	}
}
