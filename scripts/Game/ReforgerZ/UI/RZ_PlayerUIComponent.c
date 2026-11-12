[ComponentEditorProps(category: "ReforgerZ", description: "ReforgerZ UI feed: the server sends this player's infection timer, secure-radio state, noise, visibility, standing, journal and backgrounds to the owning client only. Add to the player controller prefab (UI/LAYOUTS.md).")]
class RZ_PlayerUIComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! One radio log entry (client side, this life only).
class RZ_RadioEntry
{
	string m_sTime;
	int m_iKind;
	int m_iFaction;
	string m_sTitle;
	string m_sText;
}

//------------------------------------------------------------------------------------------------
//! The data pipe for the UI, on the player controller (docs/UI.md, "Data feeds").
//!
//! Server to client: owner-only RPCs, sent by RZ_UIFeed only when something changed (the infection
//! timer also resyncs every minute; the client counts down between). Client to server: the
//! menus ask for their data when they open (at most once every 2 s per player), and chat commands
//! and the board menu send a command (/note, /noteclear; each at most every 2 s, and a
//! dropped one gets a short notice).
//! On a listen server the owner is this machine, so calls go straight to the handler.
class RZ_PlayerUIComponent : ScriptComponent
{
	protected static const float ASK_GAP = 2;

	// ---------------------------------------------------------------- client cache
	protected int m_iInfState = RZ_EInfectionUI.NONE;
	protected float m_fInfLeft;
	protected float m_fInfTotal;
	protected float m_fInfFinal;
	protected float m_fInfPause;
	protected float m_fInfStamp;
	protected int m_iListenFaction = -1;
	protected float m_fListenLeft;
	protected float m_fListenStamp;
	protected int m_iNoise;
	protected string m_sStand;			// the stand's HUD line (client)
	protected string m_sLook;			// borrowed kit's look-coming line (client), "" hidden
	protected float m_fStandUntil;
	protected int m_iSeen = -1;			// visibility 0 (hidden) to 5; -1 = none (Stealth off): not shown
	protected bool m_bSeenHidden;
	protected string m_sStanding;
	protected string m_sJournal;
	protected string m_sDetail;			// the selected journal row's detail (RZ_UIFeed.BuildDetail)
	protected int m_iDetailVersion;
	protected int m_iDetailAsked;		// the row id last asked for
	protected float m_fDetailAskAt = -100;
	protected string m_sBackgrounds;
	protected string m_sMap;
	protected string m_sBoard;
	protected string m_sHall;
	protected string m_sSeason;
	protected int m_iSeasonVersion;
	protected float m_fSeasonAsk = -100;
	protected int m_iBoardVersion;
	protected int m_iHallVersion;
	protected int m_iMapVersion;
	protected ref set<string> m_aLockedLoadouts = new set<string>();
	protected int m_iStandingVersion;
	protected int m_iJournalVersion;
	protected int m_iBackgroundsVersion;
	protected int m_iRadioVersion;
	protected ref array<ref RZ_RadioEntry> m_aRadio = {};
	protected float m_fNewLifeAt = -1;
	protected int m_iLogged;			// notices kept for the Radio tab this life
	protected int m_iLoggedSeen;
	protected int m_iMissions;			// mission notices this life (the Journal tab's count)
	protected int m_iMissionsSeen;
	protected string m_sReader;
	protected int m_iReaderVersion;
	protected string m_sTracker;		// the HUD's quest tracker (RZ_QuestTracker), "" hidden
	protected int m_iTrackerVersion;
	protected float m_fTrackAskAt = -100;	// server: the last Track asked (a spam guard)

	// ---------------------------------------------------------------- server
	protected ref array<float> m_aLastAsk = {-100, -100, -100, -100, -100, -100};
	protected ref array<float> m_aLastCommand = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};	// per CMD_*
	protected float m_fLastPick = -100;		// the last car job or purchase (PICK_GAP)
	protected static const float PICK_GAP = 0.5;

	//! Commands from the client (RZ_ChatCommands, the board menu)
	static const int CMD_NOTE = 0;
	static const int CMD_CLEAR_BOARD = 1;
	static const int CMD_CLEAR_ALL = 2;
	static const int CMD_REP = 3;		// "/rep <name>": an admin's reputation read-out (Factions/RZ_RepNames.c)
	static const int CMD_TRIBES = 4;	// /tribes (admins): the infected, to the server log
	static const int CMD_TALKDEBUG = 5;	// /talkdebug (admins): the conversation trees for the person you're talking to
	static const int CMD_MYDEAD = 6;	// /mydead hidden|shown: your name in public text if you die out there (also the notebook item's action)
	static const int CMD_EPILOGUE = 7;	// /epilogue ... (admins): the epilogue week's tools (Director/Epilogue/RZ_Epilogue.c)
	static const int CMD_SEASON = 8;	// /season: the journal's season view as a notice (also the notebook item's action)
	static const int CMD_ANON = 9;		// /anonymous [off]: keep your name out of the world's text, the reel and the hall
	static const int CMD_LASTENTRY = 10;	// /lastentry <text>: your line on the Names board (a pad writes one without typing)
	static const int CMD_TRACES = 11;	// /traces (admins): the door marks near you, to the server log (World/Traces)

	//------------------------------------------------------------------------------------------------
	//! The local player's component (client), or null.
	static RZ_PlayerUIComponent GetLocal()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;

		return RZ_PlayerUIComponent.Cast(pc.FindComponent(RZ_PlayerUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! A player's component (server), or null.
	static RZ_PlayerUIComponent GetFor(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return null;

		return RZ_PlayerUIComponent.Cast(pc.FindComponent(RZ_PlayerUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! True when this controller belongs to this machine (a listen server's own player).
	protected bool IsLocalOwner()
	{
		return GetOwner() == GetGame().GetPlayerController();
	}

	//------------------------------------------------------------------------------------------------
	protected int OwnerPlayerId()
	{
		PlayerController pc = PlayerController.Cast(GetOwner());
		if (!pc)
			return -1;

		return pc.GetPlayerId();
	}

	// ================================================================ Server: send
	//------------------------------------------------------------------------------------------------
	void SendInfection(int state, float left, float total, float finalSeconds, float pauseLeft)
	{
		if (IsLocalOwner())
			RpcDo_Infection(state, left, total, finalSeconds, pauseLeft);
		else
			Rpc(RpcDo_Infection, state, left, total, finalSeconds, pauseLeft);
	}

	//------------------------------------------------------------------------------------------------
	void SendListening(int faction, float secondsLeft)
	{
		if (IsLocalOwner())
			RpcDo_Listening(faction, secondsLeft);
		else
			Rpc(RpcDo_Listening, faction, secondsLeft);
	}

	//------------------------------------------------------------------------------------------------
	void SendNoise(int level)
	{
		if (IsLocalOwner())
			RpcDo_Noise(level);
		else
			Rpc(RpcDo_Noise, level);
	}

	//------------------------------------------------------------------------------------------------
	void SendNewLife()
	{
		if (IsLocalOwner())
			RpcDo_NewLife();
		else
			Rpc(RpcDo_NewLife);
	}

	//------------------------------------------------------------------------------------------------
	//! kind: RZ_UIFeed.PAYLOAD_* (standing, journal, backgrounds, map, notice board, hall of fame)
	void SendPayload(int kind, string payload)
	{
		if (IsLocalOwner())
			RpcDo_Payload(kind, payload);
		else
			Rpc(RpcDo_Payload, kind, payload); // VERIFY: a string RPC argument of a few KB arrives whole (RZ_UIFeed caps payloads at 6000 characters)
	}

	//------------------------------------------------------------------------------------------------
	//! A notice for this player only (RZ_GameModeComponent.NotifyPlayerAs). tag: RZ_ENoticeTag.
	void SendNotice(string title, string text, int tag)
	{
		if (IsLocalOwner())
			RpcDo_Notice(title, text, tag);
		else
			Rpc(RpcDo_Notice, title, text, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! A plain spoken line for this player only: a subtitle, the client working out the rest from
	//! the title (RZ_SayLine.Make). Kept for callers without a line; RZ_GameModeComponent.Say uses
	//! SendSayLine.
	void SendSay(string speaker, int faction, string text)
	{
		if (IsLocalOwner())
			RpcDo_Say(speaker, faction, text);
		else
			Rpc(RpcDo_Say, speaker, faction, text);
	}

	//------------------------------------------------------------------------------------------------
	//! A spoken line for this player only, with who says it and how it reaches them (RZ_SayLine):
	//! "name|role|channel", the faction, the source (RZ_ESaySource), where the speaker stood
	//! (vector.Zero: nowhere) and their body's RplId (invalid: none), then the line.
	void SendSayLine(RZ_SayLine line)
	{
		if (IsLocalOwner())
			RpcDo_SayLine(line.Head(), line.m_iFaction, line.m_iSource, line.m_vAt, line.m_Who, line.m_sText);
		else
			Rpc(RpcDo_SayLine, line.Head(), line.m_iFaction, line.m_iSource, line.m_vAt, line.m_Who, line.m_sText);
	}

	//------------------------------------------------------------------------------------------------
	//! The stand's line under the noise meter (RZ_CureFinale), shown for `seconds` unless sent again.
	//! Names routes, never players. "" clears it.
	void SendStand(string line, float seconds)
	{
		if (IsLocalOwner())
			RpcDo_Stand(line, seconds);
		else
			Rpc(RpcDo_Stand, line, seconds);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Stand(string line, float seconds)
	{
		m_sStand = line;
		m_fStandUntil = RZ_UIW.Now() + seconds;
	}

	//------------------------------------------------------------------------------------------------
	//! Borrowed kit's look-coming line, RZ_Hud_Look (RZ_UIFeed.PushLook): "PASS AS BASTION: 2 OF 2",
	//! held until the server sends "" (the look done, the kit off, 200 m on).
	void SendLook(string line)
	{
		if (IsLocalOwner())
			RpcDo_Look(line);
		else
			Rpc(RpcDo_Look, line);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Look(string line)
	{
		m_sLook = line;
	}

	//------------------------------------------------------------------------------------------------
	string GetLookLine() { return m_sLook; }

	//------------------------------------------------------------------------------------------------
	//! The stand's line while it's fresh, else "".
	string GetStandLine()
	{
		if (RZ_UIW.Now() > m_fStandUntil)
			return "";

		return m_sStand;
	}

	//------------------------------------------------------------------------------------------------
	//! level: 0 hidden to 5 in plain sight (RZ_UIFeed.PushVisibility); hidden: under Stealth > Hidden below.
	void SendVisibility(int level, bool hidden)
	{
		if (IsLocalOwner())
			RpcDo_Visibility(level, hidden);
		else
			Rpc(RpcDo_Visibility, level, hidden);
	}

	// ================================================================ Client: receive
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Infection(int state, float left, float total, float finalSeconds, float pauseLeft)
	{
		m_iInfState = state;
		m_fInfLeft = left;
		m_fInfTotal = total;
		m_fInfFinal = finalSeconds;
		m_fInfPause = pauseLeft;
		m_fInfStamp = RZ_UIW.Now();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Listening(int faction, float secondsLeft)
	{
		m_iListenFaction = faction;
		m_fListenLeft = secondsLeft;
		m_fListenStamp = RZ_UIW.Now();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Noise(int level)
	{
		m_iNoise = level;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Visibility(int level, bool hidden)
	{
		m_iSeen = level;
		m_bSeenHidden = hidden;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Notice(string title, string text, int tag)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
			gm.ShowNotice(title, text, tag);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Say(string speaker, int faction, string text)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
			gm.ShowSay(speaker, faction, text);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SayLine(string head, int faction, int source, vector at, RplId who, string text)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
			gm.ShowSayLine(RZ_SayLine.FromWire(head, faction, source, at, who, text));
	}

	//------------------------------------------------------------------------------------------------
	//! A new life: the radio log and the timers belong to the last one.
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NewLife()
	{
		m_fNewLifeAt = RZ_UIW.Now();
		m_aRadio.Clear();
		m_iLogged = 0;
		m_iLoggedSeen = 0;
		m_iMissions = 0;
		m_iMissionsSeen = 0;
		m_iRadioVersion++;
		m_iInfState = RZ_EInfectionUI.NONE;
		m_iListenFaction = -1;
		m_iNoise = 0;
		m_iSeen = -1;
		m_bSeenHidden = false;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Payload(int kind, string payload)
	{
		if (kind == RZ_UIFeed.PAYLOAD_STANDING)
		{
			m_sStanding = payload;
			m_iStandingVersion++;
		}
		else if (kind == RZ_UIFeed.PAYLOAD_JOURNAL)
		{
			m_sJournal = payload;
			m_iJournalVersion++;
		}
		else if (kind == RZ_UIFeed.PAYLOAD_MAP)
		{
			m_sMap = payload;
			m_iMapVersion++;
		}
		else if (kind == RZ_UIFeed.PAYLOAD_BOARD)
		{
			// The server sends the board when it's read: open the board menu if it isn't open
			m_sBoard = payload;
			m_iBoardVersion++;
			if (!payload.IsEmpty() && !RZ_NoticeBoardMenu.IsBoardOpen())
				RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_NoticeBoardMenu);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_HALL)
		{
			m_sHall = payload;
			m_iHallVersion++;
		}
		else if (kind == RZ_UIFeed.PAYLOAD_SEASON)
		{
			m_sSeason = payload;
			m_iSeasonVersion++;
		}
		else if (kind == RZ_UIFeed.PAYLOAD_READER)
		{
			OnReader(payload);
		}
		else if (kind == RZ_DeathBrief.PAYLOAD)
		{
			RZ_DeathBrief.OnPayload(payload);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_TALK)
		{
			RZ_TalkMenu.OnPayload(this, payload);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_CAR)
		{
			RZ_CarMenu.OnPayload(this, payload);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_SHOP)
		{
			RZ_ShopMenu.OnPayload(this, payload);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_CRAFT)
		{
			RZ_CraftMenu.OnPayload(this, payload, false);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_COOK)
		{
			RZ_CraftMenu.OnPayload(this, payload, true);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_RADIO)
		{
			RZ_RadioMenu.OnPayload(this, payload);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_DETAIL)
		{
			m_sDetail = payload;
			m_iDetailVersion++;
		}
		else if (kind == RZ_UIFeed.PAYLOAD_OBJECT)
		{
			RZ_ObjectMenu.OnPayload(this, payload);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_NAMETAGS)
		{
			RZ_NametagClient.OnPayload(payload);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_POWER)
		{
			RZ_PowerMenu.OnPayload(this, payload);
		}
		else if (kind == RZ_UIFeed.PAYLOAD_TRACKER)
		{
			m_sTracker = payload;
			m_iTrackerVersion++;
		}
		else if (kind == RZ_UIFeed.PAYLOAD_BACKGROUNDS)
		{
			m_sBackgrounds = payload;
			m_iBackgroundsVersion++;
			ReadLockedLoadouts();
		}

		// Any other kind (a newer server's) is ignored
	}

	//------------------------------------------------------------------------------------------------
	//! The reader: open it on a new paper; "" from the server closes it (the screen guard). If the
	//! reader can't open here (no preset or layout), the paper is a notice card instead.
	protected void OnReader(string payload)
	{
		if (payload.IsEmpty())
		{
			RZ_ReaderMenu.CloseFromServer();
			return;
		}

		m_sReader = payload;
		m_iReaderVersion++;
		if (RZ_ReaderMenu.IsShowing())
			return;

		if (RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_ReaderMenu))
			return;

		SendScreenClosed(RZ_ScreenGuard.KIND_READER);
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int nl = payload.IndexOf("\n");
		if (!gm || nl < 0)
			return;

		array<string> f = {};
		RZ_UIText.Split(payload.Substring(0, nl), RZ_UIText.SEP, f);
		gm.ShowNotice(RZ_UIText.Str(f, 1), payload.Substring(nl + 1, payload.Length() - nl - 1), RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	//! B records with the locked flag set (field 6), by loadout name (field 3).
	protected void ReadLockedLoadouts()
	{
		m_aLockedLoadouts.Clear();
		array<string> lines = {};
		RZ_UIText.Split(m_sBackgrounds, "\n", lines);
		foreach (string ln : lines)
		{
			array<string> f = {};
			RZ_UIText.Split(ln, RZ_UIText.SEP, f);
			if (RZ_UIText.Str(f, 0) == "B" && RZ_UIText.Int(f, 6) == 1)
				m_aLockedLoadouts.Insert(RZ_UIText.Str(f, 3));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Client: a background loadout this player hasn't unlocked, as far as the last backgrounds
	//! payload says (UI/RZ_LoadoutLock.c). False until it has arrived; the server checks anyway.
	bool IsLoadoutLocked(string loadoutName)
	{
		return m_aLockedLoadouts.Contains(loadoutName);
	}

	// ================================================================ Client: ask
	//------------------------------------------------------------------------------------------------
	//! The journal's selected row: its detail is asked for once per selection.
	void AskDetail(int id)
	{
		if (id == m_iDetailAsked)
			return;

		m_iDetailAsked = id;
		m_sDetail = "";
		if (Replication.IsServer())
			RpcAsk_Detail(id);
		else
			Rpc(RpcAsk_Detail, id);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Detail(int id)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetUIFeed())
			return;

		// At most a few a second: scrolling a list asks for each row it stops on
		float now = RZ_UIW.Now();
		if (now - m_fDetailAskAt < 0.25)
			return;

		m_fDetailAskAt = now;
		int playerId = OwnerPlayerId();
		if (playerId > 0)
			SendPayload(RZ_UIFeed.PAYLOAD_DETAIL, gm.GetUIFeed().BuildDetail(playerId, id));
	}

	//------------------------------------------------------------------------------------------------
	//! Track in the Quests tab: follow this job, or stop following it if it's the one followed
	//! (docs/UI.md, "Notebook: Quests"). The server answers with the tracker and the journal.
	void AskTrack(int id)
	{
		if (Replication.IsServer())
			RpcAsk_Track(id);
		else
			Rpc(RpcAsk_Track, id);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Track(int id)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetUIFeed())
			return;

		float now = RZ_UIW.Now();
		if (now - m_fTrackAskAt < 0.3)
			return;

		m_fTrackAskAt = now;
		int playerId = OwnerPlayerId();
		if (playerId <= 0)
			return;

		gm.GetUIFeed().GetTracker().Toggle(playerId, id);
		SendPayload(RZ_UIFeed.PAYLOAD_JOURNAL, gm.GetUIFeed().BuildPayload(playerId, RZ_UIFeed.PAYLOAD_JOURNAL));
	}

	//------------------------------------------------------------------------------------------------
	//! kind: RZ_UIFeed.PAYLOAD_*
	void Ask(int kind)
	{
		if (Replication.IsServer())
			RpcAsk_Payload(kind);
		else
			Rpc(RpcAsk_Payload, kind);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Payload(int kind)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetUIFeed())
			return;

		float now = RZ_UIW.Now();
		// The season tab asks with its own kind (after the ones only sent)
		if (kind == RZ_UIFeed.PAYLOAD_SEASON)
		{
			if (now - m_fSeasonAsk < ASK_GAP)
				return;

			m_fSeasonAsk = now;
			int asker = OwnerPlayerId();
			if (asker > 0)
				SendPayload(kind, gm.GetUIFeed().BuildPayload(asker, kind));

			return;
		}

		// A cheap spam guard per kind: the menus ask once when they open, the map every 20 s
		if (kind < 0 || kind >= RZ_UIFeed.PAYLOAD_KINDS)
			return;

		if (now - m_aLastAsk[kind] < ASK_GAP)
			return;

		m_aLastAsk[kind] = now;
		int playerId = OwnerPlayerId();
		if (playerId <= 0)
			return;

		SendPayload(kind, gm.GetUIFeed().BuildPayload(playerId, kind));
	}

	//------------------------------------------------------------------------------------------------
	//! The player closed the reader or the talk screen (kind: RZ_ScreenGuard.KIND_*).
	void SendScreenClosed(int kind)
	{
		if (Replication.IsServer())
			RpcAsk_ScreenClosed(kind);
		else
			Rpc(RpcAsk_ScreenClosed, kind);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ScreenClosed(int kind)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = OwnerPlayerId();
		if (!gm || playerId <= 0 || !gm.GetScreenGuard())
			return;

		gm.GetScreenGuard().OnClientClosed(playerId, kind);
	}

	//------------------------------------------------------------------------------------------------
	//! A reply in the talk screen, or Leave (RZ_Dialogue.LEAVE), for the node with this serial.
	void SendTalkPick(int serial, int replyId)
	{
		if (Replication.IsServer())
			RpcAsk_TalkPick(serial, replyId);
		else
			Rpc(RpcAsk_TalkPick, serial, replyId);
	}

	//------------------------------------------------------------------------------------------------
	//! Checked on the server (RZ_Dialogue.Pick): the serial is current, the id was sent and enabled.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_TalkPick(int serial, int replyId)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = OwnerPlayerId();
		if (!gm || playerId <= 0 || !gm.GetDialogue())
			return;

		gm.GetDialogue().Pick(playerId, serial, replyId);
	}

	//------------------------------------------------------------------------------------------------
	//! A job on the car repair screen (RZ_Cars.JOB_*) for the screen with this serial.
	void SendCarJob(int serial, int job)
	{
		if (Replication.IsServer())
			RpcAsk_CarJob(serial, job);
		else
			Rpc(RpcAsk_CarJob, serial, job);
	}

	//------------------------------------------------------------------------------------------------
	//! Checked on the server (RZ_Cars.DoJob): the screen is open, the serial current, the car there
	//! and the player beside it; then the same checks as the car's actions.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CarJob(int serial, int job)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = OwnerPlayerId();
		if (!gm || playerId <= 0 || !gm.GetInteractions() || !PickAllowed())
			return;

		gm.GetInteractions().GetCars().DoJob(playerId, serial, job);
	}

	//------------------------------------------------------------------------------------------------
	//! Buy (or ask for) row `index` of the shop screen with this serial.
	void SendShopBuy(int serial, int index)
	{
		if (Replication.IsServer())
			RpcAsk_ShopBuy(serial, index);
		else
			Rpc(RpcAsk_ShopBuy, serial, index);
	}

	//------------------------------------------------------------------------------------------------
	//! Checked on the server (RZ_ShopScreen.Buy): the screen is open, the serial current, the
	//! quartermaster there; then the same checks as the Buy action (standing, Favor, stock).
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ShopBuy(int serial, int index)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = OwnerPlayerId();
		if (!gm || playerId <= 0 || !PickAllowed())
			return;

		RZ_ShopScreen.Buy(gm, playerId, serial, index);
	}

	//------------------------------------------------------------------------------------------------
	//! Recipe `index` on the crafting screen (or the cooking screen) with this serial.
	void SendCraft(int serial, int index)
	{
		if (Replication.IsServer())
			RpcAsk_Craft(serial, index);
		else
			Rpc(RpcAsk_Craft, serial, index);
	}

	//------------------------------------------------------------------------------------------------
	//! Checked on the server (RZ_Crafting.DoCraft): the screen is open, the serial current, the
	//! workbench there and the player beside it; then the same checks as the bench's actions.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Craft(int serial, int index)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = OwnerPlayerId();
		if (!gm || playerId <= 0 || !gm.GetInteractions() || !PickAllowed())
			return;

		gm.GetInteractions().GetCrafting().DoCraft(playerId, serial, index);
	}

	//------------------------------------------------------------------------------------------------
	//! Row `index` of the object menu with this serial.
	void SendObjectPick(int serial, int index)
	{
		if (Replication.IsServer())
			RpcAsk_ObjectPick(serial, index);
		else
			Rpc(RpcAsk_ObjectPick, serial, index);
	}

	//------------------------------------------------------------------------------------------------
	//! Checked on the server (RZ_ObjectScreen.Pick): the menu is open, the serial current, the thing
	//! there and the player beside it; then that row's own action code.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ObjectPick(int serial, int index)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = OwnerPlayerId();
		if (!gm || playerId <= 0 || !PickAllowed())
			return;

		RZ_ObjectScreen.Pick(gm, playerId, serial, index);
	}

	//------------------------------------------------------------------------------------------------
	//! A move on the power panel with this serial: a breaker, a button, or that it couldn't open
	//! (RZ_PowerPanel.MOVE_*).
	void SendPowerMove(int serial, int move)
	{
		if (Replication.IsServer())
			RpcAsk_PowerMove(serial, move);
		else
			Rpc(RpcAsk_PowerMove, serial, move);
	}

	//------------------------------------------------------------------------------------------------
	//! Checked on the server (RZ_PowerPanel.Move): the panel is open on this serial, the control there
	//! and the player beside it. "Couldn't open" isn't rate-limited.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PowerMove(int serial, int move)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = OwnerPlayerId();
		if (!gm || playerId <= 0 || !gm.GetPowerGrid() || !gm.GetPowerGrid().GetPanel())
			return;

		if (move != RZ_PowerPanel.MOVE_FAILED && !PickAllowed())
			return;

		gm.GetPowerGrid().GetPanel().Move(playerId, serial, move);
	}

	//------------------------------------------------------------------------------------------------
	//! This client couldn't open the object menu (no preset or layout): world prompts from now on.
	void SendObjectFailed()
	{
		if (Replication.IsServer())
			RpcAsk_ObjectFailed();
		else
			Rpc(RpcAsk_ObjectFailed);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ObjectFailed()
	{
		int playerId = OwnerPlayerId();
		if (playerId > 0)
			RZ_ObjectScreen.OnClientFailed(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: a car job, a purchase or a recipe at most every half second (a held button can't flood it).
	protected bool PickAllowed()
	{
		float now = RZ_UIW.Now();
		if (now - m_fLastPick < PICK_GAP)
			return false;

		m_fLastPick = now;
		return true;
	}

	// ================================================================ Client: commands
	//------------------------------------------------------------------------------------------------
	//! A command for the server: CMD_NOTE with the note's text, the board clears (admins).
	void SendCommand(int cmd, string text)
	{
		if (Replication.IsServer())
			RpcAsk_Command(cmd, text);
		else
			Rpc(RpcAsk_Command, cmd, text);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Command(int cmd, string text)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return;

		int playerId = OwnerPlayerId();
		if (playerId <= 0 || cmd < 0 || cmd >= m_aLastCommand.Count())
			return;

		// Each command at most every 2 s, and the player hears why one was dropped
		float now = RZ_UIW.Now();
		if (now - m_aLastCommand[cmd] < ASK_GAP)
		{
			gm.NotifyPlayer(playerId, "Wait", "One moment: that was just done. Try again in a second or two.");
			return;
		}

		m_aLastCommand[cmd] = now;

		// Notes are short; anything longer is cut on the server whatever the client sent
		string note = text;
		if (note.Length() > 400)
			note = note.Substring(0, 400);

		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_NoticeBoard board;
		if (houses)
			board = houses.GetBoard();

		switch (cmd)
		{
			case CMD_NOTE:
			{
				if (board)
					board.Post(playerId, note);
				break;
			}
			case CMD_CLEAR_BOARD:
			{
				if (board)
					board.Clear(playerId, false);
				break;
			}
			case CMD_CLEAR_ALL:
			{
				if (board)
					board.Clear(playerId, true);
				break;
			}
			case CMD_REP:
			{
				if (gm.GetRepWatch())
					gm.GetRepWatch().Readout(playerId, note);
				break;
			}
			case CMD_TALKDEBUG:
			{
				RZ_Dialogue dialogue = gm.GetDialogue();
				if (dialogue)
					dialogue.GetTrees().Command(playerId, dialogue.GetSession(playerId), note);
				break;
			}
			case CMD_TRIBES:
			{
				if (gm.GetTribes())
					gm.GetTribes().Command(playerId, note);
				break;
			}
			case CMD_EPILOGUE:
			{
				if (gm.GetEpilogue())
					gm.GetEpilogue().Command(playerId, note);
				break;
			}
			case CMD_SEASON:
			{
				if (gm.GetEpilogue())
					gm.NotifyPlayer(playerId, "The season", gm.GetEpilogue().SeasonText(playerId));
				break;
			}
			case CMD_ANON:
			{
				int anon = 1;
				if (note.Contains("off"))
					anon = 0;

				RZ_EpiActions.Anonymous(gm, playerId, anon);
				break;
			}
			case CMD_LASTENTRY:
			{
				if (gm.GetEpilogue())
					RZ_EpiActions.LastEntry(gm, playerId, note);
				break;
			}
			case CMD_TRACES:
			{
				if (gm.GetTraces())
					gm.GetTraces().Command(playerId, note);
				break;
			}
			case CMD_MYDEAD:
			{
				int hidden = -1;
				if (note.Contains("hid"))
					hidden = 1;
				else if (note.Contains("show") || note.Contains("name"))
					hidden = 0;

				if (gm.GetTribes())
					gm.GetTribes().GetPeople().SetHidden(playerId, hidden);
				break;
			}
		}
	}

	// ================================================================ Client: the Radio tab
	//------------------------------------------------------------------------------------------------
	//! Every notice passes through here on the client; radio, mission, world-event and world ones
	//! are kept for the notebook's Radio tab, newest first.
	void OnNotice(string title, string text, RZ_NoticeStyle style)
	{
		if (!style || !style.IsLogged())
			return;

		RZ_RadioEntry e = new RZ_RadioEntry();
		e.m_sTime = RZ_UIText.TimeOfDay();
		e.m_iKind = style.m_iKind;
		e.m_iFaction = style.m_iFaction;
		e.m_sTitle = title;
		e.m_sText = text;
		m_aRadio.InsertAt(e, 0);
		m_iLogged++;
		if (style.m_iKind == RZ_ENoticeKind.MISSION)
			m_iMissions++;

		int cap = Math.Max(5, RZ_UISettings.Get().m_iRadioLogSize);
		while (m_aRadio.Count() > cap)
		{
			m_aRadio.Remove(m_aRadio.Count() - 1);
		}

		m_iRadioVersion++;
	}

	// ================================================================ Client: getters
	//------------------------------------------------------------------------------------------------
	int GetInfectionState() { return m_iInfState; }
	float GetInfectionTotal() { return m_fInfTotal; }
	float GetInfectionFinal() { return m_fInfFinal; }

	//------------------------------------------------------------------------------------------------
	//! Seconds left now: the timer stands still while paused (the pause counts down instead).
	float GetInfectionLeft()
	{
		float since = RZ_UIW.Now() - m_fInfStamp;
		if (m_fInfPause > 0)
			return Math.Max(0, m_fInfLeft - Math.Max(0, since - m_fInfPause));

		return Math.Max(0, m_fInfLeft - since);
	}

	//------------------------------------------------------------------------------------------------
	float GetPauseLeft()
	{
		return Math.Max(0, m_fInfPause - (RZ_UIW.Now() - m_fInfStamp));
	}

	//------------------------------------------------------------------------------------------------
	int GetListenFaction() { return m_iListenFaction; }

	//------------------------------------------------------------------------------------------------
	float GetListenLeft()
	{
		return Math.Max(0, m_fListenLeft - (RZ_UIW.Now() - m_fListenStamp));
	}

	//------------------------------------------------------------------------------------------------
	int GetNoise() { return m_iNoise; }
	//! Visibility 0 (hidden) to 5, or -1: none sent (Stealth off, or not yet this life).
	int GetSeen() { return m_iSeen; }
	bool IsSeenHidden() { return m_bSeenHidden; }
	string GetStanding() { return m_sStanding; }
	string GetJournal() { return m_sJournal; }
	string GetTracker() { return m_sTracker; }
	int GetTrackerVersion() { return m_iTrackerVersion; }
	string GetDetail() { return m_sDetail; }
	int GetDetailVersion() { return m_iDetailVersion; }
	string GetBackgrounds() { return m_sBackgrounds; }
	string GetMap() { return m_sMap; }
	string GetBoard() { return m_sBoard; }
	string GetHall() { return m_sHall; }
	string GetSeason() { return m_sSeason; }
	int GetSeasonVersion() { return m_iSeasonVersion; }
	string GetReader() { return m_sReader; }
	int GetReaderVersion() { return m_iReaderVersion; }
	int GetBoardVersion() { return m_iBoardVersion; }
	int GetHallVersion() { return m_iHallVersion; }
	int GetMapVersion() { return m_iMapVersion; }
	int GetStandingVersion() { return m_iStandingVersion; }
	int GetJournalVersion() { return m_iJournalVersion; }
	int GetBackgroundsVersion() { return m_iBackgroundsVersion; }
	int GetRadioVersion() { return m_iRadioVersion; }
	//! When this life began on this machine (RZ_UIW.Now), -1 before the first.
	float GetNewLifeAt() { return m_fNewLifeAt; }
	//! The notebook's counts: new since the tab was last looked at (this life).
	int GetUnreadRadio() { return m_iLogged - m_iLoggedSeen; }
	int GetUnreadJournal() { return m_iMissions - m_iMissionsSeen; }
	void MarkRadioSeen() { m_iLoggedSeen = m_iLogged; }
	void MarkJournalSeen() { m_iMissionsSeen = m_iMissions; }
	array<ref RZ_RadioEntry> GetRadioLog() { return m_aRadio; }
}
