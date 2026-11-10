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
//! menus ask for their data when they open (at most once every 2 s per player).
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
	protected int m_iStandingVersion;
	protected int m_iJournalVersion;
	protected int m_iBackgroundsVersion;
	protected int m_iRadioVersion;
	protected ref array<ref RZ_RadioEntry> m_aRadio = {};

	// ---------------------------------------------------------------- server
	protected float m_fLastAsk = -100;

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
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId); // VERIFY: PlayerManager.GetPlayerController(playerId) on the server
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

		return pc.GetPlayerId(); // VERIFY: PlayerController.GetPlayerId()
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
	//! kind: 0 standing, 1 journal, 2 backgrounds
	void SendPayload(int kind, string payload)
	{
		if (IsLocalOwner())
			RpcDo_Payload(kind, payload);
		else
			Rpc(RpcDo_Payload, kind, payload); // VERIFY: a string RPC argument of a few KB (RZ_UIFeed caps payloads at 6000 characters)
	}

	// ================================================================ Client: receive
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)] // VERIFY: RplRcver.Owner reaches the owning client of a player controller component
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
		else
		{
			m_sBackgrounds = payload;
			m_iBackgroundsVersion++;
		}
	}

	// ================================================================ Client: ask
	//------------------------------------------------------------------------------------------------
	//! kind: 0 standing, 1 journal, 2 backgrounds
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

		// A cheap spam guard: the menus ask once when they open
		float now = RZ_UIW.Now();
		if (now - m_fLastAsk < ASK_GAP && kind != 2)
			return;

		m_fLastAsk = now;
		int playerId = OwnerPlayerId();
		if (playerId <= 0)
			return;

		SendPayload(kind, gm.GetUIFeed().BuildPayload(playerId, kind));
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
	int GetStandingVersion() { return m_iStandingVersion; }
	int GetJournalVersion() { return m_iJournalVersion; }
	int GetBackgroundsVersion() { return m_iBackgroundsVersion; }
	int GetRadioVersion() { return m_iRadioVersion; }
	array<ref RZ_RadioEntry> GetRadioLog() { return m_aRadio; }
}
