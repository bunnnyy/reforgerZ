[ComponentEditorProps(category: "ReforgerZ", description: "ReforgerZ UI feed: the server sends this player's infection timer, secure-radio state, noise, standing, journal and backgrounds to the owning client only. Add to the player controller prefab (UI/LAYOUTS.md).")]
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
//! and the board menu send a command (/note, /noteclear, /hall; each at most every 2 s, and a
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
	protected string m_sStanding;
	protected string m_sJournal;
	protected string m_sBackgrounds;
	protected string m_sMap;
	protected string m_sBoard;
	protected string m_sHall;
	protected int m_iBoardVersion;
	protected int m_iHallVersion;
	protected int m_iMapVersion;
	protected ref set<string> m_aLockedLoadouts = new set<string>();
	protected int m_iStandingVersion;
	protected int m_iJournalVersion;
	protected int m_iBackgroundsVersion;
	protected int m_iRadioVersion;
	protected ref array<ref RZ_RadioEntry> m_aRadio = {};

	// ---------------------------------------------------------------- server
	protected ref array<float> m_aLastAsk = {-100, -100, -100, -100, -100, -100};
	protected ref array<float> m_aLastCommand = {-100, -100, -100, -100};	// per CMD_*

	//! Commands from the client (RZ_ChatCommands, the board menu)
	static const int CMD_NOTE = 0;
	static const int CMD_CLEAR_BOARD = 1;
	static const int CMD_CLEAR_ALL = 2;
	static const int CMD_HALL = 3;

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
	//! A new life: the radio log and the timers belong to the last one.
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NewLife()
	{
		m_aRadio.Clear();
		m_iRadioVersion++;
		m_iInfState = RZ_EInfectionUI.NONE;
		m_iListenFaction = -1;
		m_iNoise = 0;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Payload(int kind, string payload)
	{
		if (kind == 0)
		{
			m_sStanding = payload;
			m_iStandingVersion++;
		}
		else if (kind == 1)
		{
			m_sJournal = payload;
			m_iJournalVersion++;
		}
		else if (kind == 3)
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
		else
		{
			m_sBackgrounds = payload;
			m_iBackgroundsVersion++;
			ReadLockedLoadouts();
		}
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

		// A cheap spam guard per kind: the menus ask once when they open, the map every 20 s
		if (kind < 0 || kind >= RZ_UIFeed.PAYLOAD_KINDS)
			return;

		float now = RZ_UIW.Now();
		if (now - m_aLastAsk[kind] < ASK_GAP)
			return;

		m_aLastAsk[kind] = now;
		int playerId = OwnerPlayerId();
		if (playerId <= 0)
			return;

		SendPayload(kind, gm.GetUIFeed().BuildPayload(playerId, kind));
	}

	// ================================================================ Client: commands
	//------------------------------------------------------------------------------------------------
	//! A command for the server: CMD_NOTE with the note's text, the board clears (admins), the hall.
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
			case CMD_HALL:
			{
				string hall = "";
				if (gm.GetHall())
					hall = gm.GetHall().BuildText();
				if (hall.IsEmpty())
					hall = "Nobody's name is on it yet.";

				gm.NotifyPlayer(playerId, "Hall of fame", hall);
				break;
			}
		}
	}

	// ================================================================ Client: the radio log
	//------------------------------------------------------------------------------------------------
	//! Every notice passes through here on the client; radio ones are kept for the log.
	void OnNotice(string title, string text, RZ_NoticeStyle style)
	{
		if (!style || !style.IsRadio())
			return;

		RZ_RadioEntry e = new RZ_RadioEntry();
		e.m_sTime = RZ_UIText.TimeOfDay();
		e.m_iKind = style.m_iKind;
		e.m_iFaction = style.m_iFaction;
		e.m_sTitle = title;
		e.m_sText = text;
		m_aRadio.InsertAt(e, 0);
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
	string GetStanding() { return m_sStanding; }
	string GetJournal() { return m_sJournal; }
	string GetBackgrounds() { return m_sBackgrounds; }
	string GetMap() { return m_sMap; }
	string GetBoard() { return m_sBoard; }
	string GetHall() { return m_sHall; }
	int GetBoardVersion() { return m_iBoardVersion; }
	int GetHallVersion() { return m_iHallVersion; }
	int GetMapVersion() { return m_iMapVersion; }
	int GetStandingVersion() { return m_iStandingVersion; }
	int GetJournalVersion() { return m_iJournalVersion; }
	int GetBackgroundsVersion() { return m_iBackgroundsVersion; }
	int GetRadioVersion() { return m_iRadioVersion; }
	array<ref RZ_RadioEntry> GetRadioLog() { return m_aRadio; }
}
