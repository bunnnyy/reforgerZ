//! What each player's UI needs, worked out on the server and sent only to them (docs/UI.md,
//! "Data feeds"). Owned by RZ_GameModeComponent and run on its tick, after the infection.
//!
//! - Pushed on change: the infection timer (plus a resync every minute), the secure channel the
//!   player is listening to, a noise level 0-6 from the heat where they stand, and (with Stealth
//!   on) how visible they are, 0-5, from stealth's own update (RZ_Stealth, every second).
//! - Asked for by a menu as it opens: standing, the journal, the backgrounds, and by the map's
//!   icon layer: what this player knows the place of (one string each, one record per line,
//!   fields split by '|'; see BuildPayload).
//! - The gear each background loadout really spawns with (seen at spawns), for the deploy panel.
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
	int m_iNoise = -1;
	int m_iSeen = -1;		// visibility level last sent (-1: none yet, the client hides it)
	bool m_bSeenHidden;
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
	static const int PAYLOAD_BOARD = 4;		// the notice board the player stands at (World/RZ_NoticeBoard.c)
	static const int PAYLOAD_HALL = 5;		// the hall of fame (Persistence/RZ_HallOfFame.c)
	static const int PAYLOAD_KINDS = 6;		// the kinds a client may ask for; the ones below are only sent
	static const int PAYLOAD_TALK = 6;		// the talk screen: one node, or X|reason to close (Dialogue/RZ_Dialogue.c)
	static const int PAYLOAD_READER = 7;	// the reader: P|title|how, then the text; "" closes it (UI/RZ_Reader.c)
	static const int PAYLOAD_CAR = 8;		// the car repair screen: C, I, W, J records; F falls back; X|reason closes (Interactions/RZ_Cars.c)
	static const int PAYLOAD_SHOP = 9;		// the shop screen: S and I records; X|reason closes (Factions/RZ_ShopScreen.c)
	static const int PAYLOAD_DETAIL = 10;	// one journal row's detail, asked when it's selected (BuildDetail)
	static const int DETAIL_MAX = 1500;
	static const int ERRAND_ID_BASE = 100000;	// errand rows' ids: -(base + errand id)
	static const int TRIBE_ID_BASE = 200000;	// the packs' rows (what you know of them): -(base + pack id)
	static const int DOORS_ID = 300000;			// the one Read on doors row: -DOORS_ID (docs/TRACES.md)

	protected static const int MAX_PAYLOAD = 6000;
	protected static const int MAX_CLUES = 30;
	protected static const float JUMP_SECONDS = 5;	// the infection timer is re-sent when it jumps more than this

	protected RZ_GameModeComponent m_GM;
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
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendNewLife();

		// The notebook item, once the inventory exists
		if (!RZ_UISettings.Get().m_sNotebookPrefab.IsEmpty())
			GetGame().GetCallqueue().CallLater(GiveNotebook, 3000, false, playerId);
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
			PushNoise(ui, ent, st);
		}

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
	//! loadout carries, for the deploy panel's gear list.
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
	//! 0 quiet .. 6 very loud, from the heat where the player stands (the "loud" tip is at 45).
	protected void PushNoise(RZ_PlayerUIComponent ui, IEntity ent, RZ_UIFeedState st)
	{
		float heat = m_GM.GetHeat().GetHeat(ent.GetOrigin());
		int level = 0;
		if (heat >= 90)
			level = 6;
		else if (heat >= 70)
			level = 5;
		else if (heat >= 45)
			level = 4;
		else if (heat >= 30)
			level = 3;
		else if (heat >= 18)
			level = 2;
		else if (heat >= 8)
			level = 1;

		if (level == st.m_iNoise)
			return;

		st.m_iNoise = level;
		ui.SendNoise(level);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_Stealth's update: how visible this player is (0..1) and whether that counts as
	//! hidden. Sent as a level 0 (hidden) to 5 (in plain sight), only when it changes.
	void PushVisibility(int playerId, float vis, bool hidden)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!ui)
			return;

		int level = Math.ClampInt(RZ_Util.RoundToInt(vis * 5), 0, 5);
		RZ_UIFeedState st = State(playerId);
		if (level == st.m_iSeen && hidden == st.m_bSeenHidden)
			return;

		st.m_iSeen = level;
		st.m_bSeenHidden = hidden;
		ui.SendVisibility(level, hidden);
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
	protected string BuildHall()
	{
		RZ_HallOfFame hall = m_GM.GetHall();
		if (!hall)
			return "";

		return hall.BuildPayload();
	}

	//------------------------------------------------------------------------------------------------
	//! H|player name|background|life
	//! F|faction|fame|infamy|favor|tier|need|need line|rep (RZ_ERep)|notoriety|word|note|left to forgive (-1 no budget)|owed (0/1)
	//! The fields after the need line are new (docs/REPUTATION.md, "Reputations"): older layouts
	//! read the first eight and ignore the rest. word and owed come with reputation's later phases.
	//! L|faction|name|title|what they think of you
	//! R|kind (0 truce, 1 feud)|faction a|faction b|minutes left
	protected string BuildStanding(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		RZ_Background bg = m_GM.GetBackground(p.m_sBackgroundId);
		string bgName = "";
		if (bg)
			bgName = bg.m_sName;

		RZ_UILine head = new RZ_UILine("H");
		string s = head.Add(RZ_Util.PlayerName(playerId)).Add(bgName).AddInt(p.m_iLives).End();
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

			RZ_UILine lnF = new RZ_UILine("F");
			lnF.AddInt(f).AddInt(fame).AddInt(infamy).AddInt(p.GetFavor(ef)).AddInt(tier).AddInt(need).Add(needLine);
			lnF.AddInt(rep).AddInt(RZ_RepNames.Notoriety(fame, infamy)).Add("").Add(RZ_RepNames.Note(fame, infamy)).AddInt(left).AddInt(0);
			s += lnF.End();
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
	//! M|id|kind|giver (-1 none)|title|given by|where|direction and distance|minutes left|requirements|Fame|Favor|brief|taking part (0/1)
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
			lab = lnS.Add(m_GM.GetCure().Describe()).End();
		}

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		vector from = vector.Zero;
		if (ent)
			from = ent.GetOrigin();

		array<RZ_Mission> list = {};
		mm.ListFor(playerId, list);
		string s = lab;
		array<int> shown = {};
		foreach (RZ_Mission m : list)
		{
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

			// Contracts give a rough area until the player knows the spot (RZ_Mission)
			string dir = "";
			if (ent)
				dir = m.GetDirectionFor(m_GM, from, playerId);

			string req = "";
			if (m.m_bContract)
				req = m.m_Req.Describe();

			string part = "0";
			if (m.m_aParticipants.Contains(playerId))
				part = "1";

			RZ_UILine lnM = new RZ_UILine("M");
			lnM.AddInt(m.m_iId).AddInt(MissionKind(m)).AddInt(giver).Add(m.m_sTitle).Add(by).Add(m.GetLocationNameFor(m_GM, playerId, from)).Add(dir);
			lnM.AddInt(RZ_Util.RoundToInt(m.m_fTimeLeft / 60)).Add(req).AddInt(m.m_iFameReward).AddInt(m.m_iFavorReward).Add(m.GetBrief(m_GM)).Add(part);
			s += lnM.End();
			shown.Insert(m.m_iId);
		}

		s += BuildQuestlines(playerId, ent, from);
		s += BuildErrands(playerId, ent, from);
		s += BuildInfected(playerId, ent, from);
		s += BuildDoors(playerId);

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
			string dir = "";
			if (ent)
				dir = RZ_Util.DirectionText(from, e.m_vPlace);

			if (e.m_bStepDone && ent)
				dir = RZ_Util.DirectionText(from, e.m_vGiver);

			RZ_UILine lnE = new RZ_UILine("M");
			lnE.AddInt(-(ERRAND_ID_BASE + e.m_iId)).AddInt(RZ_EMissionKindUI.ERRAND).AddInt(e.m_iFaction).Add(e.Fill(e.m_Def.m_sTitle)).Add(e.m_sGiverTitle).Add(e.GetText("here")).Add(dir);
			lnE.AddInt(errands.MinutesLeft(e)).Add("Asked of you").AddInt(0).AddInt(0).Add(errands.JournalText(e)).Add("1");
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
				dir = RZ_Util.DirectionText(from, wheres[i]);

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
	//! One journal row's detail, sent when the row is selected (at most 1,500 characters, so the
	//! list's 6,000 is never cut for it): D|id|what they told you|what you've found|who it went to.
	string BuildDetail(int playerId, int id)
	{
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
		else
		{
			return "";
		}

		RZ_UILine ln = new RZ_UILine("D");
		string payload = ln.AddInt(id).Add(told).Add(found.Trim()).Add(went).End();
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

			string where = run.m_sPlaceName;
			string dir = "";
			if (run.m_bHasPlace && ent)
				dir = RZ_Util.DirectionText(from, run.m_vPlace);

			string part = "0";
			if (run.PlayerTookPart(m_GM, playerId))
				part = "1";

			int id = -run.m_iId;
			RZ_UILine lnQ = new RZ_UILine("M");
			lnQ.AddInt(id).AddInt(RZ_EMissionKindUI.QUESTLINE).AddInt(run.m_Line.m_iFaction).Add(run.m_Line.m_sTitle).Add(stageTitle).Add(where).Add(dir);
			lnQ.AddInt(-1).Add("Whoever takes the next step").AddInt(0).AddInt(0).Add(q.JournalFor(run)).Add(part);
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

			s += lnB.AddInt(bg.GetStartRep(RZ_EFaction.BANDITS)).AddInt(bg.GetStartRep(RZ_EFaction.INSTITUTE)).Add(gear).End();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! What this player knows the place of, for the map's icon layer (UI/RZ_MapIcons.c). Contracts
	//! are never on it: they give a rough area, in the journal. The packs they know of are range rings
	//! sized to the ground (a radius in metres as the last field).
	//! P|icon|faction (-1 none)|x|z|label[|radius]
	protected string BuildMap(int playerId)
	{
		string s = "";
		if (m_GM.HasLab())
			s += MapEntry("marker_lab", RZ_EFaction.INSTITUTE, m_GM.GetLabPosition(), "Halcyon lab");

		// Safehouses: everyone knows them, except the people the Haven turned out
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && !RZ_Reputation.IsHunted(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS)))
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				s += MapEntry("marker_safehouse", RZ_EFaction.SURVIVORS, h.m_vPos, string.Format("Safehouse, %1 (%2)", h.m_sName, RZ_Safehouse.TierName(h.GetTier())));
			}
		}

		// Strongholds are common knowledge; an outpost only once they know the faction or have been there
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			array<int> seen;
			m_mVisited.Find(m_GM.GetProfile(playerId).m_sIdentity, seen);
			foreach (RZ_Holding hold : territory.GetHoldings())
			{
				if (hold.m_bStronghold)
				{
					s += MapEntry("marker_stronghold", hold.m_eFaction, hold.m_vPos, hold.GetTitle());
					continue;
				}

				bool known = RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, hold.m_eFaction), RZ_ETier.KNOWN);
				if (known || (seen && seen.Contains(hold.m_iId)))
					s += MapEntry("marker_outpost", hold.m_eFaction, hold.m_vPos, hold.GetTitle());
			}
		}

		RZ_PowerGrid grid = m_GM.GetPowerGrid();
		if (grid && grid.IsActive())
			s += MapEntry("marker_power", -1, grid.GetPosition(), grid.GetName());

		RZ_RadioStation radio = m_GM.GetRadioStation();
		if (radio && radio.IsActive())
			s += MapEntry("marker_radio", -1, radio.GetPosition(), radio.GetName());

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (perimeter)
		{
			array<vector> checkpoints = {};
			perimeter.GetCheckpoints(checkpoints);
			foreach (vector cp : checkpoints)
			{
				s += MapEntry("marker_checkpoint", -1, cp, "Sealed checkpoint");
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

		// World events are public; contracts aren't on the map at all
		RZ_MissionManager mm = m_GM.GetMissions();
		if (mm)
		{
			foreach (RZ_Mission m : mm.GetActive())
			{
				if (!m.m_bContract && !m.m_bGlobal)
					s += MapEntry("marker_event", -1, m.m_vPos, m.m_sTitle);
			}
		}

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
