//! Traces (docs/TRACES.md): the world's environmental storytelling. The Zone already remembers a
//! lot as data; Traces makes some of it visible, near players only, and never leaves anything
//! behind. Built: the door marks (their grammar, the first days, live writes, reading by level, the
//! town summary, the journal's Read on doors, errands' old mark), the doors you can see (decals or
//! boards, text always) and the player traces. Planned (designs/TRACES.md): the first days' scenes
//! and the line, vignettes and live traces, sound and light, the epilogue's handover.
//!
//! Data first, props when set, text always. Nothing lootable, nothing blocks, nothing pays.
//! Owned by RZ_GameModeComponent (World > Traces), reached as gm.GetTraces(). Server only, but for
//! the settings and the kit, which clients read for the decals.

//------------------------------------------------------------------------------------------------
//! How door marks look (decision 1): decals drawn on clients, boards beside the door, or text only.
enum RZ_EDoorDrawing
{
	DECALS,
	BOARDS,
	TEXT
}

//------------------------------------------------------------------------------------------------
//! Every optional prefab and material Traces uses: one attribute each, empty = text only.
[BaseContainerProps()]
class RZ_TraceKit
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Door marks atlas (optional): a decal material whose image is a 4 x 4 atlas, in order: X, slash, post code, C, ATT, CLEARED, bear, patched bear, candle, hatching, ring and dot, arrow, drop, red hand, tally, beak (docs/TRACES.md, Doors you can see). Empty = no decals", "emat")]
	ResourceName m_sDoorAtlas;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Door marks overlays (optional): a second decal atlas: ring, tick, strike, PAID, scratches outside, scratches inside, smears, thumbprint, grey overpaint. Empty = the overlays aren't drawn", "emat")]
	ResourceName m_sDoorOverlays;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Door board, Bastion (optional, the board path): a plywood offcut with the orange X, leaned beside the door. Without a board for the author, that door is text only", "et")]
	ResourceName m_sBoardBastion;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Door board, the Haven (optional, the board path): a cardboard sheet with the bear in white", "et")]
	ResourceName m_sBoardHaven;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Door board, the Red Hand (optional, the board path): a plank with a red hand", "et")]
	ResourceName m_sBoardRedHand;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Door board, Halcyon (optional, the board path): a square of white primer with the beak stencilled", "et")]
	ResourceName m_sBoardHalcyon;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Door board, residents (optional, the board path): a sheet of paper on the door (Notice_A4_Vertical_V1.et will do). Also used for the infected's scratches", "et")]
	ResourceName m_sBoardResidents;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Grave (optional): two fence palings and a mound (P3). Empty = the grave is its approach line only", "et")]
	ResourceName m_sGrave;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pyre (optional): a black patch, a burnt pallet and tyre, a post (P2). Empty = text only", "et")]
	ResourceName m_sPyre;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Candle jars (optional): 3-5 jam jars with candles; the lit twin with its light on LIT_ON_SPAWN (P3b). Empty = text only", "et")]
	ResourceName m_sCandleJars;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Season mark, rescue (optional): a sheet on two posts (P5). Empty = Shared > Season mark prefab", "et")]
	ResourceName m_sSeasonSheet;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Season mark, burned and other jobs (optional): a board (P5). Empty = Shared > Season mark prefab", "et")]
	ResourceName m_sSeasonBoard;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Season mark, taken (optional): a stencilled board (P5). Empty = Shared > Season mark prefab", "et")]
	ResourceName m_sSeasonStencil;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Season mark, sealed (optional): tape between two posts (P5). Empty = Shared > Season mark prefab", "et")]
	ResourceName m_sSeasonTape;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Season mark, emptied (optional): flowers on a post (P5). Empty = Shared > Season mark prefab", "et")]
	ResourceName m_sSeasonFlowers;
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Traces. Made with RZ_Util.NewWithDefaults.
[BaseContainerProps()]
class RZ_TraceSettings
{
	[Attribute("1", desc: "Traces: the world's environmental storytelling. Door marks written by the people who were there, read by standing at a door; player traces. Data first, props when set, text always; nothing pays (docs/TRACES.md)")]
	bool m_bEnabled;

	[Attribute("1", desc: "Door marks: chalk, paint, stencils and scratches beside a few doors in each town, from the first days and from what happens this season")]
	bool m_bDoorMarks;

	[Attribute("6", UIWidgets.Slider, "Doors a town: marked doors at most in a town (two more in a stronghold's town)", "1 10 1")]
	int m_iDoorsPerTown;

	[Attribute("3", UIWidgets.Slider, "Layers a door: marks on one door at most; a new one pushes the most weathered off", "1 3 1")]
	int m_iLayersPerDoor;

	[Attribute("20", UIWidgets.Slider, "Live writes a season day on the server (2 a town at most)", "0 60 1")]
	int m_iLiveWritesPerDay;

	[Attribute("1", desc: "First days: Days 1-10 seeded from the season's seed and the canon spots, the same after every restart, never saved")]
	bool m_bFirstDays;

	[Attribute("1", desc: "Player traces: ticked bears in a town you emptied, ash where you burned the bodies, a grave, the memorial candles, a candle near where you died, the season mark's look by kind")]
	bool m_bPlayerTraces;

	[Attribute("0", UIWidgets.ComboBox, "Door drawing: Decals (drawn on each client from the door atlas; boards if it's empty), Boards (a board beside each marked door), Text (the words only)", "", ParamEnumArray.FromEnum(RZ_EDoorDrawing))]
	RZ_EDoorDrawing m_eDrawing;

	[Attribute("40", UIWidgets.Slider, "Props on the server: Traces' scene props at once (graves, ash, candles)", "0 100 1")]
	int m_iProps;

	[Attribute("12", UIWidgets.Slider, "Props a cluster: Traces' scene props within 150 m of each other", "1 40 1")]
	int m_iPropsCluster;

	[Attribute("0", desc: "Debug: log every door read and the drawn list's size")]
	bool m_bDebug;

	[Attribute(desc: "Prefabs and materials (all optional; empty = text only)")]
	ref RZ_TraceKit m_Kit;

	//------------------------------------------------------------------------------------------------
	//! The game mode's settings (clients too: the attribute values are on the prefab), else defaults.
	static RZ_TraceSettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetTraceSettings())
			return gm.GetTraceSettings();

		return RZ_TraceSettings.Cast(RZ_Util.NewWithDefaults("RZ_TraceSettings"));
	}

	//------------------------------------------------------------------------------------------------
	RZ_TraceKit Kit()
	{
		if (!m_Kit)
			m_Kit = RZ_TraceKit.Cast(RZ_Util.NewWithDefaults("RZ_TraceKit"));

		return m_Kit;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_TracesSave
{
	ref array<ref RZ_TownHistorySave> history = {};
	ref array<ref RZ_MarkedDoorSave> doors = {};
	ref array<ref RZ_PlayerTrace> players = {};
	int nextKey;
	float rainDays;
}

//------------------------------------------------------------------------------------------------
class RZ_Traces
{
	protected static const float SLOT_SECONDS = 5;
	protected static const float WATCH_SECONDS = 30;
	protected static const float PRUNE_SECONDS = 60;
	protected static const float READ_RADIUS = 5;
	protected static const int READ_SECONDS = 2;
	protected static const int JOURNAL_LINES = 12;
	static const string TOWN_LANE = "traces.town:";

	protected RZ_GameModeComponent m_GM;
	protected RZ_TraceSettings m_Settings;
	protected ref RZ_TownHistory m_History = new RZ_TownHistory();
	protected ref RZ_DoorMarks m_Doors;
	protected ref RZ_DoorSlots m_Slots;
	protected ref RZ_FirstDays m_FirstDays;
	protected ref RZ_TraceWatch m_Watch;
	protected ref RZ_PlayerTraces m_Players;
	protected ref RZ_TraceDraw m_Draw;
	protected ref RZ_TracesSave m_Restored;
	protected float m_fSlotTimer;
	protected float m_fWatchTimer;
	protected float m_fPruneTimer;
	protected float m_fRainDays;
	protected bool m_bReady;

	// Reading (runtime: a session)
	protected ref set<string> m_aRead = new set<string>();				// "player|door"
	protected ref set<string> m_aTownsSeen = new set<string>();			// "player|town"
	protected ref map<int, string> m_mAt = new map<int, string>();		// player -> the door they stand at
	protected ref map<int, int> m_mAtSeconds = new map<int, int>();
	protected ref map<int, ref array<string>> m_mJournal = new map<int, ref array<string>>();	// lost with the life

	//------------------------------------------------------------------------------------------------
	void RZ_Traces(RZ_GameModeComponent gm, RZ_TraceSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Doors = new RZ_DoorMarks(gm, this);
		m_Slots = new RZ_DoorSlots(gm);
		m_FirstDays = new RZ_FirstDays(gm, m_Doors);
		m_Watch = new RZ_TraceWatch(gm, this);
		m_Players = new RZ_PlayerTraces(gm, this);
		m_Draw = new RZ_TraceDraw(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	//! The server's Traces, null without a game mode or when off.
	static RZ_Traces Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return null;

		return gm.GetTraces();
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn() { return m_Settings.m_bEnabled && m_bReady; }
	bool DoorMarksOn() { return m_Settings.m_bEnabled && m_Settings.m_bDoorMarks; }
	RZ_TraceSettings GetSettings() { return m_Settings; }
	RZ_TraceKit GetKit() { return m_Settings.Kit(); }
	RZ_DoorMarks GetDoors() { return m_Doors; }
	RZ_DoorSlots GetSlots() { return m_Slots; }
	RZ_TownHistory History() { return m_History; }
	RZ_PlayerTraces GetPlayers() { return m_Players; }
	RZ_TraceDraw GetDraw() { return m_Draw; }

	//------------------------------------------------------------------------------------------------
	//! Season days of rain so far (0.3 or more): chalk ages twice as fast while it lasts.
	float RainDays()
	{
		return m_fRainDays;
	}

	//------------------------------------------------------------------------------------------------
	//! Marked doors a town may have: Doors a town, two more in a stronghold's town.
	int DoorsPerTown(int town)
	{
		int cap = Math.Max(1, m_Settings.m_iDoorsPerTown);
		if (RZ_WorldFacts.Get().TownOwner(town).m_eKind == RZ_ETownKind.STRONGHOLD)
			cap += 2;

		return cap;
	}

	// ================================================================ Save / setup
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_TracesSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	//! After every system it reads is set up: live marks back from the save, then the first days
	//! seeded under them (the same as last time), then the player traces back into the spawner.
	void Setup()
	{
		if (!m_Settings.m_bEnabled)
		{
			RZ_Log.Info("Traces: off.");
			return;
		}

		if (m_Restored)
		{
			m_History.Restore(m_Restored.history);
			m_Doors.Restore(m_Restored.doors, m_Restored.nextKey);
			m_fRainDays = m_Restored.rainDays;
		}

		m_bReady = true;
		if (m_Settings.m_bDoorMarks && m_Settings.m_bFirstDays)
			m_FirstDays.Seed();

		if (m_Restored)
			m_Players.Restore(m_Restored.players);

		m_Restored = null;
		RZ_Log.Info(string.Format("Traces: %1 marked doors (%2 live layers), %3 town events, drawing %4.", m_Doors.GetDoors().Count(), m_Doors.LiveCount(), m_History.Count(), DrawingName(m_Draw.Mode())));
	}

	//------------------------------------------------------------------------------------------------
	RZ_TracesSave Save()
	{
		RZ_TracesSave data = new RZ_TracesSave();
		m_History.Save(data.history);
		m_Doors.Save(data.doors);
		m_Players.Save(data.players);
		data.nextKey = m_Doors.NextKey();
		data.rainDays = m_fRainDays;
		return data;
	}

	//------------------------------------------------------------------------------------------------
	protected static string DrawingName(int mode)
	{
		if (mode == RZ_EDoorDrawing.DECALS)
			return "decals";

		if (mode == RZ_EDoorDrawing.BOARDS)
			return "boards";

		return "text";
	}

	// ================================================================ Ticks
	//------------------------------------------------------------------------------------------------
	//! The server tick: one town's doors resolved near players (5 s), the drawn list, the watch
	//! (30 s), and gone marks (60 s).
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		if (!ids.IsEmpty() && RZ_Weather.Rain() >= 0.3)
			m_fRainDays += dt / Math.Max(60, RZ_SeasonClock.DayMinutes() * 60);

		m_fSlotTimer += dt;
		if (m_fSlotTimer >= SLOT_SECONDS)
		{
			m_fSlotTimer = 0;
			int town = m_Slots.Step(players);
			if (town >= 0)
				m_Doors.Bind(town);

			m_Draw.Tick(players);
		}

		m_fWatchTimer += dt;
		if (m_fWatchTimer >= WATCH_SECONDS)
		{
			m_fWatchTimer = 0;
			m_Watch.Look();
		}

		m_fPruneTimer += dt;
		if (m_fPruneTimer >= PRUNE_SECONDS)
		{
			m_fPruneTimer = 0;
			m_Doors.Prune();
			m_Players.Prune();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second: the town's summary on entry, and a door read after 2 seconds standing at it.
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		if (!IsOn() || !m_Settings.m_bDoorMarks)
			return;

		foreach (int i, IEntity p : players)
		{
			if (!p || RZ_Util.IsInVehicle(p))
				continue;

			int pid = ids[i];
			vector pos = p.GetOrigin();
			int town = m_GM.FindNearestSettlement(pos);
			RZ_Settlement s = m_GM.GetSettlement(town);
			if (!s || RZ_Util.DistXZ(s.m_vCenter, pos) > s.m_fRadius + 30)
			{
				m_mAt.Remove(pid);
				continue;
			}

			array<RZ_MarkedDoor> doors = {};
			m_Doors.TownDoors(town, doors);
			Summary(pid, town, s, doors);
			StandAt(pid, pos, doors);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The first time this session a character enters a town with two or more marked doors.
	protected void Summary(int pid, int town, RZ_Settlement s, array<RZ_MarkedDoor> doors)
	{
		if (doors.Count() < 2)
			return;

		string key = string.Format("%1|%2", pid, town);
		if (m_aTownsSeen.Contains(key))
			return;

		m_aTownsSeen.Insert(key);
		RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
		if (lanes && !lanes.TryAmbient(pid, TOWN_LANE + town.ToString()))
			return;

		string line = RZ_TraceText.TownSummary(s.m_sName, doors);
		if (!line.IsEmpty())
			m_GM.NotifyPlayerAs(pid, "Doors", line, RZ_ENoticeTag.LOW);
	}

	//------------------------------------------------------------------------------------------------
	//! Within 5 m of a marked door for 2 seconds: its line, once per door per session.
	protected void StandAt(int pid, vector pos, array<RZ_MarkedDoor> doors)
	{
		RZ_MarkedDoor at = null;
		float best = READ_RADIUS;
		foreach (RZ_MarkedDoor d : doors)
		{
			if (!d.IsBound() || Math.AbsFloat(d.m_vPos[1] - pos[1]) > 4)
				continue;

			float dist = RZ_Util.DistXZ(d.m_vPos, pos);
			if (dist <= best)
			{
				best = dist;
				at = d;
			}
		}

		if (!at)
		{
			m_mAt.Remove(pid);
			return;
		}

		string was;
		int seconds = 0;
		if (m_mAt.Find(pid, was) && was == at.m_sKey)
			m_mAtSeconds.Find(pid, seconds);

		seconds++;
		m_mAt.Set(pid, at.m_sKey);
		m_mAtSeconds.Set(pid, seconds);
		if (seconds < READ_SECONDS)
			return;

		string key = string.Format("%1|%2", pid, at.m_sKey);
		if (m_aRead.Contains(key))
			return;

		m_aRead.Insert(key);
		ReadDoor(pid, at);
	}

	//------------------------------------------------------------------------------------------------
	//! The door's words at this reader's level, newest first; three layers open in the reader
	//! with UI > Reader on. Into the journal's Read on doors.
	void ReadDoor(int pid, RZ_MarkedDoor d)
	{
		string text = DoorLine(pid, d, "\n");
		if (text.IsEmpty())
			return;

		if (d.m_aLayers.Count() >= 3 && RZ_UISettings.Get().m_bReader)
			RZ_Reader.Show(m_GM, pid, "On a door", "Three hands, one over another", text);
		else
			m_GM.NotifyPlayerAs(pid, "On a door", DoorLine(pid, d, " "), RZ_ENoticeTag.LOW);

		AddJournal(pid, d.m_iTown, DoorLine(pid, d, " "));
		if (m_Settings.m_bDebug)
			RZ_Log.Info(string.Format("Traces: player %1 read %2#%3", pid, m_Doors.TownName(d.m_iTown), d.m_iSlot));
	}

	//------------------------------------------------------------------------------------------------
	//! Every layer, newest first, joined by `sep`, at the reader's level; the Hunter adds the age.
	string DoorLine(int pid, RZ_MarkedDoor d, string sep)
	{
		RZ_TraceReader r = RZ_TraceReader.For(m_GM, pid);
		string text = "";
		for (int i = d.m_aLayers.Count() - 1; i >= 0; i--)
		{
			RZ_DoorMark m = d.m_aLayers[i];
			float age = m_Doors.AgeDays(m);
			string line = RZ_TraceText.DoorLayer(m, r.Level(m.m_iAuthor), r, d, age, m_Doors.Stage(m));
			if (line.IsEmpty())
				continue;

			if (i == d.m_aLayers.Count() - 1 && r.Is("hunter"))
			{
				string written = RZ_TraceText.WrittenAge(m, age);
				if (!written.IsEmpty())
					line += " " + written;
			}

			if (text.IsEmpty())
				text = line;
			else
				text += sep + line;
		}

		return text;
	}

	// ================================================================ The journal
	//------------------------------------------------------------------------------------------------
	protected void AddJournal(int pid, int town, string text)
	{
		array<string> lines;
		if (!m_mJournal.Find(pid, lines))
		{
			lines = {};
			m_mJournal.Set(pid, lines);
		}

		lines.Insert(string.Format("%1, Day %2: %3", m_Doors.TownName(town), RZ_WorldFacts.Get().Day(), text));
		while (lines.Count() > JOURNAL_LINES)
		{
			lines.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The last 12 door lines this character read, newest first (the journal's Read on doors).
	void JournalFor(int pid, notnull array<string> result)
	{
		array<string> lines;
		if (!m_mJournal.Find(pid, lines))
			return;

		for (int i = lines.Count() - 1; i >= 0; i--)
		{
			result.Insert(lines[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A character died (RZ_TraceWriters.Death): the doors they read go with the life; their grave.
	void OnDeath(int pid, vector pos)
	{
		m_mJournal.Remove(pid);
		m_mAt.Remove(pid);
		m_Players.OnDeath(pid, pos);
	}

	// ================================================================ Drawing
	//------------------------------------------------------------------------------------------------
	void DoorChanged(RZ_MarkedDoor d)
	{
		m_Draw.DoorChanged(d);
	}

	//------------------------------------------------------------------------------------------------
	void DoorRemoved(RZ_MarkedDoor d)
	{
		m_Draw.DoorRemoved(d);
	}

	// ================================================================ Errands and the season marks
	//------------------------------------------------------------------------------------------------
	//! Errands' found hook "an old mark" (decision 15): the key of a mark near `at`, written in the
	//! grammar if there's none. "" with Traces off or no town near.
	static string OldMarkKey(vector at)
	{
		RZ_Traces t = Get();
		if (!t || !t.IsOn() || !t.DoorMarksOn())
			return "";

		return t.GetDoors().OldMarkKey(at);
	}

	//------------------------------------------------------------------------------------------------
	//! The old mark's words, at this reader's level. "" if it's gone.
	static string OldMarkLine(string key, int playerId)
	{
		RZ_Traces t = Get();
		if (!t || key.IsEmpty())
			return "";

		RZ_MarkedDoor d = t.GetDoors().FindByKey(key);
		if (!d)
			return "";

		string line = t.DoorLine(playerId, d, " ");
		if (!line.IsEmpty())
			t.AddJournal(playerId, d.m_iTown, line);

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! P5: the season mark's prop by its kind, else `fallback` (Shared > Season mark prefab).
	static ResourceName SeasonMarkPrefab(string kind, ResourceName fallback)
	{
		RZ_Traces t = Get();
		if (!t || !t.m_Settings.m_bEnabled || !t.m_Settings.m_bPlayerTraces)
			return fallback;

		RZ_TraceKit kit = t.GetKit();
		ResourceName look = kit.m_sSeasonBoard;
		switch (kind)
		{
			case "rescue": look = kit.m_sSeasonSheet; break;
			case "taken": look = kit.m_sSeasonStencil; break;
			case "emptied": look = kit.m_sSeasonFlowers; break;
			case "sealed": look = kit.m_sSeasonTape; break;
		}

		if (look.IsEmpty())
			return fallback;

		return look;
	}

	//------------------------------------------------------------------------------------------------
	//! P5: the season mark's words by its kind: what you see, then the line.
	static string SeasonMarkText(string kind, string line)
	{
		RZ_Traces t = Get();
		if (!t || !t.m_Settings.m_bEnabled || !t.m_Settings.m_bPlayerTraces)
			return line;

		return RZ_TraceText.SeasonMark(kind, line);
	}

	// ================================================================ Admin and status
	//------------------------------------------------------------------------------------------------
	//! "Traces: doors N (drawn M), scenes S, props P/40, lights L, loops O"
	string StatusText()
	{
		RZ_Marks marks = RZ_Marks.Get();
		int props = 0;
		if (marks)
			props = marks.Count(RZ_EMarkPool.TRACES);

		return string.Format("Traces: doors %1 (drawn %2), scenes %3, props %4/%5, lights 0, loops 0", m_Doors.GetDoors().Count(), m_Draw.DrawnCount(), m_Players.Count(), props, m_Settings.m_iProps);
	}

	//------------------------------------------------------------------------------------------------
	//! /traces (admins): every marked door within 300 m with its source; "town <name>": the town's
	//! history and doors; "write <symbol>" (Debug on): a mark at the nearest door; "clear <town>".
	//! The list goes to the server log; the admin gets a short notice.
	void Command(int playerId, string text)
	{
		if (!RZ_NoticeBoard.IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Not allowed", "Only a server admin can use /traces.");
			return;
		}

		if (!IsOn())
		{
			m_GM.NotifyPlayer(playerId, "Traces", "Traces is off (World > Traces).");
			return;
		}

		string arg = text.Trim();
		array<string> parts = {};
		arg.Split(" ", parts, true);
		string word = "";
		if (!parts.IsEmpty())
		{
			word = parts[0];
			word.ToLower();
		}

		string rest = "";
		if (parts.Count() > 1)
			rest = arg.Substring(parts[0].Length(), arg.Length() - parts[0].Length()).Trim();

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (word == "town")
		{
			int idx = FindTown(rest);
			if (idx < 0)
			{
				m_GM.NotifyPlayer(playerId, "Traces", "No town by that name.");
				return;
			}

			int n = LogTown(idx, playerId);
			m_GM.NotifyPlayer(playerId, "Traces", string.Format("%1: %2 marked doors. The list is in the server log.", m_Doors.TownName(idx), n));
			return;
		}

		if (word == "clear")
		{
			int ct = FindTown(rest);
			if (ct < 0)
			{
				m_GM.NotifyPlayer(playerId, "Traces", "No town by that name.");
				return;
			}

			int cleared = m_Doors.ClearTown(ct);
			m_History.Clear(ct);
			m_GM.NotifyPlayer(playerId, "Traces", string.Format("%1: %2 doors cleared. First-days marks come back at the next start.", m_Doors.TownName(ct), cleared));
			return;
		}

		if (word == "write" && me)
		{
			WriteHere(playerId, me.GetOrigin(), rest);
			return;
		}

		if (!me)
			return;

		int count = 0;
		foreach (RZ_MarkedDoor d : m_Doors.GetDoors())
		{
			if (!d.IsBound() || RZ_Util.DistXZ(d.m_vPos, me.GetOrigin()) > 300)
				continue;

			LogDoor(d, playerId);
			count++;
		}

		m_GM.NotifyPlayer(playerId, "Traces", string.Format("%1 marked doors within 300 m. The list is in the server log. %2", count, StatusText()));
	}

	//------------------------------------------------------------------------------------------------
	protected int FindTown(string name)
	{
		string want = name;
		want.ToLower();
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			string have = s.m_sName;
			have.ToLower();
			if (have == want)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected int LogTown(int idx, int playerId)
	{
		array<ref RZ_TownEvent> events = m_History.Get(idx);
		if (events)
		{
			foreach (RZ_TownEvent e : events)
			{
				RZ_Log.Info(string.Format("Traces: %1 history: Day %2 %3 (%4) x%5", m_Doors.TownName(idx), e.day, e.kind, RZ_Factions.GetName(e.faction), e.count));
			}
		}

		array<RZ_MarkedDoor> doors = {};
		m_Doors.TownDoors(idx, doors);
		foreach (RZ_MarkedDoor d : doors)
		{
			LogDoor(d, playerId);
		}

		return doors.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void LogDoor(RZ_MarkedDoor d, int playerId)
	{
		string layers = "";
		foreach (RZ_DoorMark m : d.m_aLayers)
		{
			string source = "live";
			if (m.IsSeeded())
				source = "first days";
			else if (m.Has(RZ_DoorMark.F_TEMP))
				source = "old mark";

			layers += string.Format(" [%1 %2 D%3 %4]", RZ_DoorMarks.AuthorKey(m.m_iAuthor), RZ_TraceText.SymbolKey(m.m_iSymbol), m.m_iDay, source);
		}

		RZ_Log.Info(string.Format("Traces: %1#%2 %3 at %4:%5 | %6", m_Doors.TownName(d.m_iTown), d.m_iSlot, d.m_sKey, RZ_SettlementScanner.GridReference(d.m_vPos), layers, DoorLine(playerId, d, " ")));
	}

	//------------------------------------------------------------------------------------------------
	//! Debug only: a mark of `symbol` at the nearest door of the town you're in.
	protected void WriteHere(int playerId, vector at, string symbol)
	{
		if (!m_Settings.m_bDebug)
		{
			m_GM.NotifyPlayer(playerId, "Traces", "Turn World > Traces > Debug on to write marks by hand.");
			return;
		}

		string key = symbol;
		key.ToLower();
		int sym = RZ_TraceText.SymbolFromKey(key);
		int town = RZ_TraceWriters.TownAt(at);
		if (sym < 0 || town < 0)
		{
			m_GM.NotifyPlayer(playerId, "Traces", "Stand in a town and name a symbol: x, slash, postcode, c, att, cleared, bear, patched_bear, candle, hatch, ring_dot, arrow, drop, hand, tally, beak, scratch_out, scratch_in, smears, words.");
			return;
		}

		if (!m_Slots.IsResolved(town))
		{
			m_Slots.Resolve(town);
			m_Doors.Bind(town);
		}

		RZ_DoorMark m = RZ_DoorMark.Make(sym, RZ_WorldFacts.Get().Day(), 3, 1, RZ_TraceText.PackFound(1, 1, 1, 2));
		m.m_iFlags = RZ_DoorMark.F_TEMP;
		m.m_iUntil = RZ_SeasonClock.Now() + 3600;
		RZ_MarkedDoor d = m_Doors.Write(town, RZ_EDoorWant.NEAR, at, m, "an admin");
		if (d)
			m_GM.NotifyPlayer(playerId, "Traces", string.Format("Written on %1#%2 for an hour.", m_Doors.TownName(town), d.m_iSlot));
		else
			m_GM.NotifyPlayer(playerId, "Traces", "Nothing written: the town is full or doesn't take marks.");
	}
}
