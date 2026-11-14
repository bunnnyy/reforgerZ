//! Talking to people (docs/DESIGN.md, "Talking to people"). Server side.
//!
//! One engine for every person who already talks: stronghold and seat leaders, safehouse
//! occupants, named people at holdings, wanderers, the Long Table's people, and the people
//! missions and questlines spawn. It doesn't rewrite them. It wraps what each already does:
//!
//! - the greeting is that person's Talk handler, run while the engine **captures** the player's
//!   notices and spoken lines whose title is the speaker's (the adapter knows it; else the first
//!   one of the greeting). Those lines go into the talk screen, and so does whatever else the
//!   handler tells the player (Capture): a journal line (a contract offer) is a card as well;
//!   danger, radio and tips stay cards. A later line from the speaker (a treatment finishing)
//!   also goes into the screen while it's open;
//! - the replies are the person's choice and use slots (their labels are on the server, in
//!   RZ_PuzzleObjectComponent), with what each needs shown before you choose, plus Leave;
//! - a pick runs that slot's handler, capturing again; the answer shows, then the replies after it.
//!
//! The screen shows at most 5 replies: a page of up to 4 and the way out, with the page and the count
//! sent along ("1/2"); the client turns pages (LB/RB, Q/E, the arrows) by sending MORE or PAGE_BACK.
//! A server admin's replies sit under one [Admin] reply that opens them as their own list.
//! A reply with nothing new behind it is sent as heard (grey, still picked as any other).
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

	[Attribute("1", desc: "Trees: a leader who won't deal with you (attitude -2 or below, not Hunted) offers Make it right: their faction's amends job; done, they start again with you, once a season (docs/DIALOGUE.md)")]
	bool m_bMakeRight;

	[Attribute("1", desc: "Trees: a greet written to call out (a road post asking who you're with, a stranger) says its first line as a subtitle when you walk up, inside the unasked-contact budget; the talk screen opens only when you talk to them")]
	bool m_bOpens;

	[Attribute("1", desc: "Trees: distress calls on the open channel (answering sends the caller to the nearest Haven house's gate) and Radio Operators talking on a cracked secure net")]
	bool m_bRadioCalls;

	[Attribute("45", desc: "Talk: seconds with no reply before a conversation ends while another player waits at the same person")]
	float m_fWaitingIdle;

	[Attribute("6", desc: "Talk: conversations with one person at once")]
	int m_iPerSpeaker;

	[Attribute("16", desc: "Talk: conversations with one person at once while someone there is in a public node (a scene)")]
	int m_iPerSpeakerScene;

	[Attribute("8", desc: "Trees: overheard lines, vouching and call-outs reach this far (m)")]
	float m_fHear;

	[Attribute("30", desc: "Trees: a deciding hold and a scene's result are told to the room this far (m)")]
	float m_fRoom;

	[Attribute("5", desc: "Trees: seconds a deciding pick waits for an objection")]
	float m_fHold;

	[Attribute("2", desc: "Trees: a promise's default hours of the promiser's own play")]
	float m_fPromiseHours;

	[Attribute("3", desc: "Trees: a promise breaks after this many real days, whatever the play")]
	int m_iPromiseDays;

	[Attribute("6", desc: "Trees: barks are said to a player passing this close (m)")]
	float m_fBarkRadius;

	[Attribute("60", desc: "Trees: seconds between two barks to one player")]
	float m_fBarkPerPlayer;

	[Attribute("600", desc: "Trees: seconds between two barks from one person to one player")]
	float m_fBarkPerSpeaker;

	[Attribute("0.15", desc: "Lies: the chance a believed lie with nothing in the world to test it is found out at each of its two rumour rounds (missions, errands and talk alike)")]
	float m_fRumourChance;

	[Attribute("1", desc: "Nametags: a small tag over people you can talk to when you're close and can see them: name, role, who they're with (needs NametagPanel in the HUD layout). Never over the infected or players")]
	bool m_bNametags;

	[Attribute("8", desc: "Nametags: shown within this distance (m)")]
	float m_fNametagRange;

	[Attribute("3", desc: "Nametags: at most this many at once, the nearest (the HUD layout has three)")]
	int m_iNametagMax;

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
	// The order replies show in (docs/DIALOGUE.md, "The order of the replies"): lowest first, the way
	// out always last
	static const int RANK_ON = 0;		// advances or hands in a job you're on
	static const int RANK_NEW = 1;		// a new job on offer you can take
	static const int RANK_LOCKED = 2;	// a job on offer you can't take yet (locked, with its need)
	static const int RANK_GOODS = 3;	// trade, services, items, favours (a built reply's default)
	static const int RANK_INFO = 4;		// information: news, what they make of you, a topic about the world
	static const int RANK_TALK = 5;		// small talk (a flavour topic)
	static const int RANK_ADMIN = 9;	// [Admin]

	int m_iId;
	string m_sLabel;
	int m_iRank = -1;	// RANK_*; -1: from the reply (RZ_TalkRank.For)
	bool m_bJob;		// starts a job or opens its pitch: never carried by a topic, never grey
	string m_sTag;		// the tag in front ("[Job]", "[Persuade - Haven - 1 of 2]", "[20 Haven Favor]")
	string m_sNeed;		// what it needs, shown when it can't be picked
	bool m_bEnabled = true;
	bool m_bOwn;		// a tree's topic or choice: tagged by the trees, not by RZ_TalkNeeds
	bool m_bAdmin;		// a server admin's reply: listed under [Admin], not with the others
	bool m_bHeard;		// nothing new behind it: shown grey, still picked as any other
	bool m_bWay;		// one of an errand's ways on offer: said as what you agree to (RZ_TalkWords.Agree)
	int m_iFor;			// the job it moves, as the Quests tab's row id (RZ_Dialogue.ForErrand, ForMission); 0: none
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
	bool m_bAdmin;						// the [Admin] list is open in place of the replies
	bool m_bEnded;						// a tree ended it: their last words, then only the way out
	int m_iPage;						// the page of replies shown (MORE and PAGE_BACK turn it)
	int m_iPages = 1;					// pages in the list now
	string m_sList;						// the list the page belongs to (the hub, a node, the admin list)
	ref array<string> m_aPicked = {};	// built replies picked in this conversation (shown grey after)
	bool m_bQuiet;						// no filler greeting: only what they actually have to say (docs/DIALOGUE.md)
	bool m_bGreeting;					// the greeting runs now (RZ_Dialogue.IsQuiet)
	bool m_bSilent;						// nothing new is said on purpose: the replies show with no line
	bool m_bAll;						// "There was something else." picked: every reply, not only the tracked job's
}

//------------------------------------------------------------------------------------------------
//! Where a conversation was when danger or distance ended it: Talk to the same person within 60 s
//! picks it up again ("Where were we?").
class RZ_TalkResume
{
	IEntity m_Speaker;		// weak
	string m_sConv;
	string m_sNode;
	float m_fAt;
}

//------------------------------------------------------------------------------------------------
class RZ_Dialogue
{
	static const int LEAVE = 99;
	static const int MORE = 98;			// the next page of replies (LB/RB, Q/E, the arrows; not a row)
	static const int PAGE_BACK = 89;	// the page before (not 95: that's Make it right's reply)
	static const int ADMIN_BACK = 97;	// Back.: from the [Admin] list to the replies
	static const int ADMIN = 96;		// [Admin]: a server admin's replies, as their own list
	static const int OTHER = 88;		// "There was something else.": every reply, after the tracked job's alone (not 62-87: a person's own actions)
	static const int ROWS = 5;			// replies on the screen at once, the way out included
	//! A house set without power can't call (OpenRadio, and the object menu's locked Call row)
	static const string NO_POWER = "No power. It'll hear the channel, but it won't carry your voice. A generator close by would do it.";
	protected static const float TALK_GAP = 2;
	protected static const float PICK_GAP = 0.3;
	protected static const int MAX_SAID = 1400;
	protected static const float IDLE_BUMP = 30;		// at the cap, the one idle this long gives way
	protected static const float RESUME_SECONDS = 60;
	protected static const string LINE_BREAK = "^";
	protected static const string NO_ANSWER = "(They have nothing to add to that.)";	// a reply that said nothing

	protected RZ_GameModeComponent m_GM;
	protected ref map<int, ref RZ_TalkSession> m_mSessions = new map<int, ref RZ_TalkSession>();
	protected ref map<int, float> m_mLastTalk = new map<int, float>();
	protected ref map<int, ref RZ_TalkResume> m_mResume = new map<int, ref RZ_TalkResume>();
	protected int m_iSerial;
	protected ref RZ_DialogueTrees m_Trees;
	protected ref RZ_DialogueScenes m_Scenes;
	protected ref RZ_Nametags m_Nametags;

	//------------------------------------------------------------------------------------------------
	void RZ_Dialogue(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		m_Trees = new RZ_DialogueTrees(gm);
		m_Scenes = new RZ_DialogueScenes(gm);
		m_Nametags = new RZ_Nametags(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! The multiplayer layer (Dialogue/RZ_DialogueScenes.c).
	RZ_DialogueScenes GetScenes() { return m_Scenes; }

	//------------------------------------------------------------------------------------------------
	//! On the game mode's tick: the scenes' holds, the trees' clocks and barks, the nametags' lists,
	//! and a change of clothes ending a conversation.
	void Tick(float dt)
	{
		m_Scenes.Tick(dt);
		m_Trees.Tick(dt);
		m_Nametags.Tick(dt);
		WatchKit();
	}

	//------------------------------------------------------------------------------------------------
	//! Kit is read on every pick and on the server tick: any change ends the conversation, so nobody
	//! opens it in one outfit and picks in another.
	protected void WatchKit()
	{
		array<int> changed = {};
		foreach (int playerId, RZ_TalkSession s : m_mSessions)
		{
			if (s.m_Tree && m_Trees.KitChanged(s))
				changed.Insert(playerId);
		}

		foreach (int id : changed)
		{
			RZ_TalkSession gone = GetSession(id);
			string name = gone.m_sName;
			int faction = gone.m_iFaction;
			End(id, "", true);
			m_GM.Say(id, name, faction, "Didn't you just change your clothes? Never mind. Off you go.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every open conversation with this speaker.
	void SessionsWith(IEntity speaker, notnull array<RZ_TalkSession> result)
	{
		if (!speaker)
			return;

		foreach (int playerId, RZ_TalkSession s : m_mSessions)
		{
			if (s.m_Speaker == speaker)
				result.Insert(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A deciding pick's hold ran out unobjected: it lands for the decider, if they're still talking.
	void FinishHold(RZ_DSceneHold h)
	{
		RZ_TalkSession s = GetSession(h.m_iDecider);
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(h.m_iDecider);
		if (!s || !s.m_Tree || s.m_Speaker != h.m_Speaker || !user)
			return;

		RZ_ECheck result = h.m_iResult;
		s.m_aNew.Clear();
		s.m_bCapturing = true;
		bool goOn = m_Trees.FinishPick(s, h.m_iChoice, result, h.m_sVerdict, user);
		s.m_bCapturing = false;
		if (!m_mSessions.Contains(h.m_iDecider))
			return;

		if (!goOn)
		{
			Finish(s, user);
			return;
		}

		TakeNew(s);
		SendNode(s, user);
	}

	//------------------------------------------------------------------------------------------------
	//! A tree's pick ended the conversation (`end`): their last words show with only the way out, so
	//! you see the answer before you go. Nothing said: it closes at once.
	protected void Finish(RZ_TalkSession s, IEntity user)
	{
		if (s.m_aNew.IsEmpty())
		{
			End(s.m_iPlayer, "", true);
			return;
		}

		s.m_bEnded = true;
		if (s.m_Tree)
			s.m_Tree.m_bInNode = false;		// nothing to pick up again afterwards

		TakeNew(s);
		SendNode(s, user);
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
	//! `calledOver`: the speaker opened it (a greet that opens by itself), so they speak first.
	bool Open(int playerId, IEntity owner, IEntity user, bool calledOver = false)
	{
		if (!IsOn() || !IsPerson(owner))
			return false;

		return Start(playerId, owner, user, RZ_TalkSpeaker.For(m_GM, owner), calledOver);
	}

	//------------------------------------------------------------------------------------------------
	//! True while a talk-screen greeting runs for this player with nothing to say first: the greeting
	//! handlers leave out their filler (a plain hello, the routine line, the status readout) and say
	//! only what's real now (a refusal, a first meeting, a callback, news, a job waiting).
	bool IsQuiet(int playerId)
	{
		RZ_TalkSession s;
		return m_mSessions.Find(playerId, s) && s.m_bGreeting && s.m_bQuiet;
	}

	//------------------------------------------------------------------------------------------------
	//! IsQuiet for code that holds the game mode (the greeting handlers in World/ and Factions/).
	static bool Quiet(RZ_GameModeComponent gm, int playerId)
	{
		return gm && gm.GetDialogue() && gm.GetDialogue().IsQuiet(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! "Call on the open channel" (RZ_RadioCallAction) on a radio: the open channel, with no body.
	//! Needs Trees (there's nothing to say on air without them).
	bool OpenRadio(int playerId, IEntity radio, IEntity user)
	{
		if (!IsOn() || !radio || !m_Trees.IsOn())
			return false;

		// A house set needs power to carry a voice (a carried radio has its own batteries)
		if (!RZ_DialogueNets.CanCall(radio))
		{
			m_GM.NotifyPlayer(playerId, "Radio", NO_POWER);
			return true;
		}

		return Start(playerId, radio, user, new RZ_TalkRadio(radio));
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
	//! A quartermaster's "Ask for whoever's in charge" (RZ_TalkStaff): the leader of this seat takes
	//! the open conversation over, greeting and all, in the same screen. False: nobody's in the
	//! chair, or they're held; the caller says so.
	bool ToSeat(RZ_TalkSession s, IEntity owner, IEntity user)
	{
		if (!s || !m_GM.GetLeaders())
			return false;

		RZ_Leader l = m_GM.GetLeaders().FindSeat(owner);
		if (!l || l.m_iHeldBy >= 0)
			return false;

		RZ_TalkLeader adapter = new RZ_TalkLeader(l);
		s.m_Adapter = adapter;
		s.m_sHeader = "";
		s.m_sName = "";
		s.m_sSub = "";
		s.m_iFaction = -1;
		s.m_sList = "";
		s.m_bAdmin = false;
		s.m_aPicked.Clear();
		adapter.Setup(m_GM, s, s.m_iPlayer);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (guard)
			guard.Open(s.m_iPlayer, RZ_ScreenGuard.KIND_TALK, owner, s.m_sName, s.m_iFaction);

		s.m_Tree = m_Trees.Begin(s, user);
		s.m_aNew.Clear();
		s.m_bQuiet = true;
		s.m_bGreeting = true;
		adapter.Greet(m_GM, s, s.m_iPlayer, user);
		s.m_bGreeting = false;
		if (s.m_Tree)
		{
			// Trees: a greet whose `when` holds goes before the built greeting, as at Start
			array<string> opening = {};
			m_Trees.Opening(s, opening, true);
			for (int i = opening.Count() - 1; i >= 0; i--)
			{
				s.m_aNew.InsertAt(opening[i], 0);
			}

			m_Trees.MarkMet(s);
		}

		// Nothing to say: the leader's replies show with no line
		if (s.m_aNew.IsEmpty())
			s.m_bSilent = true;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Start(int playerId, IEntity owner, IEntity user, RZ_TalkSpeaker adapter, bool calledOver = false)
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
		if (!RoomAt(owner))
		{
			m_GM.NotifyPlayer(playerId, "Talk", "One at a time. Somebody's already talking to them.");
			return true;
		}

		RZ_TalkSession s = new RZ_TalkSession();
		s.m_iPlayer = playerId;
		s.m_Speaker = owner;
		s.m_Adapter = adapter;
		adapter.Setup(m_GM, s, playerId);
		if (!guard.Open(playerId, RZ_ScreenGuard.KIND_TALK, owner, s.m_sName, s.m_iFaction))
			return false;

		m_mSessions.Insert(playerId, s);
		s.m_Tree = m_Trees.Begin(s, user);

		// No filler greeting: unless they called you over or speak first (an opening's contact, a
		// stranger), the screen opens on your replies and they say only what's real now
		s.m_bQuiet = !calledOver && !adapter.SpeaksFirst(m_GM, s, playerId);
		s.m_bCapturing = true;
		s.m_bGreeting = true;
		adapter.Greet(m_GM, s, playerId, user);
		s.m_bGreeting = false;
		s.m_bCapturing = false;

		// The handler may have ended it
		if (!m_mSessions.Contains(playerId))
			return true;

		// Trees: a greet whose `when` holds goes before the built greeting (quiet: only its say?
		// lines that hold, and what's still promised to them)
		if (s.m_Tree)
		{
			array<string> opening = {};
			m_Trees.Opening(s, opening, s.m_bQuiet);
			for (int i = opening.Count() - 1; i >= 0; i--)
			{
				s.m_aNew.InsertAt(opening[i], 0);
			}

			// Cut off a moment ago by danger or distance: back where it was
			array<string> resumed = {};
			if (Resume(s, resumed))
			{
				s.m_aNew.Clear();
				s.m_aNew.Insert("Where were we?");
				s.m_aNew.InsertAll(resumed);
			}
		}

		// Nothing to say: a quiet greeting opens on the replies with no line; anyone else who says
		// nothing at all (gone, not there) gets no screen
		if (s.m_aNew.IsEmpty())
		{
			if (!s.m_bQuiet || !adapter.Here(m_GM))
			{
				End(playerId, "", false);
				return true;
			}

			s.m_aSaid.Clear();
		}

		if (s.m_Tree)
			m_Trees.MarkMet(s);

		TakeNew(s);
		SendNode(s, user);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A conversation in a node, ended by danger or distance: kept 60 s for the same person.
	protected void KeepForResume(RZ_TalkSession s)
	{
		if (!s || !s.m_Tree || !s.m_Tree.m_bInNode || !s.m_Tree.m_Conv || !s.m_Tree.m_Node)
			return;

		RZ_TalkResume r = new RZ_TalkResume();
		r.m_Speaker = s.m_Speaker;
		r.m_sConv = s.m_Tree.m_Conv.m_sId;
		r.m_sNode = s.m_Tree.m_Node.m_sId;
		r.m_fAt = RZ_UIW.Now();
		m_mResume.Set(s.m_iPlayer, r);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Resume(RZ_TalkSession s, notnull array<string> lines)
	{
		RZ_TalkResume r;
		if (!m_mResume.Find(s.m_iPlayer, r))
			return false;

		m_mResume.Remove(s.m_iPlayer);
		if (r.m_Speaker != s.m_Speaker || RZ_UIW.Now() - r.m_fAt > RESUME_SECONDS)
			return false;

		return m_Trees.Resume(s, r.m_sConv, r.m_sNode, lines);
	}

	//------------------------------------------------------------------------------------------------
	//! At most 6 conversations with one person (the open channel has no cap). At the cap, the one
	//! idle longest (over 30 s) gives way; else there's no room.
	protected bool RoomAt(IEntity owner)
	{
		if (RZ_TalkRadio.IsRadio(owner))
			return true;

		array<RZ_TalkSession> with = {};
		SessionsWith(owner, with);
		int cap = RZ_DialogueSettings.Get().m_iPerSpeaker;
		foreach (RZ_TalkSession pub : with)
		{
			if (pub.m_Tree && pub.m_Tree.m_Node && pub.m_Tree.m_Node.m_bPublic)
				cap = RZ_DialogueSettings.Get().m_iPerSpeakerScene;
		}

		if (with.Count() < cap)
			return true;

		float now = RZ_UIW.Now();
		RZ_TalkSession idlest;
		foreach (RZ_TalkSession s : with)
		{
			if (now - s.m_fLastPick >= IDLE_BUMP && (!idlest || s.m_fLastPick < idlest.m_fLastPick))
				idlest = s;
		}

		if (!idlest)
		{
			RZ_Log.Info("Dialogue: session cap at " + with[0].m_sName);
			return false;
		}

		End(idlest.m_iPlayer, "", true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.NotifyPlayerAs and Say: a line for a player who is talking. True if
	//! it went into the talk screen (then it isn't a card or a subtitle).
	//!
	//! While a greeting or a pick runs, everything else it tells this player goes into the screen
	//! too, so every reply is answered there: another title's notice (a standing change, "Nothing
	//! happens"), another speaker's line (with their name). A line for the journal (a contract, an
	//! offer, a mission's end) is said there and still a card; danger, the infection, the radio and
	//! the tips stay cards only. `spoken`: a subtitle (RZ_GameModeComponent.Say), title is a name.
	bool Capture(int playerId, string title, string text, int tag = RZ_ENoticeTag.AUTO, bool spoken = false)
	{
		RZ_TalkSession s;
		if (!m_mSessions.Find(playerId, s))
			return false;

		bool apart = StaysCard(title, tag);
		bool journal = !apart && ForJournal(title, tag);
		if (s.m_sHeader.IsEmpty() && s.m_bCapturing && !journal && title != RZ_Guidance.PLAIN_TITLE)
		{
			s.m_sHeader = title;
			s.m_Adapter.FromHeader(m_GM, s, title);
		}

		if (!s.m_sHeader.IsEmpty() && title == s.m_sHeader)
		{
			s.m_aNew.Insert(text);
			if (!s.m_bCapturing)
			{
				TakeNew(s);
				SendNode(s, GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));
			}

			return true;
		}

		if (!s.m_bCapturing || apart)
			return false;

		string line = text;
		if (spoken)
		{
			string who = title;
			int comma = who.IndexOf(", ");
			if (comma > 0)
				who = who.Substring(0, comma);

			line = who + ": " + text;
		}

		s.m_aNew.Insert(line);
		return !journal;
	}

	//------------------------------------------------------------------------------------------------
	//! Never into the talk screen: danger and the infection, the broadcast and the radio, world
	//! events and the world at spawn, and the tips ("Remember").
	protected static bool StaysCard(string title, int tag)
	{
		// A job's start card (RZ_JobCards) is a card, not something they say
		if (title == RZ_Guidance.PLAIN_TITLE || tag == RZ_ENoticeTag.START)
			return true;

		RZ_NoticeStyle st = RZ_NoticeStyle.Classify(title, tag);
		if (st.IsRadio() || st.m_iPriority == RZ_ENoticePriority.URGENT)
			return true;

		return st.m_iKind == RZ_ENoticeKind.INFECTION || st.m_iKind == RZ_ENoticeKind.EVENT || st.m_iKind == RZ_ENoticeKind.WORLD;
	}

	//------------------------------------------------------------------------------------------------
	//! Said in the talk screen and still a card, for the journal and the Radio tab: a mission's
	//! notices, contracts and off-the-books jobs, a background unlocked, anything about you now.
	protected static bool ForJournal(string title, int tag)
	{
		if (tag == RZ_ENoticeTag.MISSION || tag == RZ_ENoticeTag.HIGH)
			return true;

		string t = title;
		t.ToLower();
		return t.StartsWith("off the books:") || RZ_NoticeStyle.PriorityFor(t, RZ_ENoticeKind.PLAIN, RZ_ENoticeTag.AUTO) == RZ_ENoticePriority.HIGH;
	}

	//------------------------------------------------------------------------------------------------
	//! The player picked a reply (RZ_PlayerUIComponent.RpcAsk_TalkPick), or left (LEAVE).
	void Pick(int playerId, int serial, int replyId)
	{
		RZ_TalkSession s;
		if (!m_mSessions.Find(playerId, s))
			return;

		// Leaving is taken whatever the serial: B pressed while an answer was on its way still leaves
		if (replyId == LEAVE)
		{
			End(playerId, "", false);
			return;
		}

		if (serial != s.m_iSerial)
			return;

		RZ_TalkReply reply;
		foreach (RZ_TalkReply r : s.m_aReplies)
		{
			if (r.m_iId == replyId)
				reply = r;
		}

		// Turning a page needs no row: only more than one page
		bool paging = (replyId == MORE || replyId == PAGE_BACK) && s.m_iPages > 1;
		float now = RZ_UIW.Now();
		if ((!paging && (!reply || !reply.m_bEnabled)) || now - s.m_fLastPick < PICK_GAP)
			return;

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!user || !RZ_Util.IsCharacterAlive(user) || !guard || !guard.IsOpen(playerId, RZ_ScreenGuard.KIND_TALK))
		{
			End(playerId, "gone", true);
			return;
		}

		if (!s.m_Speaker || (!IsPerson(s.m_Speaker) && !RZ_TalkRadio.Cast(s.m_Adapter)) || RZ_Util.DistXZ(user.GetOrigin(), s.m_Speaker.GetOrigin()) > RZ_DialogueSettings.Get().m_fRange)
		{
			KeepForResume(s);
			End(playerId, "left", true);
			return;
		}

		// A change of clothes since it opened ends it (also checked on the tick)
		if (s.m_Tree && m_Trees.KitChanged(s))
		{
			string name = s.m_sName;
			End(playerId, "", true);
			m_GM.Say(playerId, name, s.m_iFaction, "Didn't you just change your clothes? Never mind. Off you go.");
			return;
		}

		s.m_fLastPick = now;
		guard.Touch(playerId);

		// The screen's own: a page on or back (round to the other end), the [Admin] list and back from it (nothing is said)
		if (paging || replyId == ADMIN || replyId == ADMIN_BACK || replyId == OTHER)
		{
			if (replyId == MORE)
				s.m_iPage = (s.m_iPage + 1) % s.m_iPages;
			else if (replyId == PAGE_BACK)
				s.m_iPage = (s.m_iPage + s.m_iPages - 1) % s.m_iPages;
			else if (replyId == OTHER)
				s.m_bAll = true;
			else
				s.m_bAdmin = replyId == ADMIN;

			SendNode(s, user);
			return;
		}

		// A built reply, once picked, shows grey for the rest of the conversation
		if (!reply.m_bAdmin && !RZ_DialogueTrees.IsOurs(replyId) && s.m_aPicked.Find(reply.m_sLabel) < 0)
			s.m_aPicked.Insert(reply.m_sLabel);

		s.m_aNew.Clear();
		s.m_bSilent = false;
		s.m_bCapturing = true;
		if (s.m_Tree && RZ_DialogueTrees.IsOurs(replyId))
		{
			bool goOn = m_Trees.Pick(s, replyId, user);
			s.m_bCapturing = false;
			if (!m_mSessions.Contains(playerId))
				return;

			if (!goOn)
			{
				Finish(s, user);
				return;
			}
		}
		else if (RZ_LongTableReach.IsOurs(replyId) && RZ_LongTableReach.Get())
		{
			// One of the Long Table's jars tested at Halcyon (World/RZ_LongTableReach.c)
			RZ_LongTableReach.Get().Pick(s, replyId, user);
		}
		else if (RZ_OpeningCase.IsOurs(replyId) && RZ_OpeningCase.Get())
		{
			// The runner's case handed over (Guidance/RZ_OpeningCase.c), whoever they are
			RZ_OpeningCase.Get().Pick(s, replyId, user);
		}
		else
		{
			s.m_Adapter.Pick(m_GM, s, playerId, replyId, user);
		}

		s.m_bCapturing = false;
		if (!m_mSessions.Contains(playerId))
			return;

		// The reply opened another screen (the shop, the reader): that one is open now, this is over
		if (!guard.IsOpen(playerId, RZ_ScreenGuard.KIND_TALK))
		{
			m_mSessions.Remove(playerId);
			RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
			if (ui)
				ui.SendPayload(RZ_UIFeed.PAYLOAD_TALK, "X|other");

			return;
		}

		// Every reply is answered in the screen, then the replies after it; back at the hub with
		// nothing new (a tree's `hub`, a leader taking over with nothing to say) there's no line
		bool backAtHub = s.m_Tree && RZ_DialogueTrees.IsOurs(replyId) && !s.m_Tree.m_bInNode;
		if (s.m_aNew.IsEmpty() && (s.m_bSilent || backAtHub))
			s.m_aSaid.Clear();
		else if (s.m_aNew.IsEmpty())
			s.m_aNew.Insert(NO_ANSWER);

		TakeNew(s);
		SendNode(s, user);
	}

	//------------------------------------------------------------------------------------------------
	//! The screen is gone: the player closed it, or the screen guard did (it told the client).
	void OnScreenGone(int playerId)
	{
		KeepForResume(GetSession(playerId));
		m_mSessions.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_mSessions.Remove(playerId);
		m_mLastTalk.Remove(playerId);
		m_mResume.Remove(playerId);
		m_Trees.GetBarks().Forget(playerId);
		m_Nametags.Forget(playerId);
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
	//! T|serial|faction|name|sub|said|page|pages, then R|id|label|tag|need|enabled|heard per reply (the
	//! last is the way out). Line breaks in what they said travel as ^. At most ROWS replies go: a page
	//! of up to 4, and the way out (RZ_TalkWords.WayOut; Back. in the [Admin] list); page is from 0.
	//! `said` may be empty: the screen opens on the replies (no filler greeting).
	protected void SendNode(RZ_TalkSession s, IEntity user)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(s.m_iPlayer);
		if (!ui)
		{
			End(s.m_iPlayer, "", false);
			return;
		}

		array<ref RZ_TalkReply> all = {};
		if (s.m_Tree && s.m_Tree.m_bInNode)
		{
			m_Trees.NodeReplies(s, all);
		}
		else
		{
			s.m_Adapter.Replies(m_GM, s, s.m_iPlayer, user, all);
			// Whoever carries the runner's case: its hand-over, at any Halcyon, Red Hand or Bastion person
			if (RZ_OpeningCase.Get())
				RZ_OpeningCase.Get().AddReplies(s, user, all);

			// Whoever carries one of the Long Table's jars: Halcyon's people will test it
			if (RZ_LongTableReach.Get())
				RZ_LongTableReach.Get().AddReplies(s, user, all, RZ_OpeningCase.FactionOf(m_GM, s));

			if (s.m_Tree)
				m_Trees.HubReplies(s, all);
		}

		// A server admin's replies go under one reply, [Admin], which opens them as their own list
		array<ref RZ_TalkReply> rows = {};
		array<ref RZ_TalkReply> admin = {};
		bool inNode = s.m_Tree && s.m_Tree.m_bInNode;
		foreach (RZ_TalkReply r : all)
		{
			if (r.m_iRank < 0)
				r.m_iRank = RZ_TalkNeeds.RankFor(r.m_sLabel);

			if (!r.m_bOwn)
			{
				RZ_TalkNeeds.Apply(m_GM, s.m_iPlayer, user, r);
				RZ_TalkWords.Apply(s, r);	// what you'd say, not the action's name
			}

			if (r.m_bAdmin)
				admin.Insert(r);
			else
				rows.Insert(r);
		}

		// The most pertinent first (docs/DIALOGUE.md, "The order of the replies"): a hub's replies by
		// rank, keeping their order within a rank; a node's choices stay as written
		if (!inNode)
			ByRank(rows);

		if (admin.IsEmpty())
			s.m_bAdmin = false;

		// Ended by a tree: their last words and only the way out
		if (s.m_bEnded)
		{
			rows.Clear();
			admin.Clear();
			s.m_bAdmin = false;
		}

		// The job you're tracking has replies here: only those, and "There was something else." for the rest
		// (docs/DIALOGUE.md, "The order of the replies")
		bool focus = !inNode && !s.m_bAdmin && !s.m_bAll && Focus(s, rows);

		RZ_TalkReply way = OwnRow(LEAVE, RZ_TalkWords.WayOut(s));
		string listKey = "hub";
		if (focus)
			listKey = "tracked";
		if (s.m_bAdmin)
		{
			rows.Clear();
			foreach (RZ_TalkReply a : admin)
			{
				rows.Insert(a);
			}

			way = OwnRow(ADMIN_BACK, "Back.");
			listKey = "admin";
		}
		else
		{
			if (s.m_Tree && s.m_Tree.m_bInNode && s.m_Tree.m_Conv && s.m_Tree.m_Node)
				listKey = s.m_Tree.m_Conv.m_sId + "/" + s.m_Tree.m_Node.m_sId;

			if (!admin.IsEmpty())
				rows.Insert(OwnRow(ADMIN, "[Admin]"));
		}

		// A new list starts on its first page
		if (listKey != s.m_sList)
		{
			s.m_sList = listKey;
			s.m_iPage = 0;
		}

		s.m_aReplies.Clear();
		int per = ROWS - 1;
		s.m_iPages = Math.Max(1, (rows.Count() + per - 1) / per);
		if (s.m_iPage >= s.m_iPages || s.m_iPage < 0)
			s.m_iPage = 0;

		int from = s.m_iPage * per;
		for (int k = from; k < from + per && k < rows.Count(); k++)
		{
			s.m_aReplies.Insert(rows[k]);
		}

		s.m_aReplies.Insert(way);
		foreach (RZ_TalkReply shown : s.m_aReplies)
		{
			shown.m_bHeard = Heard(s, shown);
		}

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
			+ sep + RZ_UIText.Clean(s.m_sSub, 120) + sep + RZ_UIText.Clean(said, MAX_SAID)
			+ sep + s.m_iPage.ToString() + sep + s.m_iPages.ToString();
		foreach (RZ_TalkReply rep : s.m_aReplies)
		{
			int on = 0;
			if (rep.m_bEnabled)
				on = 1;

			int heard = 0;
			if (rep.m_bHeard)
				heard = 1;

			payload += "\nR" + sep + rep.m_iId.ToString() + sep + RZ_UIText.Clean(rep.m_sLabel, 120) + sep + RZ_UIText.Clean(rep.m_sTag, 40)
				+ sep + RZ_UIText.Clean(rep.m_sNeed, 80) + sep + on.ToString() + sep + heard.ToString();
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_TALK, payload); // VERIFY: an owner-only string RPC of up to about 2,000 characters arrives whole
	}

	//------------------------------------------------------------------------------------------------
	//! The tracked job's replies alone, then "There was something else." False (rows untouched): you track
	//! nothing, or nothing here moves it.
	protected bool Focus(RZ_TalkSession s, notnull array<ref RZ_TalkReply> rows)
	{
		int tracked = m_GM.GetProfile(s.m_iPlayer).m_iTrackedId;
		if (tracked == 0)
			return false;

		array<ref RZ_TalkReply> mine = {};
		foreach (RZ_TalkReply r : rows)
		{
			if (r.m_iFor == tracked)
				mine.Insert(r);
		}

		if (mine.IsEmpty())
			return false;

		// Nothing else to show: no such row
		bool others = mine.Count() < rows.Count();
		rows.Clear();
		foreach (RZ_TalkReply keep : mine)
		{
			rows.Insert(keep);
		}

		if (others)
			rows.Insert(OwnRow(OTHER, "There was something else."));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! An errand's row on the Quests tab, for RZ_TalkReply.m_iFor; 0 for none (an id below 0).
	static int ForErrand(int errandId)
	{
		if (errandId < 0)
			return 0;

		return -(RZ_UIFeed.ERRAND_ID_BASE + errandId);
	}

	//------------------------------------------------------------------------------------------------
	//! A mission's row on the Quests tab, for RZ_TalkReply.m_iFor: a questline's stage is its
	//! questline's row (RZ_QuestTracker.IsStageOfListedRun).
	static int ForMission(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (!m)
			return 0;

		if (RZ_QuestTracker.IsStageOfListedRun(gm, m, playerId))
			return -m.m_iQuestRun;

		return m.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! Stable sort by rank (RZ_TalkReply.RANK_*): jobs you're on, new jobs, locked jobs, trade and
	//! services, information, small talk. Insertion sort: a hub has a few dozen replies at most.
	protected static void ByRank(notnull array<ref RZ_TalkReply> rows)
	{
		for (int i = 1; i < rows.Count(); i++)
		{
			int j = i - 1;
			while (j >= 0 && rows[j].m_iRank > rows[j + 1].m_iRank)
			{
				rows.SwapItems(j, j + 1);
				j--;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One of the screen's own rows (the way out, [Admin], Back.).
	protected static RZ_TalkReply OwnRow(int id, string label)
	{
		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = id;
		r.m_sLabel = label;
		r.m_bOwn = true;
		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Nothing new behind a reply: a tree's topic or choice whose nodes you've seen through (the
	//! trees' memory), a built reply already picked in this conversation. The screen's own rows and
	//! the admins' never are.
	protected bool Heard(RZ_TalkSession s, RZ_TalkReply r)
	{
		// A job on offer is never grey: it's always something new to take
		if (r.m_bAdmin || r.m_bJob || r.m_iId == LEAVE || r.m_iId == ADMIN || r.m_iId == ADMIN_BACK || r.m_iId == OTHER)
			return false;

		if (RZ_DialogueTrees.IsOurs(r.m_iId))
			return s.m_Tree && m_Trees.Heard(s, r.m_iId);

		return s.m_aPicked.Find(r.m_sLabel) >= 0;
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

		if ((label == "Share some food" || label == "Hand over food") && RZ_HavenQuest.CountFood(gm, user) <= 0)
		{
			r.m_bEnabled = false;
			r.m_sNeed = "you have nothing to eat";
			return;
		}

		// A safehouse's (the Haven quartermaster's, the medic's): only for the bitten
		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		if (label == "Tell them you've been bitten" && profile && !profile.IsInfected())
		{
			r.m_bEnabled = false;
			r.m_sNeed = "you haven't been bitten";
			return;
		}

		if (r.m_sTag.IsEmpty())
			r.m_sTag = TagFor(label);
	}

	//------------------------------------------------------------------------------------------------
	//! Where a built reply goes in the list when its adapter didn't say (docs/DIALOGUE.md, "The order
	//! of the replies"): asking for news or what they think is information; everything else a built
	//! reply does (trade, a service, food, an antidote, a ride) is a service.
	static int RankFor(string label)
	{
		string l = label;
		l.ToLower();
		// A wanderer's job asked in one step (the Courier's letters, the Mechanic's car keys)
		if (l == "ask for work" || l == "ask about cars")
			return RZ_TalkReply.RANK_NEW;

		if (l == "heard anything?" || l == "what do you make of me?" || l.StartsWith("ask how things are") || l.StartsWith("ask about ")
			|| l == "read the notice board" || l == "about the season" || l == "where should i look?" || l == "the dead"
			|| l == "what you didn't tell me" || l == "the cure" || l == "this place" || l == "me")
			return RZ_TalkReply.RANK_INFO;

		return RZ_TalkReply.RANK_GOODS;
	}

	//------------------------------------------------------------------------------------------------
	//! A dim tag on what a reply is. A reply that starts a job is tagged by its kind ([Job]) by
	//! whoever builds it (RZ_TalkJobs); the Courier's "Ask for work" by its label here.
	static string TagFor(string label)
	{
		string l = label;
		l.ToLower();
		if (l.Contains("work") || l == "ask about cars")
			return "[Job]";
		if (l.Contains("news") || l.Contains("heard anything") || l.Contains("how things are") || l.Contains("rumour"))
			return "News";
		if (l.StartsWith("rob "))
			return "Haven Infamy";
		if (l.StartsWith("trade") || l.StartsWith("buy"))
			return "Trade";

		return "";
	}
}
