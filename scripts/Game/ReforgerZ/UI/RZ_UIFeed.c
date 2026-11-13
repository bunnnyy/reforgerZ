//! What each player's UI needs, worked out on the server and sent only to them (docs/UI.md,
//! "Data feeds"). Owned by RZ_GameModeComponent and run on its tick, after the infection.
//!
//! - Pushed on change: the infection timer (plus a resync every minute), the secure channel the
//!   player is listening to, a noise level 0-6 from the heat where they stand, and (with Stealth
//!   on) how visible they are, 0-5, from stealth's own update (RZ_Stealth, every second).
//! - Asked for by a menu as it opens: standing, the journal, the backgrounds, and by the map's
//!   icon layer: what this player knows the place of (one string each, one record per line,
//!   fields split by '|'; see BuildPayload).
//! - The gear each background loadout really spawns with (seen at spawns), for the spawn screen.
//! - Clues read in puzzle missions are remembered here per player, while the mission runs.
//! - The notebook item (UI > Notebook prefab), given to every new life a few seconds after spawn.
//!
//! Nothing here changes the game. If RZ_PlayerUIComponent isn't on the player controller, every
//! send is skipped.

//------------------------------------------------------------------------------------------------
class RZ_UIFeedState
{
	int m_iInf = -1;
	bool m_bPaused;
	float m_fSinceSync;
	float m_fLeftSent;		// what the client was last told (it counts down from these)
	float m_fPauseSent;
	int m_iListen = -2;
	int m_iSeen = -1;		// visibility level last sent (-1: none yet, the client hides it)
	bool m_bSeenHidden;
	bool m_bSeenNoisy;
	string m_sLook;			// the look-coming line last sent ("" hidden)
}

//------------------------------------------------------------------------------------------------
class RZ_UIClue
{
	int m_iMission;
	string m_sTitle;
	string m_sText;
}

//------------------------------------------------------------------------------------------------
//! One payload record: a tag, then fields split by '|' (RZ_UIText.Clean keeps them safe).
class RZ_UILine
{
	protected string m_sText;

	//------------------------------------------------------------------------------------------------
	void RZ_UILine(string tag)
	{
		m_sText = tag;
	}

	//------------------------------------------------------------------------------------------------
	RZ_UILine Add(string value)
	{
		m_sText += RZ_UIText.SEP + RZ_UIText.Clean(value);
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_UILine AddInt(int value)
	{
		m_sText += RZ_UIText.SEP + value.ToString();
		return this;
	}

	//------------------------------------------------------------------------------------------------
	string End()
	{
		return m_sText + "\n";
	}
}

//------------------------------------------------------------------------------------------------
class RZ_UIFeed
{
	static const int PAYLOAD_STANDING = 0;
	static const int PAYLOAD_JOURNAL = 1;
	static const int PAYLOAD_BACKGROUNDS = 2;
	static const int PAYLOAD_MAP = 3;
	static const float MAP_SAME_PLACE = 250;	// two of the map's places this close (m) are one place, one marker
	static const int PAYLOAD_BOARD = 4;		// the notice board the player stands at (World/RZ_NoticeBoard.c)
	static const int PAYLOAD_HALL = 5;		// the hall of fame (Persistence/RZ_HallOfFame.c)
	static const int PAYLOAD_KINDS = 6;		// the kinds a client may ask for; the ones below are only sent
	static const int PAYLOAD_TALK = 6;		// the talk screen: one node, or X|reason to close (Dialogue/RZ_Dialogue.c)
	static const int PAYLOAD_READER = 7;	// the reader: P|title|how, then the text; "" closes it (UI/RZ_Reader.c)
	static const int PAYLOAD_CAR = 8;		// the car repair screen: C, I, W, J records; F falls back; X|reason closes (Interactions/RZ_Cars.c)
	static const int PAYLOAD_SHOP = 9;		// the shop screen: S and I records; X|reason closes (Factions/RZ_ShopScreen.c)
	static const int PAYLOAD_DETAIL = 10;	// one journal row's detail, asked when it's selected (BuildDetail)
	static const int PAYLOAD_SEASON = 11;	// the notebook's season tab, asked when it opens (BuildSeason)
	static const int PAYLOAD_CRAFT = 12;	// the crafting screen: B, W, R, N records; F falls back; X|reason closes (Interactions/RZ_Crafting.c)
	static const int PAYLOAD_TRACKER = 13;	// the HUD's quest tracker: T and O records, the tracked job only; "" hides it (UI/RZ_Quests.c)
	static const int PAYLOAD_OBJECT = 14;	// the object menu: O and R records; F falls back; X|reason closes (Interactions/RZ_ObjectScreen.c)
	static const int PAYLOAD_COOK = 15;		// the cooking screen at a stove: the crafting screen's records (Interactions/RZ_Crafting.c)
	static const int PAYLOAD_RADIO = 16;	// the radio screen: V and C records; F falls back; X|reason closes (Interactions/RZ_RadioScreen.c)
	static const int PAYLOAD_NAMETAGS = 20;	// the people near you to talk to: N records; "" none (Dialogue/RZ_Nametags.c, UI/RZ_NametagClient.c)
	static const int PAYLOAD_POWER = 21;	// the power panel: P, B, C, A records; F busy; X|reason closes (World/RZ_PowerPanel.c)
	static const int PAYLOAD_HISTORY = 22;	// the Journal's past jobs, asked when it opens (Missions/RZ_JobHistory.c)
	static const int DETAIL_MAX = 1500;
	static const int ERRAND_ID_BASE = 100000;	// errand rows' ids: -(base + errand id)
	static const int TRIBE_ID_BASE = 200000;	// the packs' rows (what you know of them): -(base + pack id)
	static const int DOORS_ID = 300000;			// the one Read on doors row: -DOORS_ID (docs/TRACES.md)
	static const int FINALE_ID = 400000;		// the one finale page row: -FINALE_ID (docs/FINALE.md)

	protected static const int MAX_PAYLOAD = 6000;
	protected static const int MAX_CLUES = 30;
	protected static const float JUMP_SECONDS = 5;	// the infection timer is re-sent when it jumps more than this

	protected RZ_GameModeComponent m_GM;
	protected ref RZ_QuestTracker m_Tracker;
	protected ref map<int, ref RZ_UIFeedState> m_mStates = new map<int, ref RZ_UIFeedState>();
	protected ref map<int, ref array<ref RZ_UIClue>> m_mClues = new map<int, ref array<ref RZ_UIClue>>();
	protected ref map<string, string> m_mGear = new map<string, string>();				// loadout name -> what it spawned with
	protected ref map<string, ref array<int>> m_mVisited = new map<string, ref array<int>>();	// identity -> holdings seen up close
	protected float m_fVisitTimer;

	protected static const float VISIT_RADIUS = 300;
	protected static const int GEAR_CAP = 10;

	//------------------------------------------------------------------------------------------------
	void RZ_UIFeed(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		m_Tracker = new RZ_QuestTracker(gm);
	}

	//------------------------------------------------------------------------------------------------
	RZ_QuestTracker GetTracker()
	{
		return m_Tracker;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_UIFeedState State(int playerId)
	{
		RZ_UIFeedState st;
		if (!m_mStates.Find(playerId, st))
		{
			st = new RZ_UIFeedState();
			m_mStates.Set(playerId, st);
		}

		return st;
	}

	//------------------------------------------------------------------------------------------------
	//! A new life: forget what was sent, so everything is sent fresh.
	void OnPlayerSpawned(int playerId)
	{
		m_mStates.Remove(playerId);
		m_mClues.Remove(playerId);
		m_Tracker.Reset(playerId);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendNewLife();

		// The notebook item, once the inventory exists
		if (!RZ_UISettings.Get().m_sNotebookPrefab.IsEmpty())
			GetGame().GetCallqueue().CallLater(GiveNotebook, 3000, false, playerId);

		// A compass and a paper map (docs/DESIGN.md, "The wind-up radio and faction voices")
		GetGame().GetCallqueue().CallLater(GiveMapAndCompass, 3500, false, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Every new life gets vanilla's compass and folded paper map (UI > Compass prefab, Map prefab),
	//! unless its loadout already has one of each; one that doesn't fit lies at their feet.
	protected void GiveMapAndCompass(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent || !RZ_Util.IsCharacterAlive(ent))
			return;

		RZ_UISettings settings = RZ_UISettings.Get();
		if (!settings.m_sCompassPrefab.IsEmpty() && !CarriesWith(ent, SCR_CompassComponent))
			GiveLoose(ent, settings.m_sCompassPrefab);

		if (!settings.m_sMapPrefab.IsEmpty() && !CarriesWith(ent, SCR_MapGadgetComponent))
			GiveLoose(ent, settings.m_sMapPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Does the character carry an item with this component (anywhere on them)?
	protected static bool CarriesWith(IEntity ent, typename component)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(ent.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (item && item.FindComponent(component))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static void GiveLoose(IEntity ent, ResourceName prefab)
	{
		IEntity item = RZ_Util.SpawnPrefab(prefab, ent.GetOrigin() + Vector(0, 0.3, 0));
		if (!item)
			return;

		RZ_GameModeComponent.TrackLooseItem(item);
		RZ_Util.InsertIntoInventory(ent, item);
	}

	//------------------------------------------------------------------------------------------------
	//! Every life carries the notebook (decision: in every loadout). One that doesn't fit lies at
	//! their feet, and is cleaned up in time like any loose item.
	protected void GiveNotebook(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent || !RZ_Util.IsCharacterAlive(ent) || RZ_NotebookItem.Carries(ent))
			return;

		IEntity item = RZ_Util.SpawnPrefab(RZ_UISettings.Get().m_sNotebookPrefab, ent.GetOrigin() + Vector(0, 0.3, 0));
		if (!item)
			return;

		RZ_GameModeComponent.TrackLooseItem(item);
		RZ_Util.InsertIntoInventory(ent, item);
	}

	// ================================================================ Pushed state
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		RZ_UISettings settings = RZ_UISettings.Get();
		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
			if (!ui)
				continue;

			RZ_UIFeedState st = State(playerId);
			st.m_fSinceSync += dt;
			PushInfection(ui, playerId, st, settings);
			PushListening(ui, playerId, st);
		}

		// The quest tracker, owner-only, sent when it changes (UI/RZ_Quests.c)
		m_Tracker.Tick(dt, ids, players);

		m_fVisitTimer += dt;
		if (m_fVisitTimer >= 30)
		{
			m_fVisitTimer = 0;
			NoteVisits(ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Outposts a player has been near: they're on that player's map from then on (this session).
	protected void NoteVisits(array<int> ids, array<IEntity> players)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return;

		foreach (int i, IEntity p : players)
		{
			RZ_Holding h = territory.FindHolding(-1, p.GetOrigin(), VISIT_RADIUS, true);
			if (!h)
				continue;

			string ident = m_GM.GetProfile(ids[i]).m_sIdentity;
			if (ident.IsEmpty())
				continue;

			array<int> seen;
			if (!m_mVisited.Find(ident, seen))
			{
				seen = new array<int>();
				m_mVisited.Set(ident, seen);
			}

			if (!seen.Contains(h.m_iId))
				seen.Insert(h.m_iId);
		}
	}

	// ================================================================ Gear
	//------------------------------------------------------------------------------------------------
	//! From the spawn: a few seconds later (the inventory fills after the body), note what this
	//! loadout carries, for the spawn screen's gear list.
	void RecordGearSoon(int playerId, string loadoutName)
	{
		if (loadoutName.IsEmpty())
			return;

		GetGame().GetCallqueue().CallLater(RecordGear, 3000, false, playerId, loadoutName);
	}

	//------------------------------------------------------------------------------------------------
	protected void RecordGear(int playerId, string loadoutName)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent)
			return;

		array<string> names = {};
		RZ_Util.GearNames(ent, GEAR_CAP, names);
		if (names.IsEmpty())
			return;

		string text = "";
		foreach (int i, string n : names)
		{
			if (i > 0)
				text += ", ";

			text += n;
		}

		m_mGear.Set(loadoutName, text);
	}

	//------------------------------------------------------------------------------------------------
	protected void PushInfection(RZ_PlayerUIComponent ui, int playerId, RZ_UIFeedState st, RZ_UISettings settings)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		RZ_Infection inf = m_GM.GetInfection();
		int state = RZ_EInfectionUI.NONE;
		float left = 0;
		float total = 0;
		float finalSeconds = 0;
		if (p.IsInfected() && inf)
		{
			if (p.m_fWoundLeft > 0)
			{
				state = RZ_EInfectionUI.GOLDEN;
				left = p.m_fWoundLeft;
				total = Math.Max(1, inf.GetSettings().m_fGoldenSeconds);
			}
			else
			{
				RZ_EInfectionStage stage = inf.StageOf(p);
				state = RZ_EInfectionUI.FEVER;
				if (stage == RZ_EInfectionStage.WEAKNESS)
					state = RZ_EInfectionUI.WEAKNESS;
				else if (stage == RZ_EInfectionStage.FINAL)
					state = RZ_EInfectionUI.FINAL;

				left = p.m_fInfectionLeft;
				total = Math.Max(60, p.m_fInfectionTotal);
				finalSeconds = inf.FinalSeconds(p);
			}
		}
		else if (inf && inf.CleanPendingLeft(playerId) > 0)
		{
			// A clean that worked, not told yet: the window still shows until it would have closed
			state = RZ_EInfectionUI.GOLDEN;
			left = inf.CleanPendingLeft(playerId);
			total = Math.Max(1, inf.GetSettings().m_fGoldenSeconds);
		}

		// What the client shows now, counting down from the last send (RZ_PlayerUIComponent.GetInfectionLeft)
		float predicted = st.m_fLeftSent - st.m_fSinceSync;
		if (st.m_fPauseSent > 0)
			predicted = st.m_fLeftSent - Math.Max(0, st.m_fSinceSync - st.m_fPauseSent);

		float predictedPause = Math.Max(0, st.m_fPauseSent - st.m_fSinceSync);
		bool jumped = Math.AbsFloat(left - predicted) > JUMP_SECONDS || Math.AbsFloat(p.m_fPauseLeft - predictedPause) > JUMP_SECONDS;

		bool paused = p.m_fPauseLeft > 0;
		bool changed = state != st.m_iInf || paused != st.m_bPaused;
		bool resync = state != RZ_EInfectionUI.NONE && (jumped || st.m_fSinceSync >= settings.m_fResyncSeconds);
		if (!changed && !resync)
			return;

		st.m_iInf = state;
		st.m_bPaused = paused;
		st.m_fSinceSync = 0;
		st.m_fLeftSent = left;
		st.m_fPauseSent = p.m_fPauseLeft;
		ui.SendInfection(state, left, total, finalSeconds, p.m_fPauseLeft);
	}

	//------------------------------------------------------------------------------------------------
	protected void PushListening(RZ_PlayerUIComponent ui, int playerId, RZ_UIFeedState st)
	{
		int faction = -1;
		float secondsLeft = 0;
		if (m_GM.GetInteractions() && m_GM.GetInteractions().GetRadio())
			faction = m_GM.GetInteractions().GetRadio().GetListening(playerId, secondsLeft);

		if (faction == st.m_iListen)
			return;

		st.m_iListen = faction;
		ui.SendListening(faction, secondsLeft);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_Stealth's update: the HUD's one bar, a level 0 (hidden) to 6 (loud) in stealth's own
	//! bands (RZ_Stealth.HudLevel), whether that counts as hidden, and whether any noise is part of it.
	//! Sent only when it changes.
	void PushVisibility(int playerId, int level, bool hidden, bool noisy)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!ui)
			return;

		RZ_UIFeedState st = State(playerId);
		if (level == st.m_iSeen && hidden == st.m_bSeenHidden && noisy == st.m_bSeenNoisy)
			return;

		st.m_iSeen = level;
		st.m_bSeenHidden = hidden;
		st.m_bSeenNoisy = noisy;
		ui.SendVisibility(level, hidden, noisy);
	}

	//------------------------------------------------------------------------------------------------
	//! From borrowed kit (RZ_DisguiseClaims): the HUD's look-coming line, RZ_Hud_Look ("PASS AS
	//! BASTION: 2 OF 2"; "" hides it). Owner-only, sent only when it changes.
	void PushLook(int playerId, string line)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!ui)
			return;

		RZ_UIFeedState st = State(playerId);
		if (st.m_sLook == line)
			return;

		st.m_sLook = line;
		ui.SendLook(line);
	}

	// ================================================================ Clues
	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleMission.HandleRead.
	void OnClueRead(int playerId, int missionId, string title, string text)
	{
		array<ref RZ_UIClue> list;
		if (!m_mClues.Find(playerId, list))
		{
			list = new array<ref RZ_UIClue>();
			m_mClues.Set(playerId, list);
		}

		foreach (RZ_UIClue old : list)
		{
			if (old.m_iMission == missionId && old.m_sTitle == title)
			{
				old.m_sText = text;
				return;
			}
		}

		RZ_UIClue c = new RZ_UIClue();
		c.m_iMission = missionId;
		c.m_sTitle = title;
		c.m_sText = text;
		list.Insert(c);
		if (list.Count() > MAX_CLUES)
			list.RemoveOrdered(0);
	}

	// ================================================================ Payloads
	//------------------------------------------------------------------------------------------------
	string BuildPayload(int playerId, int kind)
	{
		string payload;
		if (kind == PAYLOAD_STANDING)
			payload = BuildStanding(playerId);
		else if (kind == PAYLOAD_JOURNAL)
			payload = BuildJournal(playerId);
		else if (kind == PAYLOAD_MAP)
			payload = BuildMap(playerId);
		else if (kind == PAYLOAD_BOARD)
			payload = BuildBoard(playerId);
		else if (kind == PAYLOAD_HALL)
			payload = BuildHall();
		else if (kind == PAYLOAD_SEASON)
			payload = BuildSeason(playerId);
		else if (kind == PAYLOAD_HISTORY)
			payload = RZ_JobHistory.Build(m_GM, playerId);
		else
			payload = BuildBackgrounds(playerId);

		// Too long: cut after the last whole record that fits, never in the middle of one
		if (payload.Length() > MAX_PAYLOAD)
		{
			payload = payload.Substring(0, MAX_PAYLOAD);
			int lastBreak = payload.LastIndexOf("\n");
			if (lastBreak >= 0)
				payload = payload.Substring(0, lastBreak + 1);
			else
				payload = "";
		}

		return payload;
	}

	//------------------------------------------------------------------------------------------------
	//! The notice board the player stands at (RZ_NoticeBoard.BuildPayload), "" if none.
	protected string BuildBoard(int playerId)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || !houses.GetBoard())
			return "";

		return houses.GetBoard().BuildPayload(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The hall of fame (RZ_HallOfFame.BuildPayload), "" if it's off.
	//------------------------------------------------------------------------------------------------
	//! The season tab: M|line (each major as it stands, Director/RZ_Majors.c), Z|line (the season as
	//! the Zone tells it, the epilogue's) and Y|line (your lines and what's still open). One record a
	//! line.
	protected string BuildSeason(int playerId)
	{
		string s = "";
		array<string> majors = {};
		if (RZ_Majors.Get())
			RZ_Majors.Get().SeasonLines(majors);

		foreach (string m : majors)
		{
			RZ_UILine rec = new RZ_UILine("M");
			s += rec.Add(m).End();
		}

		array<string> zone = {};
		array<string> you = {};
		RZ_Epilogue epi = RZ_Epilogue.Get();
		if (epi)
			epi.SeasonParts(playerId, zone, you);

		foreach (string z : zone)
		{
			RZ_UILine zr = new RZ_UILine("Z");
			s += zr.Add(z).End();
		}

		foreach (string y : you)
		{
			RZ_UILine yr = new RZ_UILine("Y");
			s += yr.Add(y).End();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected string BuildHall()
	{
		RZ_HallOfFame hall = m_GM.GetHall();
		if (!hall)
			return "";

		return hall.BuildPayload();
	}

	//------------------------------------------------------------------------------------------------
	//! H|player name|background|life|the whole picture (RZ_RepNamesPage.Header)
	//! F|faction|fame|infamy|favor|tier|need|need line|rep (RZ_ERep)|notoriety|word|note|left to forgive (-1 no budget)|owed (0/1)
	//! The fields after the need line are new (docs/REPUTATION.md, "Reputations"): older layouts
	//! read the first eight and ignore the rest. The word is what the faction calls you; owed is 1 while
	//! they owe you a hearing (the parley), and the note then says so.
	//! L|faction|name|title|what they think of you
	//! K|faction|borrowed kit with them (the Standing page's In their kit)|they know your face (0/1)|
	//!   catches this life|blown (0/1): the numbers tint the row's kit tell
	//! W|faction|the word as a sentence|the nearest met leader's verdict (the leaders column's first two lines)
	//! R|kind (0 truce, 1 feud)|faction a|faction b|minutes left
	protected string BuildStanding(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		RZ_Background bg = m_GM.GetBackground(p.m_sBackgroundId);
		string bgName = "";
		if (bg)
			bgName = bg.m_sName;

		RZ_UILine head = new RZ_UILine("H");
		string s = head.Add(RZ_Util.PlayerName(playerId)).Add(bgName).AddInt(p.m_iLives).Add(RZ_RepNamesPage.Header(m_GM, playerId)).End();
		IEntity standingEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		vector standingAt = vector.Zero;
		if (standingEnt)
			standingAt = standingEnt.GetOrigin();

		RZ_FactionLife life = m_GM.GetFactionLife();
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			int need = RZ_ENeed.NONE;
			string needLine = "";
			if (life)
				need = life.GetNeed(f);
			if (need != RZ_ENeed.NONE)
				needLine = RZ_FactionVoice.NeedLine(f, need);


			RZ_EFaction ef = f;
			int tier = p.GetTier(ef);
			int fame = p.GetFame(ef);
			int infamy = p.GetInfamy(ef);
			int rep = RZ_RepNames.GetRep(fame, infamy);
			if (!RZ_ReputationSettings.Get().m_bNames)
				rep = -1;

			int left = -1;
			if (RZ_ReputationSettings.Get().m_iForgiveBudget > 0)
				left = p.ForgiveLeft(ef);

			// The owed (the parley): the note says so; the word is what they call you to your face
			int owed = 0;
			string note = RZ_RepNames.Note(fame, infamy);
			if (RZ_RepVoice.IsOwed(m_GM, playerId, f))
			{
				owed = 1;
				note = "Hunted - they owe you a hearing";
			}

			RZ_UILine lnF = new RZ_UILine("F");
			lnF.AddInt(f).AddInt(fame).AddInt(infamy).AddInt(p.GetFavor(ef)).AddInt(tier).AddInt(need).Add(needLine);
			lnF.AddInt(rep).AddInt(RZ_RepNames.Notoriety(fame, infamy)).Add(RZ_RepVoice.Word(m_GM, playerId, f)).Add(note).AddInt(left).AddInt(owed);
			s += lnF.End();
			// Borrowed kit with them (RZ_DisguiseClaims): K|faction|your face, catches, blown, tells, enemies|face|catches|blown
			string kit = "";
			RZ_Disguise disguise;
			if (m_GM.GetHostility())
				disguise = m_GM.GetHostility().GetDisguise();

			if (disguise)
				kit = disguise.GetClaims().StandingLine(playerId, f);

			if (!kit.IsEmpty())
			{
				int face = 0;
				if (disguise.KnowsFace(playerId, f))
					face = 1;

				int blown = 0;
				if (disguise.IsBlown(playerId, f))
					blown = 1;

				RZ_UILine lnK = new RZ_UILine("K");
				s += lnK.AddInt(f).Add(kit).AddInt(face).AddInt(p.m_aCatches[f]).AddInt(blown).End();
			}

			// The word and the nearest met leader's verdict open the leaders column (RZ_RepNamesPage)
			if (RZ_ReputationSettings.Get().m_bNames)
			{
				RZ_UILine lnW = new RZ_UILine("W");
				s += lnW.AddInt(f).Add(RZ_RepNamesPage.WordLine(m_GM, playerId, f)).Add(RZ_RepNamesPage.Verdict(m_GM, playerId, f, standingAt)).End();
			}
		}

		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
		{
			array<RZ_Leader> met = {};
			leaders.ListMet(playerId, met);
			foreach (RZ_Leader l : met)
			{
				int lf = l.m_eFaction;
				RZ_UILine lnL = new RZ_UILine("L");
				s += lnL.AddInt(lf).Add(l.m_sName).Add(l.m_sTitle).Add(leaders.OpinionLine(l, playerId)).End();
			}
		}

		if (life)
		{
			array<int> a = {};
			array<int> b = {};
			array<int> kinds = {};
			array<float> minutes = {};
			life.GetRelations(a, b, kinds, minutes);
			foreach (int r, int fa : a)
			{
				RZ_UILine lnR = new RZ_UILine("R");
				s += lnR.AddInt(kinds[r]).AddInt(fa).AddInt(b[r]).AddInt(RZ_Util.RoundToInt(minutes[r])).End();
			}
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! K|the tracked job's id (0 none)
	//! M|id|kind|giver (-1 none)|title|given by|where|rough area|minutes left|requirements|Fame|Favor|brief|taking part (0/1)|
	//!   kind label ("The season's story")|current objective (one line)
	//! C|mission id|clue title|clue text
	//! S|the lab's line ("Research 63 percent. Waiting: 14 samples' worth, about 4 days of work.")
	//! A questline the player is part of is an M record too: a negative id, kind QUESTLINE, its
	//! title, the stage as "given by", minutes -1 (no clock), Fame and Favor 0 (they depend on
	//! the ending), the stage's text as the brief; its notes ("what you know so far") are C records.
	protected string BuildJournal(int playerId)
	{
		RZ_MissionManager mm = m_GM.GetMissions();
		if (!mm)
			return "";

		// The lab's state first, so a long journal never cuts it off (RZ_CureResearch.Describe)
		string lab = "";
		if (m_GM.HasLab() && m_GM.GetCure())
		{
			RZ_UILine lnS = new RZ_UILine("S");
			// The lab door's sheet, a board of record (Missions/Story/RZ_Grey.c)
			string labLine = m_GM.GetCure().Describe();
			string sheet = RZ_RecordBoards.LabSheet();
			if (!sheet.IsEmpty())
				labLine = labLine + " " + sheet;

			lab = lnS.Add(labLine).End();
		}

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		vector from = vector.Zero;
		if (ent)
			from = ent.GetOrigin();

		array<RZ_Mission> list = {};
		mm.ListFor(playerId, list);
		RZ_MissionReasons.SortFor(m_GM, list, playerId, from);	// near, reason-started, a leader they helped first
		RZ_UILine lnK = new RZ_UILine("K");
		string s = lab + lnK.AddInt(m_Tracker.TrackedFor(playerId)).End();
		array<int> shown = {};
		foreach (RZ_Mission m : list)
		{
			// A questline's stage job is the questline's own row (BuildQuestlines), never a second one
			if (RZ_QuestTracker.IsStageOfListedRun(m_GM, m, playerId))
				continue;

			int giver = -1;
			if (m.m_bHasGiver)
				giver = m.m_Req.m_eGiver;


			string by = "";
			if (m.m_iLeaderId >= 0 && m_GM.GetLeaders())
			{
				RZ_Leader l = m_GM.GetLeaders().Get(m.m_iLeaderId);
				if (l)
					by = l.GetHeader();
			}

			// A rough area, never metres (docs/UI.md, "The quest tracker"), to where the current
			// objective is done; contracts point at the rough circle until the player knows the spot
			// (RZ_Mission). The mission's own place without one.
			RZ_ObjList objs = new RZ_ObjList();
			m_Tracker.Objectives(playerId, m.m_iId, from, objs);
			RZ_ObjPlace nowAt = objs.Now();
			string where = m.GetLocationNameFor(m_GM, playerId, from);
			string dir = "";
			if (ent && !m.m_bGlobal)
				dir = RZ_QuestText.RoughArea(m_GM, from, m.GetAreaFor(m_GM, playerId, from));

			if (nowAt)
			{
				if (ent)
					dir = RZ_QuestText.RoughArea(m_GM, from, nowAt.m_vPos);

				if (!nowAt.IsRough())
					where = nowAt.Words(m_GM);
			}

			string req = "";
			if (m.m_bContract)
				req = m.m_Req.Describe();

			// A parley's amends job says so (RZ_RepNamesPage)
			string amends = RZ_RepNamesPage.JournalLine(m_GM, m, playerId);
			if (!amends.IsEmpty())
				req = amends;

			string part = "0";
			if (m.m_aParticipants.Contains(playerId))
				part = "1";

			RZ_UILine lnM = new RZ_UILine("M");
			lnM.AddInt(m.m_iId).AddInt(MissionKind(m)).AddInt(giver).Add(m.m_sTitle).Add(by).Add(where).Add(dir);
			lnM.AddInt(RZ_Util.RoundToInt(m.m_fTimeLeft / 60)).Add(req).AddInt(m.m_iFameReward).AddInt(m.m_iFavorReward).Add(m.GetBrief(m_GM)).Add(part);
			lnM.Add(RZ_QuestText.KindLabel(MissionKind(m), false)).Add(objs.First(RZ_EObjState.CURRENT));
			s += lnM.End();
			shown.Insert(m.m_iId);
		}

		s += BuildQuestlines(playerId, ent, from);
		s += BuildErrands(playerId, ent, from);
		s += BuildInfected(playerId, ent, from);
		s += BuildFinale(playerId);
		s += BuildDoors(playerId);
		s += RZ_DialogueJournal.Rows(playerId);

		array<ref RZ_UIClue> clues;
		if (m_mClues.Find(playerId, clues))
		{
			foreach (RZ_UIClue c : clues)
			{
				if (shown.Contains(c.m_iMission))
				{
					RZ_UILine lnC = new RZ_UILine("C");
					s += lnC.AddInt(c.m_iMission).Add(c.m_sTitle).Add(c.m_sText).End();
				}
			}
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! A job's objectives for its journal row (RZ_QuestTracker.Objectives): its current one in one
	//! line, and where that's done.
	protected RZ_ObjList ObjsFor(int playerId, int id, vector from)
	{
		RZ_ObjList list = new RZ_ObjList();
		m_Tracker.Objectives(playerId, id, from, list);
		return list;
	}

	//------------------------------------------------------------------------------------------------
	//! A journal row's area in words: to the current objective's place, else `fallback` (the job's
	//! own place, zero for none). "" without a player to measure from.
	protected string RowArea(RZ_ObjList list, IEntity ent, vector from, vector fallback)
	{
		if (!ent)
			return "";

		RZ_ObjPlace at = list.Now();
		if (at)
			return RZ_QuestText.RoughArea(m_GM, from, at.m_vPos);

		if (fallback == vector.Zero)
			return "";

		return RZ_QuestText.RoughArea(m_GM, from, fallback);
	}

	//------------------------------------------------------------------------------------------------
	//! The errands this player holds (Missions/Errands), as journal records: the giver as "by", the
	//! exact place, the real minutes left, what you said you'd do. Never on the map.
	protected string BuildErrands(int playerId, IEntity ent, vector from)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands)
			return "";

		array<RZ_Errand> held = {};
		errands.ForJournal(playerId, held);
		string s = "";
		foreach (RZ_Errand e : held)
		{
			// The step's place, then the giver's (RZ_QuestTracker.ErrandObjectives)
			int errandRow = -(ERRAND_ID_BASE + e.m_iId);
			RZ_ObjList objs = ObjsFor(playerId, errandRow, from);
			vector fallback = e.m_vPlace;
			if (e.m_bStepDone)
				fallback = errands.GiverAt(e);

			string dir = RowArea(objs, ent, from, fallback);
			RZ_UILine lnE = new RZ_UILine("M");
			lnE.AddInt(errandRow).AddInt(RZ_EMissionKindUI.ERRAND).AddInt(e.m_iFaction).Add(e.Fill(e.m_Def.m_sTitle)).Add(e.m_sGiverTitle).Add(e.GetText("here")).Add(dir);
			lnE.AddInt(errands.MinutesLeft(e)).Add("Asked of you").AddInt(0).AddInt(0).Add(errands.JournalText(e)).Add("1");
			lnE.Add(RZ_QuestText.KindLabel(RZ_EMissionKindUI.ERRAND, false)).Add(objs.First(RZ_EObjState.CURRENT));
			s += lnE.End();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The packs this character knows of (docs/INFECTED.md, Knowing), as journal records of kind PACK
	//! (the range ring icon), Mapped first, then Read, then Heard of: the place as the title, the level
	//! as "by", a rough area, the names heard under the place. The map's rings are BuildMap's.
	protected string BuildInfected(int playerId, IEntity ent, vector from)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes || !tribes.GetPeople().IsOn())
			return "";

		array<int> ids = {};
		array<string> titles = {};
		array<string> bys = {};
		array<vector> wheres = {};
		array<string> briefs = {};
		tribes.GetPeople().JournalFor(playerId, from, ids, titles, bys, wheres, briefs);
		string s = "";
		foreach (int i, int tribe : ids)
		{
			string dir = "";
			if (ent && wheres[i] != vector.Zero)
				dir = RZ_QuestText.RoughArea(m_GM, from, wheres[i]);

			RZ_UILine ln = new RZ_UILine("M");
			ln.AddInt(-(TRIBE_ID_BASE + tribe)).AddInt(RZ_EMissionKindUI.PACK).AddInt(-1).Add(titles[i]).Add(bys[i]).Add(titles[i]).Add(dir);
			ln.AddInt(-1).Add("").AddInt(0).AddInt(0).Add(briefs[i]).Add("0");
			s += ln.End();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Read on doors (docs/TRACES.md): one row while this character has read any door, the count as
	//! its "by", the newest line as its brief; the detail lists the last 12.
	protected string BuildDoors(int playerId)
	{
		RZ_Traces traces = m_GM.GetTraces();
		if (!traces)
			return "";

		array<string> lines = {};
		traces.JournalFor(playerId, lines);
		if (lines.IsEmpty())
			return "";

		string count = string.Format("%1 read", lines.Count());
		RZ_UILine ln = new RZ_UILine("M");
		ln.AddInt(-DOORS_ID).AddInt(RZ_EMissionKindUI.DOORS).AddInt(-1).Add("Read on doors").Add(count).Add("").Add("");
		ln.AddInt(-1).Add("").AddInt(0).AddInt(0).Add(lines[0]).Add("0");
		return ln.End();
	}

	//------------------------------------------------------------------------------------------------
	//! The finale page (docs/FINALE.md): one row while the routes are open, the readout's first line
	//! as its brief; the detail is the whole page (RZ_CureFinale.PageFor), the desk's *How do things
	//! stand?* without the walk.
	protected string BuildFinale(int playerId)
	{
		RZ_CureFinale finale = m_GM.GetFinale();
		if (!finale)
			return "";

		string page = finale.PageFor(playerId);
		if (page.IsEmpty())
			return "";

		array<string> pageLines = {};
		page.Split("\n", pageLines, true);
		if (pageLines.IsEmpty())
			return "";

		RZ_UILine ln = new RZ_UILine("M");
		ln.AddInt(-FINALE_ID).AddInt(RZ_EMissionKindUI.FINALE).AddInt(-1).Add("The finale").Add("How things stand").Add("").Add("");
		ln.AddInt(-1).Add("").AddInt(0).AddInt(0).Add(pageLines[0]).Add("0");
		return ln.End();
	}

	//------------------------------------------------------------------------------------------------
	//! One journal row's detail, sent when the row is selected (at most 1,500 characters, so the
	//! list's 6,000 is never cut for it): D|id|what they told you|what you've found|who it went to.
	string BuildDetail(int playerId, int id)
	{
		// Promises and What you know (Dialogue/RZ_DialogueJournal.c)
		if (RZ_DialogueJournal.IsOurs(id))
			return RZ_DialogueJournal.Detail(playerId, id);

		string told = "";
		string found = "";
		string went = "";
		if (id == -DOORS_ID)
		{
			RZ_Traces traces = m_GM.GetTraces();
			array<string> read = {};
			if (traces)
				traces.JournalFor(playerId, read);

			foreach (string doorLine : read)
			{
				told += doorLine + "\n\n";
			}

			told = told.Trim();
		}
		else if (id == -FINALE_ID)
		{
			if (!m_GM.GetFinale())
				return "";

			told = m_GM.GetFinale().PageFor(playerId);
		}
		else if (id <= -TRIBE_ID_BASE)
		{
			RZ_Tribes tribes = m_GM.GetTribes();
			array<int> tids = {};
			array<string> ttitles = {};
			array<string> tbys = {};
			array<vector> twheres = {};
			array<string> tbriefs = {};
			if (tribes)
				tribes.GetPeople().JournalFor(playerId, vector.Zero, tids, ttitles, tbys, twheres, tbriefs);

			int at = tids.Find(-id - TRIBE_ID_BASE);
			if (at < 0)
				return "";

			told = tbriefs[at];
		}
		else if (id <= -ERRAND_ID_BASE && m_GM.GetErrands())
		{
			RZ_Errand e = m_GM.GetErrands().FindById(-id - ERRAND_ID_BASE);
			if (!e || e.m_iAsker != playerId)
				return "";

			told = e.Fill(e.m_Def.m_sTitle) + ": " + m_GM.GetErrands().JournalText(e);
		}
		else if (id > 0 && m_GM.GetMissions())
		{
			RZ_Mission m = m_GM.GetMissions().FindById(id);
			if (!m)
				return "";

			// Only a mission this player's journal shows (BuildJournal's test): a client-sent id
			// never reads someone else's brief, finds or result
			array<RZ_Mission> visible = {};
			m_GM.GetMissions().ListFor(playerId, visible);
			if (!visible.Contains(m))
				return "";

			told = m.GetBrief(m_GM);
			if (m.m_Story)
			{
				foreach (string line : m.m_Story.m_aFound)
				{
					found += line + "\n";
				}

				RZ_Beneficiaries parties = m.m_Story.m_Parties;
				if (parties.m_iChosen >= 0)
					went = parties.m_aParties[parties.m_iChosen].m_sName;
			}
		}
		else if (id < 0 && id > -ERRAND_ID_BASE && m_GM.GetQuestlines())
		{
			// A questline the player sees: its journal text (the stage's line, the season's)
			RZ_Questlines q = m_GM.GetQuestlines();
			bool seen = false;
			foreach (RZ_QuestRun run : q.GetRuns())
			{
				if (run.m_iId == -id && q.IsVisibleTo(run, playerId))
				{
					told = q.JournalFor(run);
					seen = true;
					break;
				}
			}

			if (!seen)
				return "";
		}
		else
		{
			return "";
		}

		// The objectives, O|id|state|line, before the D record so the cap cuts the brief, not them
		string objectives = "";
		RZ_ObjList list = new RZ_ObjList();
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		vector from = vector.Zero;
		if (ent)
			from = ent.GetOrigin();

		m_Tracker.Objectives(playerId, id, from, list);
		// O|id|state|line|place: an open objective's place in words when it differs from the one
		// before it ("Go back to Mara" after a step elsewhere: "Morton")
		string lastPlace = "";
		foreach (int oi, string objLine : list.m_aLines)
		{
			int objState = list.m_aStates[oi];
			string objPlace = "";
			if (objState != RZ_EObjState.DONE && objState != RZ_EObjState.FAILED)
			{
				objPlace = m_Tracker.OtherPlace(list.m_aPlaces[oi], lastPlace);
				if (!objPlace.IsEmpty())
					lastPlace = objPlace;
			}

			RZ_UILine lnO = new RZ_UILine("O");
			objectives += lnO.AddInt(id).AddInt(objState).Add(objLine).Add(objPlace).End();
		}

		RZ_UILine ln = new RZ_UILine("D");
		string payload = objectives + ln.AddInt(id).Add(told).Add(found.Trim()).Add(went).End();
		if (payload.Length() > DETAIL_MAX)
			payload = payload.Substring(0, DETAIL_MAX);

		return payload;
	}

	//------------------------------------------------------------------------------------------------
	//! The questlines this player is part of, as journal records (see BuildJournal).
	protected string BuildQuestlines(int playerId, IEntity ent, vector from)
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (!q || !q.IsEnabled())
			return "";

		string s = "";
		foreach (RZ_QuestRun run : q.GetRuns())
		{
			if (!q.IsVisibleTo(run, playerId))
				continue;

			RZ_QuestStage st = run.m_Line.GetStage(run.m_iStage);
			string stageTitle = "";
			if (st)
				stageTitle = run.Fill(st.m_sTitle);

			// The stage's current objective's place (its stage mission's), else the questline's own
			int id = -run.m_iId;
			RZ_ObjList objs = ObjsFor(playerId, id, from);
			string where = run.m_sPlaceName;
			vector fallback = vector.Zero;
			if (run.m_bHasPlace)
				fallback = run.m_vPlace;

			string dir = RowArea(objs, ent, from, fallback);
			RZ_ObjPlace nowAt = objs.Now();
			if (nowAt && !nowAt.IsRough())
				where = nowAt.Words(m_GM);

			// Taking part in the stage's job is taking part in the questline (as the tracker counts it)
			string part = "0";
			RZ_Mission stageJob = null;
			if (m_GM.GetMissions())
				stageJob = m_GM.GetMissions().FindById(run.m_iMissionId);

			if (run.PlayerTookPart(m_GM, playerId) || (stageJob && stageJob.m_aParticipants.Contains(playerId)))
				part = "1";

			RZ_UILine lnQ = new RZ_UILine("M");
			lnQ.AddInt(id).AddInt(RZ_EMissionKindUI.QUESTLINE).AddInt(run.m_Line.m_iFaction).Add(run.m_Line.m_sTitle).Add(stageTitle).Add(where).Add(dir);
			lnQ.AddInt(-1).Add("Whoever takes the next step").AddInt(0).AddInt(0).Add(q.JournalFor(run)).Add(part);
			lnQ.Add(RZ_QuestText.KindLabel(RZ_EMissionKindUI.QUESTLINE, run.m_Line.IsMajor())).Add(objs.First(RZ_EObjState.CURRENT));
			s += lnQ.End();

			foreach (int n, string note : run.m_aNotes)
			{
				RZ_UILine lnN = new RZ_UILine("C");
				s += lnN.AddInt(id).Add(run.m_aNoteTitles[n]).Add(note).End();
			}
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! G|legacy|Legacy for the Drifter
	//! B|id|name|loadout|perk|flaw|locked (0/1)|unlock text|Haven|Bastion|Red Hand|Halcyon (signed start standing)|gear
	protected string BuildBackgrounds(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		RZ_UILine lnG = new RZ_UILine("G");
		string s = lnG.AddInt(p.m_iLegacy).AddInt(RZ_GameModeComponent.GetDrifterLegacy()).End();
		array<ref RZ_Background> list = m_GM.GetBackgrounds();
		if (!list)
			return s;

		foreach (RZ_Background bg : list)
		{
			string locked = "0";
			if (bg.m_bLocked && !p.HasUnlocked(bg.m_sUnlockId))
				locked = "1";

			RZ_UILine lnB = new RZ_UILine("B");
			lnB.Add(bg.m_sId).Add(bg.m_sName).Add(bg.m_sLoadoutName).Add(bg.m_sPerk).Add(bg.m_sFlaw).Add(locked).Add(bg.m_sUnlockText);
			lnB.AddInt(bg.GetStartRep(RZ_EFaction.SURVIVORS)).AddInt(bg.GetStartRep(RZ_EFaction.MILITARY));
			string gear;
			if (!m_mGear.Find(bg.m_sLoadoutName, gear))
				gear = bg.m_sGear;

			lnB.AddInt(bg.GetStartRep(RZ_EFaction.BANDITS)).AddInt(bg.GetStartRep(RZ_EFaction.INSTITUTE)).Add(gear);
			s += lnB.Add(RZ_OpeningText.ScreenPhrase(bg.m_sId)).End();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! What this player knows the place of, for the map's icon layer (UI/RZ_MapIcons.c). Of the jobs,
	//! only the tracked one's current objective is on it, and for this player alone (last, so it's
	//! drawn above the rest); no other contract, event or errand. The packs they know of are range
	//! rings sized to the ground (a radius in metres as the last field). A place's label is its kind
	//! alone ("Stronghold", "Safehouse"): the disc's colour and badge say whose it is. One place is
	//! one marker: Halcyon's stronghold at the lab is the lab's, and a safehouse at the Haven's
	//! stronghold is the stronghold's.
	//! P|icon|faction (-1 none)|x|z|label[|radius]
	protected string BuildMap(int playerId)
	{
		string s = "";
		bool lab = m_GM.HasLab();
		vector labPos = m_GM.GetLabPosition();
		if (lab)
			s += MapEntry("marker_lab", RZ_EFaction.INSTITUTE, labPos, "Lab");

		// Strongholds are common knowledge; an outpost only once they know the faction or have been there
		bool haven = false;
		vector havenSeat;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			array<int> seen;
			m_mVisited.Find(m_GM.GetProfile(playerId).m_sIdentity, seen);
			foreach (RZ_Holding hold : territory.GetHoldings())
			{
				if (hold.m_bStronghold)
				{
					if (hold.m_eFaction == RZ_EFaction.INSTITUTE && lab && RZ_Util.DistXZ(hold.m_vPos, labPos) <= MAP_SAME_PLACE)
						continue;

					if (hold.m_eFaction == RZ_EFaction.SURVIVORS)
					{
						haven = true;
						havenSeat = hold.m_vPos;
					}

					s += MapEntry("marker_stronghold", hold.m_eFaction, hold.m_vPos, "Stronghold");
					continue;
				}

				bool known = RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, hold.m_eFaction), RZ_ETier.KNOWN);
				if (!known && (!seen || !seen.Contains(hold.m_iId)))
					continue;

				string outpost = "Outpost";
				if (hold.m_eFaction == RZ_EFaction.INSTITUTE)
					outpost = "Field station";

				s += MapEntry("marker_outpost", hold.m_eFaction, hold.m_vPos, outpost);
			}
		}

		// Safehouses: everyone knows them, except the people the Haven turned out
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && !RZ_Reputation.IsHunted(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS)))
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (haven && RZ_Util.DistXZ(h.m_vPos, havenSeat) <= MAP_SAME_PLACE)
					continue;

				s += MapEntry("marker_safehouse", RZ_EFaction.SURVIVORS, h.m_vPos, "Safehouse");
			}
		}

		RZ_PowerGrid grid = m_GM.GetPowerGrid();
		if (grid && grid.IsActive())
			s += MapEntry("marker_power", -1, grid.GetPosition(), "Power station");

		RZ_RadioStation radio = m_GM.GetRadioStation();
		if (radio && radio.IsActive())
			s += MapEntry("marker_radio", -1, radio.GetPosition(), "Radio station");

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (perimeter)
		{
			array<vector> checkpoints = {};
			perimeter.GetCheckpoints(checkpoints);
			foreach (vector cp : checkpoints)
			{
				s += MapEntry("marker_checkpoint", -1, cp, "Checkpoint");
			}
		}

		// The packs this character knows of: a ring round the ground (Heard of: round the rough area)
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes && tribes.GetPeople().IsOn())
		{
			array<vector> rings = {};
			array<float> radii = {};
			array<string> labels = {};
			tribes.GetPeople().MapFor(playerId, rings, radii, labels);
			foreach (int r, vector ring : rings)
			{
				RZ_UILine lnR = new RZ_UILine("P");
				s += lnR.Add("marker_range").AddInt(-1).AddInt(RZ_Util.RoundToInt(ring[0])).AddInt(RZ_Util.RoundToInt(ring[2])).Add(labels[r]).AddInt(RZ_Util.RoundToInt(radii[r])).End();
			}
		}

		// The tracked job's current objective: a diamond on a known spot, a search area round a rough
		// one (UI/RZ_Quests.c). No other job is on the map
		s += m_Tracker.MapObjective(playerId);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected string MapEntry(string icon, int faction, vector pos, string label)
	{
		RZ_UILine ln = new RZ_UILine("P");
		return ln.Add(icon).AddInt(faction).AddInt(RZ_Util.RoundToInt(pos[0])).AddInt(RZ_Util.RoundToInt(pos[2])).Add(label).End();
	}

	//------------------------------------------------------------------------------------------------
	//! The journal's type icon. Leader jobs and puzzles by flag and base class; the unarmed,
	//! Perimeter and arc missions by class (docs/MISSIONS.md lists them).
	static int MissionKind(RZ_Mission m)
	{
		if (m.m_iQuestRun >= 0)
			return RZ_EMissionKindUI.QUESTLINE;

		if (m.m_bInPersonOnly || m.m_iJobFaction >= 0)
			return RZ_EMissionKindUI.LEADER;

		typename t = m.Type();
		if (t == RZ_Mission_BlindSpot || t == RZ_Mission_TheCrossing || t == RZ_Mission_ShotAtTheLine)
			return RZ_EMissionKindUI.PERIMETER;

		if (t == RZ_Mission_DeserterMap || t == RZ_Mission_FeverWater || t == RZ_Mission_FeverEvacuation)
			return RZ_EMissionKindUI.ARC;

		if (t == RZ_Mission_CarKeys
			|| t == RZ_Mission_LeadThemAway
			|| t == RZ_Mission_TrailOfClues || t == RZ_Mission_BarterChain || t == RZ_Mission_Distraction || t == RZ_Mission_QuietCount)
			return RZ_EMissionKindUI.UNARMED;

		if (m.IsInherited(RZ_PuzzleMission))
			return RZ_EMissionKindUI.PUZZLE;

		if (!m.m_bContract)
			return RZ_EMissionKindUI.EVENT;

		return RZ_EMissionKindUI.CONTRACT;
	}
}

//------------------------------------------------------------------------------------------------
//! Server: who is speaking, for a spoken line whose caller has no body to give (RZ_GameModeComponent.Say):
//! the nearest living character to the player that isn't a player or one of the infected, people
//! you can talk to (a clue or puzzle component) before anyone else. One small sphere query per line.
class RZ_SpeakerFinder
{
	protected vector m_vFrom;
	protected IEntity m_Best;
	protected float m_fBest;
	protected bool m_bBestTalks;

	//------------------------------------------------------------------------------------------------
	//! The speaker within `radius` of `from`, or null.
	static IEntity Nearest(vector from, float radius)
	{
		RZ_SpeakerFinder finder = new RZ_SpeakerFinder();
		return finder.Run(from, radius);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity Run(vector from, float radius)
	{
		m_vFrom = from;
		GetGame().GetWorld().QueryEntitiesBySphere(from, radius, OnEntity, null, EQueryEntitiesFlags.DYNAMIC);
		return m_Best;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (!ChimeraCharacter.Cast(e) || !RZ_Util.IsCharacterAlive(e) || RZ_BaconBody.IsBacon(e))
			return true;

		if (GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(e) > 0)
			return true;

		bool talks = e.FindComponent(RZ_ClueComponent) != null || e.FindComponent(RZ_PuzzleObjectComponent) != null;
		float d = RZ_Util.DistXZ(e.GetOrigin(), m_vFrom);
		if (m_Best && (m_bBestTalks && !talks))
			return true;

		if (!m_Best || (talks && !m_bBestTalks) || d < m_fBest)
		{
			m_Best = e;
			m_fBest = d;
			m_bBestTalks = talks;
		}

		return true;
	}
}
