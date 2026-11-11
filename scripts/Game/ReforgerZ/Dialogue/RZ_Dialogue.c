//! Talking to people (docs/DESIGN.md, "Talking to people"). Server side.
//!
//! One engine for every person who already talks: stronghold and seat leaders, safehouse
//! occupants, named people at holdings, wanderers, the Long Table's people, and the people
//! missions and questlines spawn. It doesn't rewrite them. It wraps what each already does:
//!
//! - the greeting is that person's Talk handler, run while the engine **captures** the player's
//!   notices and spoken lines whose title is the speaker's (the adapter knows it; else the first
//!   one of the greeting). Those lines go into the talk screen; anything else (a contract offer,
//!   a standing change) still goes out as a card. A later line from the speaker (a treatment
//!   finishing) also goes into the screen while it's open;
//! - the replies are the person's choice and use slots (their labels are on the server, in
//!   RZ_PuzzleObjectComponent), with what each needs shown before you choose, plus Leave;
//! - a pick runs that slot's handler, capturing again.
//!
//! So every rule that's built (a Trader's price, a leader's refusal, an exile's shut door) works
//! exactly as built. The engine adds no rewards and no memory. The client gets one node at a time,
//! owner-only (RZ_UIFeed.PAYLOAD_TALK); its picks come back as RZ_PlayerUIComponent.RpcAsk_TalkPick
//! and are checked here (the serial, a reply id that was sent and enabled, alive, in range, not in
//! danger, at most one pick every 0.3 s). Nothing is saved.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Dialogue: the talk screen and the screen guard that closes
//! the talk screen and the reader when danger comes (UI/RZ_ScreenGuard.c). Read on every machine.
[BaseContainerProps()]
class RZ_DialogueSettings
{
	[Attribute("1", desc: "The talk screen: Talk to a person opens it, and their action-menu replies hide (needs UI > Talk reply layout and the RZ_TalkMenu preset). Off = the actions and notices, as before")]
	bool m_bTalkScreen;

	[Attribute("6", desc: "Talk: you stay in a conversation while you're this close to the person (m)")]
	float m_fRange;

	[Attribute("120", desc: "Talk: seconds with no reply picked before the conversation ends by itself")]
	float m_fIdleSeconds;

	[Attribute("6", desc: "Danger: an infected this close to you (or to the person you talk to) closes the talk screen and the reader (m)")]
	float m_fDangerNear;

	[Attribute("30", desc: "Danger: an infected within this distance that came 2 m closer since the last check closes the screen (m)")]
	float m_fDangerClosing;

	[Attribute("32", desc: "Talk screens and readers open at once on the server, at most")]
	int m_iMaxOpen;

	[Attribute("0", desc: "Conversation trees on the talk screen: topics, checks shown before you choose, memory (docs/DIALOGUE.md). Off (the default) = the talk screen as built. Needs the talk screen")]
	bool m_bTrees;

	[Attribute("20", desc: "Trees: the most Favor talk pays a player with one faction in a real day (all conversations together)")]
	int m_iTalkFavorPerDay;

	protected static ref RZ_DialogueSettings s_Defaults;

	//------------------------------------------------------------------------------------------------
	//! The game mode's settings, else the defaults (always non-null).
	static RZ_DialogueSettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetDialogueSettings())
			return gm.GetDialogueSettings();

		if (!s_Defaults)
			s_Defaults = RZ_DialogueSettings.Cast(RZ_Util.NewWithDefaults("RZ_DialogueSettings"));

		return s_Defaults;
	}
}

//------------------------------------------------------------------------------------------------
//! One reply in the talk screen.
class RZ_TalkReply
{
	int m_iId;
	string m_sLabel;
	string m_sTag;		// a dim tag on the right ("a job", "trade", "20 Haven Favor")
	string m_sNeed;		// what it needs, shown when it can't be picked
	bool m_bEnabled = true;
	bool m_bOwn;		// a tree's topic or choice: tagged by the trees, not by RZ_TalkNeeds
}

//------------------------------------------------------------------------------------------------
//! One open conversation.
class RZ_TalkSession
{
	int m_iPlayer;
	IEntity m_Speaker;			// weak
	ref RZ_TalkSpeaker m_Adapter;
	int m_iSerial;
	string m_sHeader;			// the title their lines come under
	string m_sName;
	string m_sSub;
	int m_iFaction = -1;
	ref array<string> m_aSaid = {};		// what they said last (shown)
	ref array<string> m_aNew = {};		// caught while a handler runs
	ref array<ref RZ_TalkReply> m_aReplies = {};
	bool m_bCapturing;
	float m_fLastPick = -100;
	ref RZ_DTalk m_Tree;				// Dialogue > Trees: the topics and the node (null when off)
}

//------------------------------------------------------------------------------------------------
class RZ_Dialogue
{
	static const int LEAVE = 99;
	protected static const float TALK_GAP = 2;
	protected static const float PICK_GAP = 0.3;
	protected static const int MAX_SAID = 1400;
	protected static const string LINE_BREAK = "^";

	protected RZ_GameModeComponent m_GM;
	protected ref map<int, ref RZ_TalkSession> m_mSessions = new map<int, ref RZ_TalkSession>();
	protected ref map<int, float> m_mLastTalk = new map<int, float>();
	protected int m_iSerial;
	protected ref RZ_DialogueTrees m_Trees;

	//------------------------------------------------------------------------------------------------
	void RZ_Dialogue(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		m_Trees = new RZ_DialogueTrees(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! Conversation trees (Dialogue > Trees): the database and memory, used only while it's on.
	RZ_DialogueTrees GetTrees() { return m_Trees; }

	//------------------------------------------------------------------------------------------------
	//! The open conversation of a player, or null.
	RZ_TalkSession GetSession(int playerId)
	{
		RZ_TalkSession s;
		m_mSessions.Find(playerId, s);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The talk screen is used: Dialogue > Talk screen on and UI > Talk reply layout set (the
	//! client's RZ_TalkMenu.Available() reads the same two).
	static bool IsOn()
	{
		return RZ_DialogueSettings.Get().m_bTalkScreen && !RZ_UISettings.Get().m_sTalkReplyLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! A living character: a person to talk to (bodies, objects and spots keep their actions).
	static bool IsPerson(IEntity e)
	{
		return ChimeraCharacter.Cast(e) != null && RZ_Util.IsCharacterAlive(e);
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to ..." on a person (RZ_ReadClueAction). False: not handled, the caller does what it
	//! always did. True: the talk screen took it (or ignored a second Talk within 2 s).
	bool Open(int playerId, IEntity owner, IEntity user)
	{
		if (!IsOn() || !IsPerson(owner))
			return false;

		return Start(playerId, owner, user, RZ_TalkSpeaker.For(m_GM, owner));
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to whoever's in charge" on a quartermaster (RZ_LeaderAction): the leader of this seat.
	bool OpenSeat(int playerId, IEntity owner, IEntity user)
	{
		if (!IsOn() || !m_GM.GetLeaders())
			return false;

		RZ_Leader l = m_GM.GetLeaders().FindSeat(owner);
		if (!l || l.m_iHeldBy >= 0)
			return false;

		return Start(playerId, owner, user, new RZ_TalkLeader(l));
	}

	//------------------------------------------------------------------------------------------------
	protected bool Start(int playerId, IEntity owner, IEntity user, RZ_TalkSpeaker adapter)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!adapter || !ui || !guard)
			return false;

		float now = RZ_UIW.Now();
		float last;
		if (m_mLastTalk.Find(playerId, last) && now - last < TALK_GAP)
			return true;

		m_mLastTalk.Set(playerId, now);
		m_mSessions.Remove(playerId);
		RZ_TalkSession s = new RZ_TalkSession();
		s.m_iPlayer = playerId;
		s.m_Speaker = owner;
		s.m_Adapter = adapter;
		adapter.Setup(m_GM, s, playerId);
		if (!guard.Open(playerId, RZ_ScreenGuard.KIND_TALK, owner, s.m_sName, s.m_iFaction))
			return false;

		m_mSessions.Insert(playerId, s);
		s.m_Tree = m_Trees.Begin(s, user);
		s.m_bCapturing = true;
		adapter.Greet(m_GM, s, playerId, user);
		s.m_bCapturing = false;

		// The handler may have ended it, or said nothing at all: no screen then
		if (!m_mSessions.Contains(playerId))
			return true;

		// Trees: a greet whose `when` holds goes before the built greeting
		if (s.m_Tree)
		{
			array<string> opening = {};
			m_Trees.Opening(s, opening);
			for (int i = opening.Count() - 1; i >= 0; i--)
			{
				s.m_aNew.InsertAt(opening[i], 0);
			}
		}

		if (s.m_aNew.IsEmpty())
		{
			End(playerId, "", false);
			return true;
		}

		TakeNew(s);
		SendNode(s, user);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.NotifyPlayerAs and Say: a line for a player who is talking. True if
	//! it went into the talk screen (then it isn't a card or a subtitle).
	bool Capture(int playerId, string title, string text)
	{
		RZ_TalkSession s;
		if (!m_mSessions.Find(playerId, s))
			return false;

		if (s.m_sHeader.IsEmpty())
		{
			if (!s.m_bCapturing)
				return false;

			s.m_sHeader = title;
			s.m_Adapter.FromHeader(m_GM, s, title);
		}

		if (title != s.m_sHeader)
			return false;

		s.m_aNew.Insert(text);
		if (!s.m_bCapturing)
		{
			TakeNew(s);
			SendNode(s, GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The player picked a reply (RZ_PlayerUIComponent.RpcAsk_TalkPick), or left (LEAVE).
	void Pick(int playerId, int serial, int replyId)
	{
		RZ_TalkSession s;
		if (!m_mSessions.Find(playerId, s) || serial != s.m_iSerial)
			return;

		if (replyId == LEAVE)
		{
			End(playerId, "", false);
			return;
		}

		RZ_TalkReply reply;
		foreach (RZ_TalkReply r : s.m_aReplies)
		{
			if (r.m_iId == replyId)
				reply = r;
		}

		float now = RZ_UIW.Now();
		if (!reply || !reply.m_bEnabled || now - s.m_fLastPick < PICK_GAP)
			return;

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!user || !RZ_Util.IsCharacterAlive(user) || !guard || !guard.IsOpen(playerId, RZ_ScreenGuard.KIND_TALK))
		{
			End(playerId, "gone", true);
			return;
		}

		if (!s.m_Speaker || !IsPerson(s.m_Speaker) || RZ_Util.DistXZ(user.GetOrigin(), s.m_Speaker.GetOrigin()) > RZ_DialogueSettings.Get().m_fRange)
		{
			End(playerId, "left", true);
			return;
		}

		s.m_fLastPick = now;
		guard.Touch(playerId);
		s.m_aNew.Clear();
		s.m_bCapturing = true;
		if (s.m_Tree && RZ_DialogueTrees.IsOurs(replyId))
		{
			bool goOn = m_Trees.Pick(s, replyId, user);
			s.m_bCapturing = false;
			if (!m_mSessions.Contains(playerId))
				return;

			if (!goOn)
			{
				End(playerId, "", true);
				return;
			}
		}
		else
		{
			s.m_Adapter.Pick(m_GM, s, playerId, replyId, user);
		}

		s.m_bCapturing = false;
		if (!m_mSessions.Contains(playerId))
			return;

		// Nothing new said (the answer was a card, a contract offer): what they said stays
		TakeNew(s);
		SendNode(s, user);
	}

	//------------------------------------------------------------------------------------------------
	//! The screen is gone: the player closed it, or the screen guard did (it told the client).
	void OnScreenGone(int playerId)
	{
		m_mSessions.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_mSessions.Remove(playerId);
		m_mLastTalk.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Ends a conversation; tellClient sends the close record (X|reason).
	protected void End(int playerId, string reason, bool tellClient)
	{
		m_mSessions.Remove(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (guard)
			guard.Close(playerId);

		if (!tellClient)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_TALK, "X|" + reason);
	}

	//------------------------------------------------------------------------------------------------
	protected void TakeNew(RZ_TalkSession s)
	{
		if (s.m_aNew.IsEmpty())
			return;

		s.m_aSaid.Clear();
		s.m_aSaid.InsertAll(s.m_aNew);
		s.m_aNew.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! T|serial|faction|name|sub|said, then R|id|label|tag|need|enabled per reply (the last is
	//! Leave). Line breaks in what they said travel as ^.
	protected void SendNode(RZ_TalkSession s, IEntity user)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(s.m_iPlayer);
		if (!ui)
		{
			End(s.m_iPlayer, "", false);
			return;
		}

		s.m_aReplies.Clear();
		if (s.m_Tree && s.m_Tree.m_bInNode)
		{
			m_Trees.NodeReplies(s, s.m_aReplies);
		}
		else
		{
			s.m_Adapter.Replies(m_GM, s, s.m_iPlayer, user, s.m_aReplies);
			if (s.m_Tree)
				m_Trees.HubReplies(s, s.m_aReplies);
		}

		while (s.m_aReplies.Count() > 6)
		{
			s.m_aReplies.Remove(s.m_aReplies.Count() - 1);
		}

		foreach (RZ_TalkReply r : s.m_aReplies)
		{
			if (!r.m_bOwn)
				RZ_TalkNeeds.Apply(m_GM, s.m_iPlayer, user, r);
		}

		RZ_TalkReply leave = new RZ_TalkReply();
		leave.m_iId = LEAVE;
		leave.m_sLabel = "I'll be going.";
		s.m_aReplies.Insert(leave);

		string said = "";
		foreach (int i, string line : s.m_aSaid)
		{
			if (i > 0)
				said += LINE_BREAK;

			string one = line;
			one.Replace("\n", LINE_BREAK);
			said += one;
		}

		m_iSerial++;
		s.m_iSerial = m_iSerial;
		string sep = RZ_UIText.SEP;
		string payload = "T" + sep + s.m_iSerial.ToString() + sep + s.m_iFaction.ToString() + sep + RZ_UIText.Clean(s.m_sName, 80)
			+ sep + RZ_UIText.Clean(s.m_sSub, 120) + sep + RZ_UIText.Clean(said, MAX_SAID);
		foreach (RZ_TalkReply rep : s.m_aReplies)
		{
			int on = 0;
			if (rep.m_bEnabled)
				on = 1;

			payload += "\nR" + sep + rep.m_iId.ToString() + sep + RZ_UIText.Clean(rep.m_sLabel, 120) + sep + RZ_UIText.Clean(rep.m_sTag, 40)
				+ sep + RZ_UIText.Clean(rep.m_sNeed, 80) + sep + on.ToString();
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_TALK, payload); // VERIFY: an owner-only string RPC of up to about 2,000 characters arrives whole
	}
}

//------------------------------------------------------------------------------------------------
//! What a reply needs, shown before you choose (no dice anywhere): a price in Favor in its label
//! ("Buy supplies (20 Haven Favor)"), the antidote you'd give, the food you'd share.
class RZ_TalkNeeds
{
	//------------------------------------------------------------------------------------------------
	static void Apply(RZ_GameModeComponent gm, int playerId, IEntity user, RZ_TalkReply r)
	{
		string label = r.m_sLabel;
		int open = label.LastIndexOf("(");
		if (open > 0 && label.EndsWith(" Favor)"))
		{
			string inside = label.Substring(open + 1, label.Length() - open - 2);	// "20 Haven Favor"
			string lower = inside;
			lower.ToLower();
			int cost = inside.ToInt();
			int faction = RZ_NoticeStyle.FactionIn(lower);
			string bare = label.Substring(0, open);
			r.m_sLabel = bare.Trim();
			r.m_sTag = inside;
			if (cost > 0 && faction >= 0)
			{
				int have = gm.GetProfile(playerId).GetFavor(faction);
				if (have < cost)
				{
					r.m_bEnabled = false;
					r.m_sNeed = string.Format("needs %1, you have %2", inside, have);
				}
			}

			return;
		}

		if (label == "Give them your antidote" && !RZ_AntidoteHelper.Has(user))
		{
			r.m_bEnabled = false;
			r.m_sNeed = "you have none";
			return;
		}

		if (label == "Share some food" && RZ_HavenQuest.CountFood(gm, user) <= 0)
		{
			r.m_bEnabled = false;
			r.m_sNeed = "you have nothing to eat";
			return;
		}

		if (r.m_sTag.IsEmpty())
			r.m_sTag = TagFor(label);
	}

	//------------------------------------------------------------------------------------------------
	//! A dim tag on what a reply is.
	static string TagFor(string label)
	{
		string l = label;
		l.ToLower();
		if (l.Contains("work"))
			return "a job";
		if (l.Contains("news") || l.Contains("heard anything") || l.Contains("how things are") || l.Contains("rumour"))
			return "news";
		if (l.StartsWith("rob "))
			return "Haven Infamy";
		if (l.StartsWith("trade") || l.StartsWith("buy"))
			return "trade";

		return "";
	}
}
