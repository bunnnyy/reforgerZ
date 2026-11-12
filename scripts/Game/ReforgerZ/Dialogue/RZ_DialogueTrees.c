//! Conversation trees on the talk screen (docs/DIALOGUE.md). Server only.
//!
//! With Dialogue > Trees on (off by default), Talk still runs the person's built greeting and
//! their built replies stay as the hub's verbs. On top come the hub's topics: every conversation
//! for that speaker whose `when` holds. A topic opens a short tree of nodes; a node's choices show
//! what they need before you pick (a check's score against its difficulty, a price, a background),
//! and nothing is rolled. Talk never pays Fame and never gives items; it pays at most a little
//! Favor inside one daily budget per faction. With Trees off, nothing here runs and the talk screen
//! is exactly the built one.

//------------------------------------------------------------------------------------------------
//! One session's tree state (RZ_TalkSession.m_Tree).
class RZ_DTalk
{
	ref RZ_DCtx m_Ctx;
	RZ_DConv m_Conv;				// weak: the database owns it
	RZ_DNode m_Node;
	string m_sPrev;
	bool m_bInNode;
	int m_iPage;
	ref array<RZ_DConv> m_aTopics = {};
	ref array<string> m_aHubSaid = {};	// what they said at the hub, to show again
	int m_iFavorPaid;				// this conversation (10 at most)
	float m_fNodeAt;
	string m_sLabel;				// the choice being landed (a promise's words)
	int m_iKit;						// what the player wore when it opened (a change ends it)
}

//------------------------------------------------------------------------------------------------
class RZ_DialogueTrees
{
	static const int MORE = 98;
	static const int TOPIC_BASE = 100;
	static const int CHOICE_BASE = 200;
	protected static const int MAX_REPLIES = 6;
	protected static const int FAVOR_PER_CONV = 10;
	protected static const float FRESH_NODE = 1.5;
	protected static const int MAX_JUMPS = 8;

	protected static RZ_DialogueTrees s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected ref RZ_DialogueDB m_DB;
	protected ref RZ_DialogueMemory m_Memory;
	protected ref RZ_RepSeason m_Record;
	protected ref RZ_DialogueBarks m_Barks;
	protected ref RZ_DialogueOpens m_Opens = new RZ_DialogueOpens();

	//------------------------------------------------------------------------------------------------
	void RZ_DialogueTrees(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		m_DB = new RZ_DialogueDB();
		m_Memory = new RZ_DialogueMemory(gm);
		m_Record = new RZ_RepSeason(gm);
		m_Barks = new RZ_DialogueBarks(gm, this);
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_DialogueTrees()
	{
		if (s_Instance == this)
			s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_DialogueTrees Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! At start, after the season is known.
	void Load()
	{
		m_DB.Load();
		m_Memory.Load();
		m_Record.Load();
		RZ_Log.Info(string.Format("Dialogue: trees %1", OnOff(IsOn())));
	}

	//------------------------------------------------------------------------------------------------
	//! On the server tick (RZ_Dialogue.Tick): the season record always; with Trees on, the promise
	//! and callback clocks of the players online, and the barks.
	void Tick(float dt)
	{
		m_Record.Tick(dt);
		if (!IsOn())
			return;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			m_Memory.TickPlayer(id, dt);
		}

		m_Barks.Tick(dt);
		RZ_DialogueMakeRight.Tick(m_GM, dt);
		RZ_DialogueStory.Tick(m_GM, dt);
		RZ_DialogueBinds.Tick(m_GM);
		m_Opens.Tick(m_GM, dt);
	}

	//------------------------------------------------------------------------------------------------
	//! Trees are used: Dialogue > Trees on, and the talk screen in use.
	bool IsOn()
	{
		return RZ_DialogueSettings.Get().m_bTrees && RZ_Dialogue.IsOn();
	}

	//------------------------------------------------------------------------------------------------
	RZ_DialogueMemory GetMemory() { return m_Memory; }
	RZ_DialogueDB GetDB() { return m_DB; }

	//------------------------------------------------------------------------------------------------
	void SaveIfChanged()
	{
		m_Memory.SaveIfChanged();
		m_Record.SaveIfChanged();
	}

	// ================================================================ A session
	//------------------------------------------------------------------------------------------------
	//! After the adapter's setup: the speaker as conditions see them. Null when trees are off or
	//! the adapter can't say who it is.
	RZ_DTalk Begin(RZ_TalkSession s, IEntity user)
	{
		if (!IsOn() || !s.m_Adapter)
			return null;

		RZ_DCtx ctx = DescribeFor(s, user);
		if (!ctx)
			return null;

		RZ_DTalk t = new RZ_DTalk();
		t.m_Ctx = ctx;
		t.m_iKit = KitNow(s.m_iPlayer);

		// Talking to their people undisguised: they know your face this life
		if (RZ_Factions.IsValid(ctx.m_iFaction) && !InTheirKit(s.m_iPlayer, ctx.m_iFaction))
			m_Memory.NoteFace(s.m_iPlayer, ctx.m_iFaction);

		// A promise to come back is kept by coming back
		m_Memory.KeepVisits(s.m_iPlayer, ctx.m_sKey);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! The speaker as conditions see them (the adapter's Describe, the town and place filled in), or
	//! null when the adapter can't say who it is. Also used for barks.
	RZ_DCtx DescribeFor(RZ_TalkSession s, IEntity user)
	{
		RZ_DCtx ctx = new RZ_DCtx();
		ctx.m_GM = m_GM;
		ctx.m_iPlayer = s.m_iPlayer;
		ctx.m_User = user;
		ctx.m_Body = s.m_Speaker;
		ctx.m_iFaction = s.m_iFaction;
		if (!s.m_Adapter.Describe(m_GM, s, ctx) || ctx.m_aKinds.IsEmpty())
			return null;

		if (ctx.m_iTown < 0)
			ctx.m_iTown = m_GM.FindNearestSettlement(ctx.Pos());

		RZ_Settlement town = m_GM.GetSettlement(ctx.m_iTown);
		if (ctx.m_sPlace.IsEmpty() && town)
			ctx.m_sPlace = town.m_sName;

		return ctx;
	}

	//------------------------------------------------------------------------------------------------
	//! What the player wears, as one number (the faction whose kit and its level): a change ends a
	//! conversation, so nobody opens it in one outfit and picks in another.
	int KitNow(int playerId)
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		if (!hostility || !hostility.GetDisguise().IsOn())
			return -1;

		RZ_Disguise d = hostility.GetDisguise();
		int f = d.Wearing(playerId);
		if (f < 0)
			return -1;

		return f * 10 + d.KitLevel(playerId, f);
	}

	//------------------------------------------------------------------------------------------------
	//! The player changed clothes since the conversation opened (or since the last pick that changed
	//! them). A catch taking the kit isn't a change of clothes: that's accepted as the new kit.
	bool KitChanged(RZ_TalkSession s)
	{
		RZ_DTalk t = s.m_Tree;
		int now = KitNow(s.m_iPlayer);
		if (!t || now == t.m_iKit)
			return false;

		RZ_Hostility hostility = m_GM.GetHostility();
		int was = t.m_iKit / 10;
		if (t.m_iKit >= 0 && hostility && hostility.GetDisguise().IsBlown(s.m_iPlayer, was))
		{
			t.m_iKit = now;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool InTheirKit(int playerId, int f)
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		return hostility && hostility.GetDisguise().InTheirKit(playerId, f);
	}

	//------------------------------------------------------------------------------------------------
	RZ_DialogueBarks GetBarks() { return m_Barks; }
	RZ_RepSeason GetRecord() { return m_Record; }

	//------------------------------------------------------------------------------------------------
	//! A greet whose `when` holds: its lines go before the built greeting.
	void Opening(RZ_TalkSession s, notnull array<string> lines)
	{
		RZ_DTalk t = s.m_Tree;
		array<RZ_DConv> all = {};
		m_DB.ForSpeaker(t.m_Ctx.m_aKinds, all);
		RZ_DConv best;
		foreach (RZ_DConv c : all)
		{
			if (!c.IsGreet() || !RZ_DialogueCond.Holds(t.m_Ctx, c.m_When) || !TopicFree(t, c))
				continue;

			if (!best || c.m_iPrio < best.m_iPrio)
				best = c;
		}

		if (best && !best.m_aNodes.IsEmpty())
		{
			NoteTopic(t, best);
			Lines(t, best.m_aNodes[0], lines);
		}

		// What's still promised to this speaker, with the play left
		m_Memory.OpenLines(s.m_iPlayer, t.m_Ctx.m_sKey, lines);
	}

	//------------------------------------------------------------------------------------------------
	//! Back into a node a moment after danger or distance ended the conversation (no entry effects
	//! run again). False if the conversation no longer holds.
	bool Resume(RZ_TalkSession s, string convId, string nodeId, notnull array<string> lines)
	{
		RZ_DTalk t = s.m_Tree;
		RZ_DConv c = m_DB.Find(convId);
		if (!t || !c || !RZ_DialogueCond.Holds(t.m_Ctx, c.m_When) || Refuses(t.m_Ctx))
			return false;

		RZ_DNode node = c.GetNode(nodeId);
		if (!node)
			return false;

		t.m_Conv = c;
		t.m_Node = node;
		t.m_bInNode = true;
		t.m_sPrev = "";
		t.m_fNodeAt = RZ_UIW.Now();
		t.m_Ctx.m_sScene = RZ_DialogueScenes.SceneKey(c, t.m_Ctx);
		LinesFor(t.m_Ctx, node, lines);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The hub: the built replies (verbs) are already in `replies`; the topics follow, a page at a
	//! time, with More... when they don't fit.
	void HubReplies(RZ_TalkSession s, notnull array<ref RZ_TalkReply> replies)
	{
		RZ_DTalk t = s.m_Tree;
		RZ_DialogueScenes scenes = Scenes();
		if (scenes)
			scenes.HubVerbs(s, replies);

		RZ_DialogueMakeRight.Verb(s, replies);

		CollectTopics(t);
		while (replies.Count() > MAX_REPLIES - 1 && !t.m_aTopics.IsEmpty())
		{
			replies.Remove(replies.Count() - 1);
		}

		int room = MAX_REPLIES - replies.Count();
		if (t.m_aTopics.IsEmpty() || room <= 0)
			return;

		bool paged = t.m_aTopics.Count() > room;
		int shown = room;
		if (paged)
			shown = room - 1;

		if (shown <= 0)
			shown = 1;

		int pages = Math.Max(1, (t.m_aTopics.Count() + shown - 1) / shown);
		if (t.m_iPage >= pages)
			t.m_iPage = 0;

		int from = t.m_iPage * shown;
		for (int i = from; i < from + shown && i < t.m_aTopics.Count(); i++)
		{
			RZ_DConv c = t.m_aTopics[i];
			RZ_TalkReply r = new RZ_TalkReply();
			r.m_iId = TOPIC_BASE + i;
			r.m_sLabel = RZ_DialogueText.Fill(t.m_Ctx, c.m_sLabel);
			r.m_bOwn = true;
			if (m_Memory.Has(s.m_iPlayer, RZ_DialogueMemory.SEEN, c.m_sId + "/start"))
				r.m_sTag = "heard";

			replies.Insert(r);
		}

		if (paged && replies.Count() < MAX_REPLIES)
		{
			RZ_TalkReply more = new RZ_TalkReply();
			more.m_iId = MORE;
			more.m_sLabel = "Something else.";
			more.m_bOwn = true;
			replies.Insert(more);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CollectTopics(RZ_DTalk t)
	{
		t.m_aTopics.Clear();
		// A refusing speaker still offers its `always` topics (the finale's route topics)
		bool refuses = Refuses(t.m_Ctx);
		array<RZ_DConv> all = {};
		m_DB.ForSpeaker(t.m_Ctx.m_aKinds, all);
		for (int prio = 0; prio <= 4; prio++)
		{
			array<RZ_DConv> band = {};
			foreach (RZ_DConv c : all)
			{
				if (refuses && (c.m_iFlags & RZ_DConv.FLAG_ALWAYS) == 0)
					continue;

				if (c.m_iPrio == prio && !c.IsGreet() && !c.IsBark() && TopicFree(t, c) && RZ_DialogueCond.Holds(t.m_Ctx, c.m_When))
					band.Insert(c);
			}

			// Flavour rotates on the speaker and the day, so the same few don't always lead
			int start = 0;
			if (prio == 4 && band.Count() > 1)
				start = Math.AbsInt(t.m_Ctx.m_sKey.Hash() + RZ_SeasonClock.Day()) % band.Count();

			for (int k = 0; k < band.Count(); k++)
			{
				t.m_aTopics.Insert(band[(start + k) % band.Count()]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The built refusals come first: a leader or wanderer at attitude -2 or below, an exile at a
	//! Haven speaker, a Hunted player with a faction's people, someone the Long Table has crossed.
	//! They get the built greeting (the refusal) and no topics.
	bool Refuses(RZ_DCtx ctx)
	{
		int pid = ctx.m_iPlayer;
		if ((ctx.m_Leader || ctx.m_Wanderer) && Attitude(ctx) <= -2)
			return true;

		if (ctx.m_iFaction == RZ_EFaction.SURVIVORS && m_GM.GetSafehouses() && m_GM.GetSafehouses().IsExiled(pid))
			return true;

		if (RZ_Factions.IsValid(ctx.m_iFaction) && RZ_Reputation.IsHunted(m_GM.GetTier(pid, ctx.m_iFaction)))
			return true;

		RZ_LongTable table = m_GM.GetLongTable();
		if (ctx.m_iTableRole >= 0 && table && table.Remembers(pid, RZ_LongTable.MEM_CROSSED) && !table.Remembers(pid, RZ_LongTable.MEM_FAMILY))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Once: never again this life at this speaker. Daily: once a real day.
	protected bool TopicFree(RZ_DTalk t, RZ_DConv c)
	{
		return FreeFor(t.m_Ctx, c);
	}

	//------------------------------------------------------------------------------------------------
	bool FreeFor(RZ_DCtx ctx, RZ_DConv c)
	{
		if ((c.m_iFlags & (RZ_DConv.FLAG_ONCE | RZ_DConv.FLAG_DAILY)) == 0)
			return true;

		int at = m_Memory.TopicAt(ctx.m_iPlayer, ctx.m_sKey + "|" + c.m_sId);
		if (at <= 0)
			return true;

		if ((c.m_iFlags & RZ_DConv.FLAG_ONCE) != 0)
			return false;

		return RZ_SeasonClock.Now() - at >= RZ_DialogueMemory.DAY;
	}

	//------------------------------------------------------------------------------------------------
	protected void NoteTopic(RZ_DTalk t, RZ_DConv c)
	{
		NoteFor(t.m_Ctx, c);
	}

	//------------------------------------------------------------------------------------------------
	void NoteFor(RZ_DCtx ctx, RZ_DConv c)
	{
		if ((c.m_iFlags & (RZ_DConv.FLAG_ONCE | RZ_DConv.FLAG_DAILY)) != 0)
			m_Memory.NoteTopic(ctx.m_iPlayer, ctx.m_sKey + "|" + c.m_sId);
	}

	//------------------------------------------------------------------------------------------------
	//! Our reply ids: topics, choices and More.
	static bool IsOurs(int replyId)
	{
		return replyId == MORE || replyId == RZ_DialogueMakeRight.VERB || replyId >= TOPIC_BASE || RZ_DialogueScenes.IsOurs(replyId);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_DialogueScenes Scenes()
	{
		RZ_Dialogue dialogue = m_GM.GetDialogue();
		if (!dialogue)
			return null;

		return dialogue.GetScenes();
	}

	//------------------------------------------------------------------------------------------------
	//! A node's choices, each with what it needs (no dice: a check shows its score).
	void NodeReplies(RZ_TalkSession s, notnull array<ref RZ_TalkReply> replies)
	{
		RZ_DTalk t = s.m_Tree;
		if (!t.m_Node)
			return;

		foreach (int i, RZ_DChoice c : t.m_Node.m_aChoices)
		{
			if (!RZ_DialogueCond.Holds(t.m_Ctx, c.m_If) || !CalloutOpen(t.m_Ctx, c))
				continue;

			RZ_TalkReply r = new RZ_TalkReply();
			r.m_iId = CHOICE_BASE + i;
			r.m_sLabel = RZ_DialogueText.Fill(t.m_Ctx, c.m_sLabel);
			r.m_bOwn = true;
			Tag(t, c, r);
			replies.Insert(r);
			if (replies.Count() >= MAX_REPLIES)
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! [Evidence]: a call-out shows only to a player who was told that lie and knows the truth.
	protected bool CalloutOpen(RZ_DCtx ctx, RZ_DChoice c)
	{
		if (c.m_sCallout.IsEmpty())
			return true;

		string key = Scoped(ctx, c.m_sCallout);
		return m_Memory.HeardLie(ctx.m_iPlayer, key) && m_Memory.Knows(ctx.m_iPlayer, key);
	}

	// ================================================================ Picks
	//------------------------------------------------------------------------------------------------
	//! A topic, More or a choice. Lines go into s.m_aNew; false ends the conversation.
	bool Pick(RZ_TalkSession s, int replyId, IEntity user)
	{
		RZ_DTalk t = s.m_Tree;
		t.m_Ctx.m_User = user;
		if (replyId == MORE)
		{
			t.m_iPage++;
			return true;
		}

		if (replyId == RZ_DialogueMakeRight.VERB)
		{
			RZ_DialogueMakeRight.Pick(s);
			if (s.m_aNew.IsEmpty())
				s.m_aNew.InsertAll(s.m_aSaid);

			return true;
		}

		if (RZ_DialogueScenes.IsOurs(replyId))
		{
			RZ_DialogueScenes scenes = Scenes();
			if (scenes)
				scenes.PickVerb(s, replyId, user);

			if (s.m_aNew.IsEmpty())
				s.m_aNew.InsertAll(s.m_aSaid);

			return true;
		}

		if (replyId >= CHOICE_BASE)
			return PickChoice(s, replyId - CHOICE_BASE, user);

		int idx = replyId - TOPIC_BASE;
		if (idx < 0 || idx >= t.m_aTopics.Count())
			return true;

		RZ_DConv c = t.m_aTopics[idx];
		if (!RZ_DialogueCond.Holds(t.m_Ctx, c.m_When))
		{
			s.m_aNew.Insert("They didn't catch that.");
			return true;
		}

		t.m_aHubSaid.Clear();
		t.m_aHubSaid.InsertAll(s.m_aSaid);
		t.m_Conv = c;
		t.m_Ctx.m_sScene = RZ_DialogueScenes.SceneKey(c, t.m_Ctx);
		t.m_sPrev = "";
		t.m_iFavorPaid = 0;
		RZ_DialogueBinds.Pick(t.m_Ctx, c);
		NoteTopic(t, c);
		array<string> none = {};
		EnterNode(s, "start", none, 0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool PickChoice(RZ_TalkSession s, int idx, IEntity user)
	{
		RZ_DTalk t = s.m_Tree;
		if (!t.m_bInNode || !t.m_Node || idx < 0 || idx >= t.m_Node.m_aChoices.Count())
			return true;

		RZ_DChoice c = t.m_Node.m_aChoices[idx];
		RZ_DCtx ctx = t.m_Ctx;
		int pid = s.m_iPlayer;

		// The live world again: the choice's own condition and the conversation's
		if (!RZ_DialogueCond.Holds(ctx, c.m_If) || !RZ_DialogueCond.Holds(ctx, t.m_Conv.m_When) || !CalloutOpen(ctx, c))
		{
			s.m_aNew.Insert("They didn't catch that.");
			s.m_aNew.InsertAll(s.m_aSaid);
			return true;
		}

		// A price is never taken off a label that hasn't refreshed yet
		if (!c.m_sCostKind.IsEmpty() && RZ_UIW.Now() - t.m_fNodeAt < FRESH_NODE)
			return true;

		// The finale's lock: only from a [Point of no return] node, and only if the finale says yes
		string commitTo = CommitRoute(c);
		if (!commitTo.IsEmpty())
		{
			string no = "\"Not here.\"";
			if (t.m_Node.m_bConfirm && RZ_CureFinale.Get())
				no = RZ_CureFinale.Get().CanCommit(pid, RZ_CureFinale.RouteOfWord(commitTo));

			if (!no.IsEmpty() || RZ_UIW.Now() - t.m_fNodeAt < FRESH_NODE)
			{
				if (!no.IsEmpty())
					s.m_aNew.Insert(no);

				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}
		}

		if (!Pay(t, c, user))
		{
			s.m_aNew.Insert("You can't pay that.");
			s.m_aNew.InsertAll(s.m_aSaid);
			return true;
		}

		RZ_ECheck result = RZ_ECheck.PASS;
		string verdict = "";
		if (c.m_sCheck == "persuade")
		{
			string key = ctx.m_sKey + "|" + t.m_Conv.m_sId + "/@" + c.m_sId;
			if (!m_Memory.Has(pid, RZ_DialogueMemory.PASSED, key))
			{
				int score = TrustScore(ctx, c);
				int failedAt = m_Memory.FailedAt(pid, key);
				if (failedAt > -100 && score <= failedAt)
				{
					s.m_aNew.Insert("\"You've said that already.\"");
					s.m_aNew.InsertAll(s.m_aSaid);
					return true;
				}

				result = RZ_Checks.Result(score, Difficulty(ctx, c));
				if (result == RZ_ECheck.PASS)
					m_Memory.Add(pid, RZ_DialogueMemory.PASSED, key);
				else
					m_Memory.NoteFail(pid, key, score);
			}

			verdict = PersuadeVerdict(result);
		}
		else if (c.m_sCheck == "threaten")
		{
			if (Passing(pid, c.m_iCheckFaction))
			{
				s.m_aNew.Insert("(You can't lean on them while you're passing as one of them.)");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			RZ_WorldFacts facts = m_GM.GetFacts();
			if (facts && facts.Flag("threat." + ctx.m_sKey) > 0)
			{
				s.m_aNew.Insert("\"Already scared. Of someone else.\"");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			// Free-standing talk threats: never at a safehouse or inside a stronghold's or the lab's wire
			string here = "";
			if (user)
				here = RZ_Checks.ThreatRefusedAt(m_GM, user.GetOrigin());

			if (!here.IsEmpty())
			{
				s.m_aNew.Insert(here);
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			// Never during a parley: you came to talk
			RZ_Checkpoints parleys = m_GM.GetCheckpoints();
			if (parleys && parleys.GetParley() && parleys.GetParley().IsRunning(pid))
			{
				s.m_aNew.Insert("(Not now. You came here to talk.)");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			result = RZ_Checks.Threaten(m_GM, pid, ctx.m_sKey, c.m_iCheckFaction, -1, Difficulty(ctx, c), ctx.m_bStronghold, RZ_Checks.THREAT_INFAMY, c.m_sFavoured);
			m_Memory.AddRegard(pid, ctx.m_sKey, "threatened");

			// A threatened leader or wanderer remembers being crossed (this life)
			string threatened = m_GM.GetProfile(pid).m_sIdentity;
			if (ctx.m_Leader)
				ctx.m_Leader.Remember(threatened, RZ_Leaders.MEM_CROSSED);
			else if (ctx.m_Wanderer)
				ctx.m_Wanderer.Remember(threatened, RZ_Wanderers.MEM_WRONGED);
			verdict = FearVerdict(result);
		}
		else if (c.m_sCheck == "lie" && !IsTrue(ctx, c))
		{
			if (LieBarred(ctx))
			{
				s.m_aNew.Insert("\"Not again. Not today.\"");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			result = TellLie(s, c);
			verdict = LieVerdict(result);
		}
		else if (c.m_sCheck == "look")
		{
			string cannot = CannotLook(ctx, c);
			if (!cannot.IsEmpty())
			{
				s.m_aNew.Insert("(" + cannot + ")");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			// One claim per speaker per life: they remember who said they were one of them
			if (m_Memory.Has(pid, RZ_DialogueMemory.NOTES, ctx.m_sKey + "|claimed"))
			{
				s.m_aNew.Insert("\"You told me that already. I'm still thinking about it.\"");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			m_Memory.Add(pid, RZ_DialogueMemory.NOTES, ctx.m_sKey + "|claimed");
			result = Look(s, c);
			verdict = LookVerdict(ctx, c, result);
		}

		// A deciding pick in a scene waits 5 s for objections before it lands
		if (HasEffect(c.m_aDo, "decide") && !ctx.m_sScene.IsEmpty())
		{
			if (t.m_Conv && t.m_Conv.m_SceneWho && !RZ_DialogueCond.Holds(ctx, t.m_Conv.m_SceneWho))
			{
				s.m_aNew.Insert("\"It's not your call. Not here.\"");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			RZ_WorldFacts sceneFacts = m_GM.GetFacts();
			if (sceneFacts && sceneFacts.Flag("scene." + ctx.m_sScene) > 0)
			{
				s.m_aNew.Insert("\"It's been settled. You weren't here for it.\"");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}

			RZ_DialogueScenes scenes = Scenes();
			if (scenes && result == RZ_ECheck.PASS)
			{
				if (!scenes.StartHold(s, ctx.m_sScene, idx, result, verdict, RZ_DialogueText.Fill(ctx, c.m_sLabel)))
				{
					s.m_aNew.Insert("\"Give it a moment. Someone's answering already.\"");
					s.m_aNew.InsertAll(s.m_aSaid);
					return true;
				}

				s.m_aNew.Insert("You're about to answer for everyone here. Anyone can object in the next few seconds.");
				s.m_aNew.InsertAll(s.m_aSaid);
				return true;
			}
		}

		return Land(s, c, result, verdict, user);
	}

	//------------------------------------------------------------------------------------------------
	//! A deciding pick's hold ran out unobjected (RZ_DialogueScenes): it lands now.
	bool FinishPick(RZ_TalkSession s, int idx, RZ_ECheck result, string verdict, IEntity user)
	{
		RZ_DTalk t = s.m_Tree;
		if (!t || !t.m_bInNode || !t.m_Node || idx < 0 || idx >= t.m_Node.m_aChoices.Count())
			return true;

		return Land(s, t.m_Node.m_aChoices[idx], result, verdict, user);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasEffect(array<ref RZ_DEffect> effects, string name)
	{
		foreach (RZ_DEffect e : effects)
		{
			if (e.m_sName == name)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A [Lie] (check lie): a tell catches it first, whatever the score (the bite showing, another
	//! faction's colours, a different claim already told this speaker); then Trust against the
	//! difficulty, +1 under a vouch. Believed: a false claim in the lie store, found out later by the
	//! rumour rounds. Caught now: regard *lied*, +2 Infamy, the liar count.
	protected RZ_ECheck TellLie(RZ_TalkSession s, RZ_DChoice c)
	{
		RZ_DTalk t = s.m_Tree;
		RZ_DCtx ctx = t.m_Ctx;
		int pid = s.m_iPlayer;
		RZ_DialogueScenes scenes = Scenes();
		int bonus = 0;
		if (scenes)
			bonus = scenes.VouchBonus(pid);

		RZ_ECheck result = RZ_Checks.Lie(m_GM, pid, c.m_iCheckFaction, LieDifficulty(ctx, c, true), HasTell(ctx) || c.m_bTellAlways, bonus);
		RZ_WorldFacts facts = m_GM.GetFacts();
		string key = "talk." + t.m_Conv.m_sId + "/@" + c.m_sId;
		if (result == RZ_ECheck.PASS)
		{
			RZ_Lie lie;
			if (facts)
			{
				lie = facts.Memory().Lies().Tell(m_GM, pid, key, ctx.m_sKey, c.m_iCheckFaction, ctx.m_sKey, ctx.Pos(), RZ_ELieTest.RUMOUR, 2,
					string.Format("%1 found out what you told them. It'll come up next time.", ctx.m_sName));
				if (lie && ctx.m_Leader && ctx.m_Leader.HasTrait(RZ_ELeaderTrait.TALKER))
					lie.talker = true;

				if (lie && scenes)
					facts.Memory().Lies().Vouched(lie, scenes.Voucher(pid));
			}

			if (scenes)
				scenes.NoteClaim(s, lie);

			if (facts && !ctx.m_sScene.IsEmpty())
			{
				facts.SetFlag("scene_lied." + ctx.m_sScene, 1);
				facts.Memory().Record("scene_lied." + ctx.m_sScene, ctx.m_sKey, m_GM.GetProfile(pid).m_sIdentity, RZ_WorldMemory.SOURCE_CHOICE, false);
			}

			return result;
		}

		// Caught now: no more lies at this speaker for a real day
		m_Memory.AddRegard(pid, ctx.m_sKey, "lied", LieWeight(ctx));
		m_Memory.NoteTopic(pid, ctx.m_sKey + "|lie_caught");
		if (RZ_Factions.IsValid(c.m_iCheckFaction))
		{
			m_GM.AddInfamy(pid, c.m_iCheckFaction, 2, "a lie, caught", RZ_RepFlag.ACT);
			RZ_Deeds.AddLifeRecord(m_GM.GetProfile(pid), "lied." + c.m_iCheckFaction.ToString(), true);
		}

		RZ_Log.Info(string.Format("Story: lie %1 caught now by %2", key, ctx.m_sKey));
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! A tell: the bite showing, another faction's colours on you, or a claim already told this speaker.
	protected bool HasTell(RZ_DCtx ctx)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(ctx.m_iPlayer);
		if (profile.m_iInfectionStage >= RZ_EInfectionStage.WEAKNESS)
			return true;

		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (f != ctx.m_iFaction && hostility.GetDisguise().WearsColours(ctx.m_iPlayer, f))
					return true;
			}
		}

		return m_Memory.HasPrefix(ctx.m_iPlayer, RZ_DialogueMemory.CLAIMS, ctx.m_sKey + "|");
	}

	//------------------------------------------------------------------------------------------------
	//! A [Lie] whose `truth` holds isn't a lie: no tag, no check, nothing stored.
	protected bool IsTrue(RZ_DCtx ctx, RZ_DChoice c)
	{
		return c.m_Truth && !c.m_Truth.IsEmpty() && RZ_DialogueCond.Holds(ctx, c.m_Truth);
	}

	//------------------------------------------------------------------------------------------------
	//! This speaker caught a lie from this player in the last real day.
	protected bool LieBarred(RZ_DCtx ctx)
	{
		int at = m_Memory.TopicAt(ctx.m_iPlayer, ctx.m_sKey + "|lie_caught");
		return at > 0 && RZ_SeasonClock.Now() - at < RZ_DialogueMemory.DAY;
	}

	//------------------------------------------------------------------------------------------------
	//! Passing as `f` (a look passed): talk can't lean on them, and persuading them counts 0.
	protected bool Passing(int playerId, int f)
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		return RZ_Factions.IsValid(f) && hostility && hostility.GetDisguise().IsPassing(playerId, f);
	}

	// ================================================================ Claims in someone else's kit
	//------------------------------------------------------------------------------------------------
	//! Why this player can't claim to be one of them here ("" if they can): the disguise rules, and
	//! leaders and home ground, who see through any claim.
	protected string CannotLook(RZ_DCtx ctx, RZ_DChoice c)
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		if (!hostility || !RZ_Factions.IsValid(c.m_iCheckFaction))
			return "Nobody here would believe it.";

		string why = hostility.GetDisguise().CannotClaim(ctx.m_iPlayer, c.m_iCheckFaction);
		if (why == "Off")
			return "Nobody here would believe it.";

		return why;
	}

	//------------------------------------------------------------------------------------------------
	//! The look (RZ_Disguise.Look, the fixed score shown before you pick). A leader of theirs or
	//! anyone inside their wire sees through it: caught. Passed at a road post: while it stands;
	//! elsewhere 10 minutes.
	protected RZ_ECheck Look(RZ_TalkSession s, RZ_DChoice c)
	{
		RZ_DCtx ctx = s.m_Tree.m_Ctx;
		RZ_Disguise d = m_GM.GetHostility().GetDisguise();
		int f = c.m_iCheckFaction;
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(ctx.m_iPlayer);
		if ((ctx.m_Leader && ctx.m_Leader.m_eFaction == f) || (ctx.m_bStronghold && ctx.m_iFaction == f))
		{
			if (me)
				d.Caught(ctx.m_iPlayer, f, me, "\"Nice try. I know my own.\"");

			return RZ_ECheck.REFUSED;
		}

		float seconds = 600;
		if (ctx.m_iPost >= 0)
			seconds = 0;

		if (d.Look(ctx.m_iPlayer, f, Difficulty(ctx, c), ctx.m_sKey, "", seconds))
			return RZ_ECheck.PASS;

		return RZ_ECheck.REFUSED;
	}

	//------------------------------------------------------------------------------------------------
	//! A passed look often goes without a word (one time in three at least, the server's pick);
	//! otherwise a line in their voice from the bank, else a plain one.
	protected string LookVerdict(RZ_DCtx ctx, RZ_DChoice c, RZ_ECheck result)
	{
		string kind = "look_caught";
		if (result == RZ_ECheck.PASS)
		{
			if (Math.RandomFloat01() < 0.34)
				return "[Passed]";

			kind = "look_pass";
		}

		string line = RZ_VoiceBank.PickFor(m_GM, kind, c.m_iCheckFaction, "", ctx.m_Leader, ctx.m_iPlayer, ctx.Pos(), ctx.m_sPlace);
		if (result == RZ_ECheck.PASS)
		{
			if (line.IsEmpty())
				return "[Passed]";

			return "[Passed] " + line;
		}

		if (line.IsEmpty())
			return "[Caught]";

		return "[Caught] " + line;
	}

	//------------------------------------------------------------------------------------------------
	protected static string LieVerdict(RZ_ECheck result)
	{
		if (result == RZ_ECheck.PASS)
			return "They believe you. For now.";

		return "They don't believe a word of it.";
	}

	//------------------------------------------------------------------------------------------------
	//! The rest of a pick: its effects, the next node, and what others within 8 m overhear.
	protected bool Land(RZ_TalkSession s, RZ_DChoice c, RZ_ECheck result, string verdict, IEntity user)
	{
		RZ_DTalk t = s.m_Tree;
		bool pub = t.m_Node && t.m_Node.m_bPublic;
		string label = RZ_DialogueText.Fill(t.m_Ctx, c.m_sLabel);
		bool goOn = LandPick(s, c, result, verdict, user);
		RZ_DialogueScenes scenes = Scenes();
		if (pub && scenes)
			scenes.Overheard(s, label, s.m_aNew);

		// A lie said in a public node is a lie told to everyone who heard it
		if (s.m_Tree && s.m_Tree.m_Node && s.m_Tree.m_Node.m_bPublic)
			MarkOverheardLies(s);

		if (scenes && HasEffect(c.m_aDo, "claim") && c.m_sCheck != "lie")
			scenes.NoteClaim(s, null);

		return goOn;
	}

	//------------------------------------------------------------------------------------------------
	//! The lies (~lie) in the node just entered go to every other player within hearing, as heard.
	protected void MarkOverheardLies(RZ_TalkSession s)
	{
		RZ_DTalk t = s.m_Tree;
		array<string> keys = {};
		foreach (RZ_DSay item : t.m_Node.m_aItems)
		{
			if (!item.m_sLie.IsEmpty())
				keys.Insert(Scoped(t.m_Ctx, item.m_sLie));
		}

		if (keys.IsEmpty() || !t.m_Ctx.m_User)
			return;

		float reach = RZ_DialogueSettings.Get().m_fHear;
		if (t.m_Ctx.m_bRadio)
			reach = RZ_TalkRadio.REACH;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] == s.m_iPlayer || RZ_Util.DistXZ(p.GetOrigin(), t.m_Ctx.m_User.GetOrigin()) > reach)
				continue;

			foreach (string key : keys)
			{
				m_Memory.NoteHeardLie(ids[i], key);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool LandPick(RZ_TalkSession s, RZ_DChoice c, RZ_ECheck result, string verdict, IEntity user)
	{
		RZ_DTalk t = s.m_Tree;

		// The effects, then the next node. Lines the effects say (a built reply's answer, a
		// payment) come after the node's own.
		s.m_aNew.Clear();
		t.m_sLabel = RZ_DialogueText.Fill(t.m_Ctx, c.m_sLabel);
		if (result == RZ_ECheck.PASS)
			RunEffects(s, c.m_aDo, user);
		else if (result == RZ_ECheck.SHORT)
			RunEffects(s, c.m_aShort, user);
		else
			RunEffects(s, c.m_aElse, user);

		if (!s.m_Tree)
			return false;

		// Kit the conversation itself changed (a catch takes it) doesn't end the conversation
		t.m_iKit = KitNow(s.m_iPlayer);

		array<string> extra = {};
		if (!verdict.IsEmpty())
			extra.Insert(verdict);

		extra.InsertAll(s.m_aNew);
		s.m_aNew.Clear();

		string target = c.TargetFor(result);
		if (target == "end")
		{
			s.m_aNew.InsertAll(extra);
			return false;
		}

		if (target == "hub")
		{
			GoHub(s, extra);
			return true;
		}

		if (target == "back")
			target = t.m_sPrev;

		EnterNode(s, target, extra, 0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void GoHub(RZ_TalkSession s, array<string> extra)
	{
		RZ_DTalk t = s.m_Tree;
		t.m_bInNode = false;
		t.m_Node = null;
		t.m_Conv = null;
		if (!extra.IsEmpty())
			s.m_aNew.InsertAll(extra);
		else
			s.m_aNew.InsertAll(t.m_aHubSaid);
	}

	//------------------------------------------------------------------------------------------------
	//! Into a node: its jumps (the first that holds), its entry effects, its lines.
	protected void EnterNode(RZ_TalkSession s, string id, array<string> extra, int depth)
	{
		RZ_DTalk t = s.m_Tree;
		RZ_DNode node;
		if (t.m_Conv)
			node = t.m_Conv.GetNode(id);

		if (!node || depth > MAX_JUMPS)
		{
			GoHub(s, extra);
			return;
		}

		foreach (RZ_DSay item : node.m_aItems)
		{
			if (item.m_bJump && RZ_DialogueCond.Holds(t.m_Ctx, item.m_Cond))
			{
				EnterNode(s, item.m_sText, extra, depth + 1);
				return;
			}
		}

		if (t.m_Node)
			t.m_sPrev = t.m_Node.m_sId;

		t.m_Node = node;
		t.m_bInNode = true;
		t.m_fNodeAt = RZ_UIW.Now();
		m_Memory.Add(s.m_iPlayer, RZ_DialogueMemory.SEEN, t.m_Conv.m_sId + "/" + node.m_sId);

		s.m_aNew.Clear();
		foreach (RZ_DSay fx : node.m_aItems)
		{
			if (fx.m_bDo)
				RunEffects(s, fx.m_aDo, t.m_Ctx.m_User);
		}

		array<string> effectLines = {};
		effectLines.InsertAll(s.m_aNew);
		s.m_aNew.Clear();
		array<string> lines = {};
		Lines(t, node, lines);
		if (node.m_bConfirm)
			ConfirmLines(t, node, lines);

		s.m_aNew.InsertAll(lines);
		s.m_aNew.InsertAll(effectLines);
		s.m_aNew.InsertAll(extra);
	}

	//------------------------------------------------------------------------------------------------
	//! A node's lines: a say? whose condition holds replaces the next plain say.
	protected void Lines(RZ_DTalk t, RZ_DNode node, notnull array<string> lines)
	{
		LinesFor(t.m_Ctx, node, lines);
	}

	//------------------------------------------------------------------------------------------------
	//! The same for any speaker as conditions see them (barks too).
	void LinesFor(RZ_DCtx ctx, RZ_DNode node, notnull array<string> lines)
	{
		RZ_DSay pending;
		bool hasPending;
		foreach (RZ_DSay item : node.m_aItems)
		{
			if (item.m_bJump || item.m_bDo)
				continue;

			if (!item.m_Cond.IsEmpty())
			{
				if (!hasPending && RZ_DialogueCond.Holds(ctx, item.m_Cond))
				{
					pending = item;
					hasPending = true;
				}

				continue;
			}

			if (hasPending)
				SayLine(ctx, pending, lines);
			else
				SayLine(ctx, item, lines);

			hasPending = false;
		}

		if (hasPending)
			SayLine(ctx, pending, lines);
	}

	//------------------------------------------------------------------------------------------------
	//! One line said: a lie the player is told (~lie) or the truth (~truth) is noted as it's heard.
	protected void SayLine(RZ_DCtx ctx, RZ_DSay item, notnull array<string> lines)
	{
		lines.Insert(RZ_DialogueText.Fill(ctx, item.m_sText));
		if (!item.m_sLie.IsEmpty())
			m_Memory.NoteHeardLie(ctx.m_iPlayer, Scoped(ctx, item.m_sLie));

		if (!item.m_sTruth.IsEmpty())
			m_Memory.Learn(ctx.m_iPlayer, Scoped(ctx, item.m_sTruth));
	}

	//------------------------------------------------------------------------------------------------
	//! A key bound to this speaker: "@here" becomes "@<speaker key>" (a promise or a lie at one house).
	static string Scoped(RZ_DCtx ctx, string key)
	{
		if (!key.Contains("@"))
			return key;

		string s = key;
		s.Replace("@here", "@" + ctx.m_sKey);
		foreach (string name, string value : ctx.m_mBind)
		{
			s.Replace("@" + name, "@" + value);
		}

		return s;
	}

	// ================================================================ The finale's commit
	//------------------------------------------------------------------------------------------------
	//! The route word of a choice's commit effect ("bastion"), "" if it has none.
	protected static string CommitRoute(RZ_DChoice c)
	{
		foreach (RZ_DEffect e : c.m_aDo)
		{
			if (e.m_sName == "commit")
				return e.m_sArg;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A [Point of no return] node: the finale's three plain lines (the admission, what you bring,
	//! what you leave) for the route its commit choice names.
	protected void ConfirmLines(RZ_DTalk t, RZ_DNode node, notnull array<string> lines)
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (!finale)
			return;

		foreach (RZ_DChoice c : node.m_aChoices)
		{
			string route = CommitRoute(c);
			if (route.IsEmpty())
				continue;

			array<string> card = {};
			finale.LockCard(t.m_Ctx.m_iPlayer, RZ_CureFinale.RouteOfWord(route)).Split("\n", card, true);
			lines.InsertAll(card);
			return;
		}
	}

	// ================================================================ Checks and tags
	//------------------------------------------------------------------------------------------------
	//! Trust (RZ_Checks.TrustScore) with dialogue's bonus: +1 for regard 2 or more, or the built
	//! helped or saved bit this life. The Long Table's rank counts 0.
	int TrustScore(RZ_DCtx ctx, RZ_DChoice c)
	{
		// Passing as one of them: no rewards through a claim, so persuading them counts 0
		if (Passing(ctx.m_iPlayer, c.m_iCheckFaction))
			return 0;

		int bonus = 0;
		if (m_Memory.Regard(ctx.m_iPlayer, ctx.m_sKey) >= 2 || HelpedThisLife(ctx))
			bonus = 1;

		RZ_DialogueScenes scenes = Scenes();
		if (scenes && scenes.VouchBonus(ctx.m_iPlayer) > 0)
			bonus = 1;

		if (ctx.m_iTableRole >= 0)
		{
			int score = bonus;
			if (RZ_QuestApproach.HasBackground(m_GM, ctx.m_iPlayer, c.m_sFavoured))
				score++;

			return score;
		}

		return RZ_Checks.TrustScore(m_GM, ctx.m_iPlayer, c.m_iCheckFaction, c.m_sFavoured, bonus);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HelpedThisLife(RZ_DCtx ctx)
	{
		RZ_DTerm helped = new RZ_DTerm();
		helped.m_sKey = "helped";
		RZ_DTerm saved = new RZ_DTerm();
		saved.m_sKey = "saved";
		RZ_DCond any = new RZ_DCond();
		any.m_aTerms.Insert(helped);
		if (RZ_DialogueCond.Holds(ctx, any))
			return true;

		any.m_aTerms.Clear();
		any.m_aTerms.Insert(saved);
		return RZ_DialogueCond.Holds(ctx, any);
	}

	//------------------------------------------------------------------------------------------------
	//! The difficulty, moved one by the speaker's traits (easier:merciful, harder:paranoid), 1 to 4.
	int Difficulty(RZ_DCtx ctx, RZ_DChoice c)
	{
		int d = c.m_iDifficulty;
		if (ctx.m_Leader)
		{
			if (!c.m_sEasier.IsEmpty() && ctx.m_Leader.HasTrait(RZ_DialogueCond.TraitKey(c.m_sEasier)))
				d--;

			if (!c.m_sHarder.IsEmpty() && ctx.m_Leader.HasTrait(RZ_DialogueCond.TraitKey(c.m_sHarder)))
				d++;
		}

		return Math.ClampInt(d, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! A lie's difficulty: +1 at a Paranoid leader, +1 where this speaker caught you lying this life,
	//! -1 for an Ex-Con or a Raider, and (never shown) the speaker's own credulity, seeded by who they
	//! are and the season: one in three believes a little more, one in three a little less.
	int LieDifficulty(RZ_DCtx ctx, RZ_DChoice c, bool hidden)
	{
		int d = Difficulty(ctx, c);
		if (ctx.m_Leader && ctx.m_Leader.HasTrait(RZ_ELeaderTrait.PARANOID))
			d++;

		if (m_Memory.TopicAt(ctx.m_iPlayer, ctx.m_sKey + "|lie_caught") > 0)
			d++;

		string bg = m_GM.GetProfile(ctx.m_iPlayer).m_sBackgroundId;
		if (bg == "excon" || bg == "raider")
			d--;

		if (hidden)
		{
			int seed = 0;
			if (m_GM.GetFacts())
				seed = m_GM.GetFacts().GetSeed();

			d += Math.AbsInt(ctx.m_sKey.Hash() + seed) % 3 - 1;
		}

		return Math.ClampInt(d, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! The tag a choice carries: what it needs and costs, never what comes of it.
	protected void Tag(RZ_DTalk t, RZ_DChoice c, RZ_TalkReply r)
	{
		RZ_DCtx ctx = t.m_Ctx;
		string tag = "";
		foreach (RZ_DTerm term : c.m_If.m_aTerms)
		{
			if (term.m_bNeg)
				continue;

			if (term.m_sKey == "bg" && !term.m_aArgs.IsEmpty())
				tag = "[" + BackgroundName(m_GM.GetProfile(ctx.m_iPlayer).m_sBackgroundId) + "]";
			else if (term.m_sKey == "tier" && (term.m_sOp == ">=" || term.m_sOp == "=") && RZ_Factions.IsValid(term.m_iFaction))
				tag = string.Format("[%1 - %2]", RZ_Reputation.GetTierName(RZ_DialogueCond.TierKey(term.m_sVal)), RZ_Factions.GetName(term.m_iFaction));
		}

		if (c.m_sCheck == "persuade")
		{
			int score = TrustScore(ctx, c);
			int d = Difficulty(ctx, c);
			string who = RZ_Factions.GetName(c.m_iCheckFaction);
			if (!RZ_Factions.IsValid(c.m_iCheckFaction))
				tag = string.Format("[Persuade - %1 of %2]", Math.Max(0, score), d);
			else
				tag = string.Format("[Persuade - %1 - %2 of %3]", who, Math.Max(0, score), d);

			if (score < 0)
			{
				r.m_bEnabled = false;
				r.m_sNeed = "they won't hear it from you";
			}
			else if (m_Memory.FailedAt(ctx.m_iPlayer, ctx.m_sKey + "|" + t.m_Conv.m_sId + "/@" + c.m_sId) >= score)
			{
				r.m_bEnabled = false;
				r.m_sNeed = "you've tried that";
			}
		}
		else if (c.m_sCheck == "lie" && IsTrue(ctx, c))
		{
			// The truth: no [Lie] tag (a tag never gives a lie away, and this isn't one)
		}
		else if (c.m_sCheck == "look")
		{
			tag = LookTag(ctx, c, r);
		}
		else if (c.m_sCheck == "lie")
		{
			int lieScore = RZ_Checks.TrustScore(m_GM, ctx.m_iPlayer, c.m_iCheckFaction, "", 0);
			if (LieBarred(ctx))
			{
				r.m_bEnabled = false;
				r.m_sNeed = "they caught you lying today";
			}

			if (HasTell(ctx) || c.m_bTellAlways)
				tag = "[Lie - they'd see through it]";
			else
				tag = string.Format("[Lie - %1 of %2]", Math.Max(0, lieScore), LieDifficulty(ctx, c, false));
		}
		else if (c.m_sCheck == "threaten")
		{
			int fear = RZ_Checks.FearScore(m_GM, ctx.m_iPlayer, c.m_iCheckFaction, -1, c.m_sFavoured);
			tag = string.Format("[Fear %1 of %2 - +%3 Infamy", fear, Difficulty(ctx, c), RZ_Checks.THREAT_INFAMY);
			string cliff = RZ_Checks.FearTag(m_GM, ctx.m_iPlayer, c.m_iCheckFaction);
			if (cliff.Contains("Hunted"))
				tag += ": Hunted";

			tag += "]";
		}

		if (c.m_sCostKind == "favor")
		{
			string price = string.Format("%1 %2 Favor", c.m_iCost, RZ_Factions.GetName(c.m_iCostFaction));
			tag = Join(tag, "[" + price + "]");
			int have = m_GM.GetProfile(ctx.m_iPlayer).GetFavor(c.m_iCostFaction);
			if (have < c.m_iCost)
			{
				r.m_bEnabled = false;
				r.m_sNeed = string.Format("needs %1, you have %2", price, have);
			}
		}
		else if (c.m_sCostKind == "food")
		{
			tag = Join(tag, string.Format("[%1 food]", c.m_iCost));
			int food = 0;
			if (ctx.m_User)
				food = RZ_HavenQuest.CountFood(m_GM, ctx.m_User);

			if (food < c.m_iCost)
			{
				r.m_bEnabled = false;
				r.m_sNeed = string.Format("you have %1 food", food);
			}
		}

		// The lock: [Point of no return], dimmed with the finale's reason when you can't
		string commitTo = CommitRoute(c);
		if (!commitTo.IsEmpty())
		{
			tag = Join(tag, "[Point of no return]");
			string why = "\"Not here.\"";
			if (t.m_Node && t.m_Node.m_bConfirm && RZ_CureFinale.Get())
				why = RZ_CureFinale.Get().CanCommit(ctx.m_iPlayer, RZ_CureFinale.RouteOfWord(commitTo));

			if (!why.IsEmpty())
			{
				r.m_bEnabled = false;
				r.m_sNeed = why;
			}
		}

		if (!c.m_sCallout.IsEmpty())
			tag = Join("[Evidence]", tag);

		// Who may decide: a deciding choice in a scene with `who` is dimmed for everyone else
		if (r.m_bEnabled && t.m_Conv && t.m_Conv.m_SceneWho && HasEffect(c.m_aDo, "decide") && !RZ_DialogueCond.Holds(ctx, t.m_Conv.m_SceneWho))
		{
			r.m_bEnabled = false;
			r.m_sNeed = "only someone " + ctx.m_sShort + " trusts can answer for it";
		}

		r.m_sTag = tag;
	}

	//------------------------------------------------------------------------------------------------
	//! [Look - Bastion - 2 of 2] with the look's own tags ([Full kit +2] [Watched -1]), or dimmed with
	//! why they can't claim at all.
	protected string LookTag(RZ_DCtx ctx, RZ_DChoice c, RZ_TalkReply r)
	{
		int f = c.m_iCheckFaction;
		string cannot = CannotLook(ctx, c);
		if (!cannot.IsEmpty())
		{
			r.m_bEnabled = false;
			r.m_sNeed = cannot;
			return string.Format("[Look - %1]", RZ_Factions.GetName(f));
		}

		string tags;
		bool instant;
		int score = RZ_Checks.LookScore(m_GM, ctx.m_iPlayer, f, ctx.m_sKey, "", tags, instant);
		if (instant || (ctx.m_Leader && ctx.m_Leader.m_eFaction == f) || (ctx.m_bStronghold && ctx.m_iFaction == f))
			return string.Format("[Look - %1 - they'd see through it]", RZ_Factions.GetName(f));

		return Join(tags, string.Format("[Look - %1 - %2 of %3]", RZ_Factions.GetName(f), Math.Max(0, score), Difficulty(ctx, c)));
	}

	//------------------------------------------------------------------------------------------------
	protected static string OnOff(bool on)
	{
		if (on)
			return "on";

		return "off";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Join(string a, string b)
	{
		if (a.IsEmpty())
			return b;

		return a + " " + b;
	}

	//------------------------------------------------------------------------------------------------
	protected static string PersuadeVerdict(RZ_ECheck result)
	{
		if (result == RZ_ECheck.PASS)
			return "[Persuaded]";

		if (result == RZ_ECheck.SHORT)
			return "[Short]";

		return "[Not persuaded]";
	}

	//------------------------------------------------------------------------------------------------
	protected static string FearVerdict(RZ_ECheck result)
	{
		if (result == RZ_ECheck.PASS)
			return "[They back down]";

		if (result == RZ_ECheck.SHORT)
			return "[Short]";

		return "[They don't scare]";
	}

	//------------------------------------------------------------------------------------------------
	static string BackgroundName(string id)
	{
		switch (id)
		{
			case "doctor": return "Doctor";
			case "nurse": return "Nurse";
			case "labtech": return "Lab Technician";
			case "police": return "Police";
			case "soldier": return "Soldier";
			case "hunter": return "Hunter";
			case "mechanic": return "Mechanic";
			case "radioham": return "Radio Operator";
			case "elder": return "Elder";
			case "excon": return "Ex-Con";
			case "raider": return "Raider";
			case "drifter": return "Drifter";
		}

		return "Unemployed";
	}

	// ================================================================ Prices and effects
	//------------------------------------------------------------------------------------------------
	protected bool Pay(RZ_DTalk t, RZ_DChoice c, IEntity user)
	{
		if (c.m_sCostKind == "favor")
			return m_GM.SpendFavor(t.m_Ctx.m_iPlayer, c.m_iCostFaction, c.m_iCost);

		if (c.m_sCostKind == "food")
			return user && RZ_HavenQuest.TakeFood(m_GM, user, c.m_iCost);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Each effect through the system that owns it, with that system's caps. Lines go into s.m_aNew.
	protected void RunEffects(RZ_TalkSession s, array<ref RZ_DEffect> effects, IEntity user)
	{
		RZ_DTalk t = s.m_Tree;
		if (!t)
			return;

		RZ_DCtx ctx = t.m_Ctx;
		int pid = s.m_iPlayer;
		foreach (RZ_DEffect e : effects)
		{
			switch (e.m_sName)
			{
				case "mark":
				{
					RZ_WorldFacts facts = m_GM.GetFacts();
					if (facts && facts.Flag(e.m_sArg) == 0)
						facts.SetFlag(e.m_sArg, 1);

					break;
				}
				case "cset":
				{
					int eq = e.m_sArg.IndexOf("=");
					if (eq > 0)
						m_Memory.SetCharFlag(pid, e.m_sArg.Substring(0, eq), e.m_sArg.Substring(eq + 1, e.m_sArg.Length() - eq - 1).ToInt());
					else
						m_Memory.SetCharFlag(pid, e.m_sArg, 1);

					break;
				}
				case "nset":
					m_Memory.Add(pid, RZ_DialogueMemory.NOTES, ctx.m_sKey + "|" + e.m_sArg);
					break;
				case "regard":
					m_Memory.AddRegard(pid, ctx.m_sKey, e.m_sArg, LieWeight(ctx));
					break;
				case "favor":
					TalkFavor(s, e.m_sArg);
					break;
				case "take":
				{
					int star = e.m_sArg.IndexOf("*");
					if (star > 0 && user)
						RZ_HavenQuest.TakeFood(m_GM, user, e.m_sArg.Substring(star + 1, e.m_sArg.Length() - star - 1).ToInt());

					break;
				}
				case "claim":
				{
					m_Memory.Add(pid, RZ_DialogueMemory.CLAIMS, ctx.m_sKey + "|" + e.m_sArg);
					// What was said, for {claim:key}
					if (!t.m_sLabel.IsEmpty())
						m_Memory.Add(pid, RZ_DialogueMemory.NOTES, ctx.m_sKey + "|said." + e.m_sArg + "|" + t.m_sLabel);

					break;
				}
				case "watch":
					RZ_DialogueBinds.Watch(s, e.m_sArg);
					break;
				case "learn":
				{
					// learn:<key>, or learn:<key>:<hours>h for a passing fact
					int lc = e.m_sArg.IndexOf(":");
					if (lc > 0 && e.m_sArg.EndsWith("h"))
						m_Memory.Learn(pid, Scoped(ctx, e.m_sArg.Substring(0, lc)), e.m_sArg.Substring(lc + 1, e.m_sArg.Length() - lc - 2).ToFloat());
					else
						m_Memory.Learn(pid, Scoped(ctx, e.m_sArg));

					break;
				}
				case "set":
				case "qset":
				case "note":
				case "begin":
				case "approach":
				case "test":
					RZ_DialogueStory.Effect(s, e.m_sName, e.m_sArg);
					break;
				case "rumour":
					Rumour(ctx, e.m_sArg);
					break;
				case "call":
					Call(s, e.m_sArg, user);
					break;
				case "promise":
					Promise(s, e.m_sArg);
					break;
				case "keep":
					m_Memory.Keep(pid, Scoped(ctx, e.m_sArg));
					break;
				case "wire":
				{
					// Bastion's word at the wire: right, it counts at the next look; wrong, caught
					if (RZ_DialogueWire.Say(m_GM, pid, e.m_sArg.ToInt(), user))
						s.m_aNew.Insert("(A nod at the word.)");

					break;
				}
				case "later":
				{
					// later:6h:key: due:key once this player has played that long (words only)
					int colon = e.m_sArg.IndexOf(":");
					if (colon > 1)
						m_Memory.Later(pid, Scoped(ctx, e.m_sArg.Substring(colon + 1, e.m_sArg.Length() - colon - 1)), e.m_sArg.Substring(0, colon - 1).ToFloat());

					break;
				}
				case "sway":
				{
					// A major's sitting: its own vote, one sway per player per sitting
					string swayed;
					if (RZ_DialogueVotes.Sway(m_GM, pid, e.m_sArg, ctx.m_Leader, swayed))
					{
						if (!swayed.IsEmpty())
							s.m_aNew.Insert(swayed);

						break;
					}

					// One contribution per player to a count its owner reads
					RZ_WorldFacts tally = m_GM.GetFacts();
					string mine = "tally." + e.m_sArg + "." + m_GM.GetProfile(pid).m_sIdentity;
					if (tally && tally.Flag(mine) == 0)
					{
						tally.SetFlag(mine, 1);
						tally.SetFlag("tally." + e.m_sArg, tally.Flag("tally." + e.m_sArg) + 1);
						RZ_Log.Info(string.Format("Dialogue: sway %1 by %2 (now %3)", e.m_sArg, RZ_Util.PlayerName(pid), tally.Flag("tally." + e.m_sArg)));
					}

					break;
				}
				case "decide":
				{
					// The scene's record: who, which, once for the world
					RZ_WorldFacts scene = m_GM.GetFacts();
					if (scene && !ctx.m_sScene.IsEmpty() && scene.Flag("scene." + ctx.m_sScene) == 0)
					{
						scene.SetFlag("scene." + ctx.m_sScene, 1);
						scene.SetFlag("scene." + ctx.m_sScene + "." + e.m_sArg, 1);
						scene.Memory().Record("scene." + ctx.m_sScene, ctx.m_sKey, e.m_sArg, RZ_WorldMemory.SOURCE_CHOICE, false);
						RZ_Log.Info(string.Format("Dialogue: scene %1 decided %2 by %3", ctx.m_sScene, e.m_sArg, RZ_Util.PlayerName(pid)));
					}

					break;
				}
				case "commit":
				{
					// The finale's lock (RZ_CureFinale.Commit checks CanCommit again)
					RZ_CureFinale finale = RZ_CureFinale.Get();
					if (finale && t.m_Node && t.m_Node.m_bConfirm && finale.Commit(pid, RZ_CureFinale.RouteOfWord(e.m_sArg)))
					{
						s.m_aNew.Insert(string.Format("Written down for %1, until the cure.", RZ_CureFinale.RouteName(RZ_CureFinale.RouteOfWord(e.m_sArg))));
						RZ_Log.Info(string.Format("Dialogue: %1 committed to %2 at %3", RZ_Util.PlayerName(pid), e.m_sArg, ctx.m_sKey));
					}

					break;
				}
				case "offer":
				{
					// A speaker a running story holds offers nothing else this turn
					if (RZ_DialogueWorld.Claimed(ctx) && (!t.m_Conv || t.m_Conv.m_iPrio != 0))
					{
						s.m_aNew.Insert("\"Not now. I've got enough on.\"");
						break;
					}

					if (ctx.m_Leader && m_GM.GetLeaders())
						m_GM.GetLeaders().HandleChoice(pid, ctx.m_Leader.m_iId, 0, ctx.m_Body, user);

					break;
				}
			}

			if (!s.m_Tree)
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! promise:<key>[:<hours>h]: a promise to this speaker, in the words of the choice that made it.
	//! Two hours of the promiser's own play by default, three real days at most, three open a player.
	protected void Promise(RZ_TalkSession s, string arg)
	{
		RZ_DTalk t = s.m_Tree;
		string key = arg;
		float hours = RZ_DialogueSettings.Get().m_fPromiseHours;
		int colon = arg.IndexOf(":");
		if (colon > 0)
		{
			key = arg.Substring(0, colon);
			string h = arg.Substring(colon + 1, arg.Length() - colon - 1);
			h.Replace("h", "");
			hours = Math.Clamp(h.ToFloat(), 0.25, 72);
		}

		string words = t.m_sLabel;
		if (words.IsEmpty())
			words = "to come back";

		string why = m_Memory.Promise(s.m_iPlayer, Scoped(t.m_Ctx, key), t.m_Ctx.m_sKey, words, hours);
		if (!why.IsEmpty())
		{
			s.m_aNew.Insert("(" + why + ")");
			return;
		}

		s.m_aNew.Insert(string.Format("(A promise: %1 hours of your play to keep it.)", RZ_Util.RoundToInt(hours)));
		RZ_Log.Info(string.Format("Dialogue: promise %1 made by %2", Scoped(t.m_Ctx, key), RZ_Util.PlayerName(s.m_iPlayer)));
	}

	//------------------------------------------------------------------------------------------------
	//! A caught lie costs a Merciful speaker 2, a Paranoid one 4, anyone else 3.
	protected int LieWeight(RZ_DCtx ctx)
	{
		if (ctx.m_Leader && ctx.m_Leader.HasTrait(RZ_ELeaderTrait.MERCIFUL))
			return 2;

		if (ctx.m_Leader && ctx.m_Leader.HasTrait(RZ_ELeaderTrait.PARANOID))
			return 4;

		return 3;
	}

	//------------------------------------------------------------------------------------------------
	//! favor:haven+5: at most 10 a conversation, and all talk shares Dialogue > Talk Favor a day per
	//! faction. Past it the choice still works, unpaid.
	protected void TalkFavor(RZ_TalkSession s, string arg)
	{
		RZ_DTalk t = s.m_Tree;
		int plus = arg.IndexOf("+");
		if (plus <= 0)
			return;

		int faction = RZ_DialogueDB.FactionKey(arg.Substring(0, plus));
		int want = arg.Substring(plus + 1, arg.Length() - plus - 1).ToInt();
		want = Math.Min(want, FAVOR_PER_CONV - t.m_iFavorPaid);
		int paid = m_Memory.TakeTalkFavor(s.m_iPlayer, faction, want, RZ_DialogueSettings.Get().m_iTalkFavorPerDay);
		if (paid <= 0)
		{
			s.m_aNew.Insert("\"That's all I've got today.\"");
			RZ_Log.Info(string.Format("Dialogue: budget reached %1 %2", RZ_Util.PlayerName(s.m_iPlayer), RZ_Factions.GetName(faction)));
			return;
		}

		t.m_iFavorPaid += paid;
		m_GM.AddFavor(s.m_iPlayer, faction, paid);
		s.m_aNew.Insert(string.Format("(+%1 %2 Favor.)", paid, RZ_Factions.GetName(faction)));
	}

	//------------------------------------------------------------------------------------------------
	//! rumour:"text"@hours: gossip on the roads (the world's rumour list).
	protected void Rumour(RZ_DCtx ctx, string arg)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (!facts)
			return;

		string text = arg;
		float hours = 6;
		int at = text.LastIndexOf("@");
		if (at > 0)
		{
			hours = text.Substring(at + 1, text.Length() - at - 1).ToFloat();
			text = text.Substring(0, at);
		}

		text.Replace("\"", "");
		facts.AddRumour(RZ_DialogueText.Fill(ctx, text), Math.Max(1, hours));
	}

	//------------------------------------------------------------------------------------------------
	//! call: built behaviour, unchanged. slot:N runs the person's own built reply N; hand_in:N hands
	//! food in at their safehouse (as built); toll and post_wave are the road post's; standing is
	//! what they make of you, in their voice.
	protected void Call(RZ_TalkSession s, string what, IEntity user)
	{
		RZ_DTalk t = s.m_Tree;
		RZ_DCtx ctx = t.m_Ctx;
		int pid = s.m_iPlayer;
		if (what.StartsWith("slot:"))
		{
			int slot = what.Substring(5, what.Length() - 5).ToInt();
			s.m_bCapturing = true;
			s.m_Adapter.Pick(m_GM, s, pid, slot, user);
			return;
		}

		if (what.StartsWith("hand_in:"))
		{
			int count = what.Substring(8, what.Length() - 8).ToInt();
			if (m_GM.GetSafehouses() && ctx.m_iHouse >= 0)
				m_GM.GetSafehouses().HandInFood(pid, ctx.m_iHouse, user, count);

			return;
		}

		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (what == "toll" && posts)
		{
			s.m_aNew.Insert(posts.PayToll(pid, ctx.m_iPost));
			return;
		}

		if (what == "post_wave" && posts)
		{
			s.m_aNew.Insert(posts.WaveThrough(pid, ctx.m_iPost));
			return;
		}

		if (what == "standing")
		{
			array<string> lines = {};
			RZ_RepVoice.Verdict(m_GM, pid, ctx.m_iFaction, ctx.m_Leader, lines);
			s.m_aNew.InsertAll(lines);
			return;
		}

		// The epilogue week's carriers, as built: the season at this seat, and where to look
		RZ_Epilogue epi = RZ_Epilogue.Get();
		if ((what == "season" || what == "look") && epi && ctx.m_Leader)
		{
			s.m_bCapturing = true;
			if (what == "season")
				epi.LeaderPick(ctx.m_Leader, pid, 40);
			else
				epi.LeaderPick(ctx.m_Leader, pid, 41);

			return;
		}

		// A bite lie caught at a safehouse counts as caught hiding that bite (the house's own rule)
		if (what == "caught_bite")
		{
			RZ_PlayerProfile bitten = m_GM.GetProfile(pid);
			if (bitten.IsInfected() && !bitten.m_bBiteDeclared && !bitten.m_bBiteCaught)
			{
				bitten.m_bBiteCaught = true;
				s.m_aNew.Insert("(Caught hiding a bite: the houses won't take your word on it again this life.)");
			}

			return;
		}

		// Telling a safehouse about a bite: the house's own service, as built
		if (what == "declare_bite" && m_GM.GetSafehouses())
		{
			m_GM.GetSafehouses().HandleAction(pid, RZ_ESafehouseAction.DECLARE_BITE, ctx.m_Body, user);
			return;
		}

		switch (what)
		{
			case "distress": RZ_DialogueNets.Answer(s); return;
			case "net_word": RZ_DialogueNets.NetWord(s); return;
			case "net_lost": RZ_DialogueNets.NetLost(s); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A leader's or wanderer's attitude to this player (regard included), 0 for anyone else.
	int Attitude(RZ_DCtx ctx)
	{
		if (ctx.m_Leader && m_GM.GetLeaders())
			return m_GM.GetLeaders().Attitude(ctx.m_Leader, ctx.m_iPlayer);

		if (ctx.m_Wanderer && m_GM.GetWanderers())
			return m_GM.GetWanderers().AttitudeOf(ctx.m_Wanderer, ctx.m_iPlayer);

		return 0;
	}

	// ================================================================ Found out later
	//------------------------------------------------------------------------------------------------
	//! A lie told through a conversation was found out (RZ_Lies): its choice's `found` effects run,
	//! on things still running only: the speaker's note, a character flag, a season mark, a tally's sway
	//! taken back.
	void OnLieFound(RZ_Lie lie, int playerId)
	{
		if (!lie || !lie.key.StartsWith("talk.") || playerId <= 0)
			return;

		string rest = lie.key.Substring(5, lie.key.Length() - 5);
		int at = rest.IndexOf("/@");
		if (at <= 0)
			return;

		RZ_DChoice c = m_DB.FindChoice(rest.Substring(0, at), rest.Substring(at + 2, rest.Length() - at - 2));
		if (!c || c.m_aFound.IsEmpty())
			return;

		foreach (RZ_DEffect e : c.m_aFound)
		{
			switch (e.m_sName)
			{
				case "nset":
					m_Memory.Add(playerId, RZ_DialogueMemory.NOTES, lie.speaker + "|" + e.m_sArg);
					break;
				case "cset":
					m_Memory.SetCharFlag(playerId, e.m_sArg, 1);
					break;
				case "mark":
				{
					RZ_WorldFacts facts = m_GM.GetFacts();
					if (facts && facts.Flag(e.m_sArg) == 0)
						facts.SetFlag(e.m_sArg, 1);

					break;
				}
				case "unsway":
				{
					// A tally's contribution taken back (a major's sitting counts its own sways)
					RZ_WorldFacts tally = m_GM.GetFacts();
					string mine = "tally." + e.m_sArg + "." + m_GM.GetProfile(playerId).m_sIdentity;
					if (tally && tally.Flag(mine) > 0)
					{
						tally.SetFlag(mine, 0);
						tally.SetFlag("tally." + e.m_sArg, Math.Max(0, tally.Flag("tally." + e.m_sArg) - 1));
					}

					break;
				}
			}
		}

		RZ_Log.Info(string.Format("Dialogue: lie %1 found out: its found effects ran", lie.key));
	}

	// ================================================================ Admin tools
	//------------------------------------------------------------------------------------------------
	//! "/talkdebug" (admins), and its [Admin] replies: "" reads the person you're talking to; "test"
	//! runs every conversation once for you (the runtime test); "reset <scene>" undecides a scene
	//! ("reset" alone: the scenes of the person you're talking to).
	void Command(int playerId, RZ_TalkSession s, string text)
	{
		string cmd = text.Trim();
		if (cmd.IsEmpty())
		{
			Debug(playerId, s);
			return;
		}

		if (!RZ_NoticeBoard.IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Dialogue", "Only a server admin can do that.");
			return;
		}

		if (cmd == "test")
		{
			m_GM.NotifyPlayer(playerId, "Dialogue", SelfTest(playerId, s));
			return;
		}

		if (cmd.StartsWith("reset"))
		{
			string scene = cmd.Substring(5, cmd.Length() - 5).Trim();
			m_GM.NotifyPlayer(playerId, "Dialogue", ResetScenes(scene, s));
			return;
		}

		m_GM.NotifyPlayer(playerId, "Dialogue", "/talkdebug, /talkdebug test, /talkdebug reset <scene>");
	}

	//------------------------------------------------------------------------------------------------
	//! Every conversation's nodes read for this player (their conditions, lines and labels, through the
	//! real evaluator and tokens), and a lie, a promise and a callback run once on test keys. Any script
	//! error shows in the log; the counts go back to the admin.
	protected string SelfTest(int playerId, RZ_TalkSession s)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_DCtx ctx;
		if (s && s.m_Tree)
			ctx = s.m_Tree.m_Ctx;

		if (!ctx)
		{
			ctx = new RZ_DCtx();
			ctx.m_GM = m_GM;
			ctx.m_iPlayer = playerId;
			ctx.m_User = me;
			ctx.m_sKey = "talktest";
			ctx.m_aKinds.Insert("talktest");
			if (me)
				ctx.m_iTown = m_GM.FindNearestSettlement(me.GetOrigin());
		}

		array<RZ_DConv> all = {};
		m_DB.All(all);
		int holds = 0;
		int nodes = 0;
		int lines = 0;
		int labels = 0;
		foreach (RZ_DConv c : all)
		{
			if (RZ_DialogueCond.Holds(ctx, c.m_When))
				holds++;

			RZ_DialogueBinds.Pick(ctx, c);
			foreach (RZ_DNode n : c.m_aNodes)
			{
				nodes++;
				array<string> said = {};
				LinesFor(ctx, n, said);
				lines += said.Count();
				foreach (RZ_DChoice ch : n.m_aChoices)
				{
					RZ_DialogueCond.Holds(ctx, ch.m_If);
					RZ_DialogueText.Fill(ctx, ch.m_sLabel);
					labels++;
				}
			}
		}

		ctx.m_mBind.Clear();

		// A lie, a promise and a callback, once each, on keys nobody else uses
		m_Memory.NoteHeardLie(playerId, "talktest.lie");
		m_Memory.Learn(playerId, "talktest.lie", 0.01);
		bool lieOk = m_Memory.HeardLie(playerId, "talktest.lie") && m_Memory.Knows(playerId, "talktest.lie");
		string promised = m_Memory.Promise(playerId, "talktest.promise", "talktest", "to test the promises", 0.05);
		bool promiseOk = promised.IsEmpty() && m_Memory.Keep(playerId, "talktest.promise");
		m_Memory.Later(playerId, "talktest.later", 0);
		bool laterOk = m_Memory.Due(playerId, "talktest.later");
		string result = string.Format("Talk test: %1 conversations (%2 hold for you here), %3 nodes, %4 lines, %5 labels. Lie %6, promise %7, callback %8.",
			all.Count(), holds, nodes, lines, labels, OkWord(lieOk), OkWord(promiseOk || !promised.IsEmpty()), OkWord(laterOk));
		RZ_Log.Info("Dialogue: " + result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! The runtime test's call (tools/runtime_test): the same self test, for any player.
	string RunSelfTest(int playerId)
	{
		return SelfTest(playerId, null);
	}

	//------------------------------------------------------------------------------------------------
	protected static string OkWord(bool ok)
	{
		if (ok)
			return "ok";

		return "FAILED";
	}

	//------------------------------------------------------------------------------------------------
	//! A scene's record cleared: decided again by whoever answers next. `scene` empty: every scene of
	//! the person this admin is talking to.
	protected string ResetScenes(string scene, RZ_TalkSession s)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (!facts)
			return "No world facts.";

		array<RZ_DConv> all = {};
		m_DB.All(all);
		int cleared = 0;
		foreach (RZ_DConv c : all)
		{
			if (c.m_sScene.IsEmpty())
				continue;

			string key = "";
			if (!scene.IsEmpty() && (c.m_sScene == scene || scene.StartsWith(c.m_sScene + "@")))
				key = scene;
			else if (scene.IsEmpty() && s && s.m_Tree)
				key = RZ_DialogueScenes.SceneKey(c, s.m_Tree.m_Ctx);

			if (key.IsEmpty() || facts.Flag("scene." + key) == 0)
				continue;

			facts.SetFlag("scene." + key, 0);
			foreach (RZ_DNode n : c.m_aNodes)
			{
				foreach (RZ_DChoice ch : n.m_aChoices)
				{
					foreach (RZ_DEffect e : ch.m_aDo)
					{
						if (e.m_sName == "decide")
							facts.SetFlag("scene." + key + "." + e.m_sArg, 0);
					}
				}
			}

			cleared++;
			RZ_Log.Info(string.Format("Dialogue: scene %1 reset by an admin", key));
		}

		return string.Format("%1 scenes undecided.", cleared);
	}

	// ================================================================ Debug
	//------------------------------------------------------------------------------------------------
	//! "/talkdebug" (admins): whether trees are on, the database, the admin's memory, and for the
	//! person the admin is talking to now, every conversation for them and why each one shows or
	//! doesn't (the first `when` term that fails).
	void Debug(int playerId, RZ_TalkSession s)
	{
		if (!RZ_NoticeBoard.IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Dialogue", "Only a server admin can read this.");
			return;
		}

		string text = string.Format("Trees %1. %2 conversations. Memory for %3 players. Yours: %4.", OnOff(IsOn()), m_DB.Count(),
			m_Memory.Count(), m_Memory.Describe(playerId));
		if (s && s.m_Tree)
		{
			RZ_DCtx ctx = s.m_Tree.m_Ctx;
			text += string.Format("\nTalking to %1 (%2):", ctx.m_sKey, RZ_UIText.Clean(ctx.m_aKinds[0], 60));
			array<RZ_DConv> all = {};
			m_DB.ForSpeaker(ctx.m_aKinds, all);
			foreach (RZ_DConv c : all)
			{
				string why = "shows";
				foreach (RZ_DTerm term : c.m_When.m_aTerms)
				{
					RZ_DCond one = new RZ_DCond();
					one.m_aTerms.Insert(term);
					if (!RZ_DialogueCond.Holds(ctx, one))
					{
						why = "fails " + term.m_sKey;
						break;
					}
				}

				text += string.Format("\n  %1: %2", c.m_sId, why);
			}
		}
		else
		{
			text += "\nTalk to someone first to see their conversations.";
		}

		RZ_Log.Info("Dialogue debug: " + text);
		m_GM.NotifyPlayer(playerId, "Dialogue", text);
	}
}
