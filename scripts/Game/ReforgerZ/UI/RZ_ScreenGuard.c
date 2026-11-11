//! The screen guard (docs/UI.md, "The screen guard"). Server side.
//!
//! The talk screen, the reader, the car repair screen and the shop screen stop you moving while
//! the world keeps running, so the server watches every open one and closes it when the player is
//! in danger: hurt, firing, an infected close by or closing in, dead, or (all but the reader) the
//! person, car or quartermaster gone, too far away, or nothing picked for a while. Polled once a second on the game mode's SecondTick; each check is a few distance tests,
//! and at most Dialogue > Open at most screens are open on the server.

//------------------------------------------------------------------------------------------------
//! One open screen.
class RZ_OpenScreen
{
	int m_iPlayerId;
	int m_iKind;
	IEntity m_Speaker;			// talk: the person (weak)
	bool m_bHasSpeaker;
	string m_sSpeaker;
	int m_iFaction = -1;
	float m_fLastPickAt;
	float m_fHealth;
	float m_fNearest = -1;		// the nearest infected within the closing-in radius at the last check, -1 none
	bool m_bShot;
}

//------------------------------------------------------------------------------------------------
class RZ_ScreenGuard
{
	static const int KIND_READER = 1;
	static const int KIND_TALK = 2;
	static const int KIND_CAR = 3;		// the car repair screen (Interactions/RZ_Cars.c)
	static const int KIND_SHOP = 4;		// the quartermaster's shop screen (Factions/RZ_ShopScreen.c)

	protected static const float HURT_DROP = 0.5;
	protected static const float CLOSING_STEP = 2;

	protected RZ_GameModeComponent m_GM;
	protected ref map<int, ref RZ_OpenScreen> m_mOpen = new map<int, ref RZ_OpenScreen>();
	protected float m_fClock;

	//------------------------------------------------------------------------------------------------
	void RZ_ScreenGuard(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! A screen opens for this player (it replaces one already open). False when the server is
	//! at its cap: the caller falls back to notices.
	bool Open(int playerId, int kind, IEntity speaker, string speakerName, int faction)
	{
		RZ_OpenScreen s;
		if (!m_mOpen.Find(playerId, s))
		{
			if (m_mOpen.Count() >= Math.Max(1, RZ_DialogueSettings.Get().m_iMaxOpen))
				return false;

			s = new RZ_OpenScreen();
			m_mOpen.Insert(playerId, s);
		}

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		s.m_iPlayerId = playerId;
		s.m_iKind = kind;
		s.m_Speaker = speaker;
		s.m_bHasSpeaker = speaker != null;
		s.m_sSpeaker = speakerName;
		s.m_iFaction = faction;
		s.m_fLastPickAt = m_fClock;
		s.m_fHealth = RZ_BiteDetector.HealthOf(player);
		s.m_fNearest = -1;
		s.m_bShot = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The player picked a reply: the idle clock starts again.
	void Touch(int playerId)
	{
		RZ_OpenScreen s;
		if (m_mOpen.Find(playerId, s))
			s.m_fLastPickAt = m_fClock;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpen(int playerId, int kind)
	{
		RZ_OpenScreen s;
		if (!m_mOpen.Find(playerId, s))
			return false;

		return s.m_iKind == kind;
	}

	//------------------------------------------------------------------------------------------------
	//! Forget a screen without telling anyone (its owner closed it).
	void Close(int playerId)
	{
		m_mOpen.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The client says the player closed it (RZ_PlayerUIComponent.RpcAsk_ScreenClosed).
	void OnClientClosed(int playerId, int kind)
	{
		RZ_OpenScreen s;
		if (!m_mOpen.Find(playerId, s) || s.m_iKind != kind)
			return;

		m_mOpen.Remove(playerId);
		Gone(playerId, kind);
	}

	//------------------------------------------------------------------------------------------------
	//! The system behind a screen forgets it (closed by the player or by the guard).
	protected void Gone(int playerId, int kind)
	{
		if (kind == KIND_TALK && m_GM.GetDialogue())
			m_GM.GetDialogue().OnScreenGone(playerId);
		else if (kind == KIND_CAR && m_GM.GetInteractions())
			m_GM.GetInteractions().GetCars().OnScreenGone(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A shot from this player (RZ_GameModeComponent.OnPlayerShot): closes at the next check.
	void OnShot(int playerId)
	{
		RZ_OpenScreen s;
		if (m_mOpen.Find(playerId, s))
			s.m_bShot = true;
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_mOpen.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second (RZ_GameModeComponent.SecondTick).
	void Tick()
	{
		m_fClock += 1;
		if (m_mOpen.IsEmpty())
			return;

		array<int> tripped = {};
		array<string> reasons = {};
		foreach (int playerId, RZ_OpenScreen s : m_mOpen)
		{
			string reason = Check(s);
			if (!reason.IsEmpty())
			{
				tripped.Insert(playerId);
				reasons.Insert(reason);
			}
		}

		foreach (int i, int id : tripped)
		{
			Trip(id, reasons[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "" while it may stay open; else why it closes: danger, left, idle or gone.
	protected string Check(RZ_OpenScreen s)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayerId);
		if (!player || !RZ_Util.IsCharacterAlive(player))
			return "gone";

		if (s.m_bShot)
			return "danger";

		float hp = RZ_BiteDetector.HealthOf(player);
		if (hp < s.m_fHealth - HURT_DROP)
			return "danger";

		s.m_fHealth = hp;
		RZ_DialogueSettings settings = RZ_DialogueSettings.Get();
		bool talk = s.m_iKind == KIND_TALK;
		bool near = s.m_iKind != KIND_READER;	// talk, the car and the shop: you stay with them
		if (near && s.m_bHasSpeaker)
		{
			if (!s.m_Speaker)
				return "gone";

			if (ChimeraCharacter.Cast(s.m_Speaker) && !RZ_Util.IsCharacterAlive(s.m_Speaker))
				return "gone";
		}

		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		if (spawner)
		{
			float closing = Math.Max(settings.m_fDangerNear, settings.m_fDangerClosing);
			float d = spawner.NearestZombieDist(player.GetOrigin(), closing);
			if (d <= settings.m_fDangerNear)
				return "danger";

			if (s.m_fNearest >= 0 && d <= closing && d < s.m_fNearest - CLOSING_STEP)
				return "danger";

			if (d <= closing)
				s.m_fNearest = d;
			else
				s.m_fNearest = -1;

			if (talk && s.m_Speaker && spawner.NearestZombieDist(s.m_Speaker.GetOrigin(), settings.m_fDangerNear) <= settings.m_fDangerNear)
				return "danger";
		}

		if (!near)
			return "";

		if (s.m_Speaker && RZ_Util.DistXZ(player.GetOrigin(), s.m_Speaker.GetOrigin()) > settings.m_fRange)
			return "left";

		if (m_fClock - s.m_fLastPickAt > settings.m_fIdleSeconds)
			return "idle";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Close it: the client's screen, the conversation, and in danger one short line from the person.
	protected void Trip(int playerId, string reason)
	{
		RZ_OpenScreen s;
		if (!m_mOpen.Find(playerId, s))
			return;

		int kind = s.m_iKind;
		string speaker = s.m_sSpeaker;
		int faction = s.m_iFaction;
		m_mOpen.Remove(playerId);
		RZ_Log.Info(string.Format("Screen guard: closed %1 for player %2 (%3).", KindName(kind), playerId, reason));

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
		{
			if (kind == KIND_TALK)
				ui.SendPayload(RZ_UIFeed.PAYLOAD_TALK, "X|" + reason);
			else if (kind == KIND_CAR)
				ui.SendPayload(RZ_UIFeed.PAYLOAD_CAR, "X|" + reason);
			else
				ui.SendPayload(RZ_UIFeed.PAYLOAD_READER, "");
		}

		Gone(playerId, kind);
		if (kind != KIND_TALK)
			return;

		if (reason == "danger" && !speaker.IsEmpty())
			m_GM.Say(playerId, speaker, faction, "\"Not now!\"");
	}

	//------------------------------------------------------------------------------------------------
	static string KindName(int kind)
	{
		if (kind == KIND_TALK)
			return "talk";

		if (kind == KIND_CAR)
			return "car repair";

		if (kind == KIND_SHOP)
			return "shop";

		return "reader";
	}
}

//------------------------------------------------------------------------------------------------
//! Server: while a screen acts for its player (a car job, a purchase), what the game would tell
//! that player as a notice is caught here instead and shown in the screen's status line
//! (RZ_GameModeComponent.NotifyPlayerAs asks Take first). One at a time, synchronous only.
class RZ_ScreenCapture
{
	protected static int s_iPlayer = -1;
	protected static string s_sTitle;
	protected static string s_sText;

	//------------------------------------------------------------------------------------------------
	static void Begin(int playerId)
	{
		s_iPlayer = playerId;
		s_sTitle = "";
		s_sText = "";
	}

	//------------------------------------------------------------------------------------------------
	//! True if the notice was caught (it's for the player a screen is acting for).
	static bool Take(int playerId, string title, string text)
	{
		if (s_iPlayer < 0 || playerId != s_iPlayer)
			return false;

		s_sTitle = title;
		if (!s_sText.IsEmpty())
			s_sText += " ";

		s_sText += text;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Stops catching; what was caught ("" if nothing).
	static string End()
	{
		s_iPlayer = -1;
		return s_sText;
	}

	//------------------------------------------------------------------------------------------------
	//! The title of the last notice caught (for a card when the screen closed meanwhile).
	static string LastTitle()
	{
		return s_sTitle;
	}
}
