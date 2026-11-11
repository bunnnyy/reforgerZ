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

	//------------------------------------------------------------------------------------------------
	void RZ_DialogueTrees(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		m_DB = new RZ_DialogueDB();
		m_Memory = new RZ_DialogueMemory(gm);
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
		RZ_Log.Info(string.Format("Dialogue: trees %1", OnOff(IsOn())));
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
	}

	// ================================================================ A session
	//------------------------------------------------------------------------------------------------
	//! After the adapter's setup: the speaker as conditions see them. Null when trees are off or
	//! the adapter can't say who it is.
	RZ_DTalk Begin(RZ_TalkSession s, IEntity user)
	{
		if (!IsOn() || !s.m_Adapter)
			return null;

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

		RZ_DTalk t = new RZ_DTalk();
		t.m_Ctx = ctx;

		// Talking to their people undisguised: they know your face this life
		if (RZ_Factions.IsValid(ctx.m_iFaction))
			m_Memory.NoteFace(s.m_iPlayer, ctx.m_iFaction);

		return t;
	}

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

		if (!best || best.m_aNodes.IsEmpty())
			return;

		NoteTopic(t, best);
		Lines(t, best.m_aNodes[0], lines);
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
		if (Refuses(t.m_Ctx))
			return;
		array<RZ_DConv> all = {};
		m_DB.ForSpeaker(t.m_Ctx.m_aKinds, all);
		for (int prio = 0; prio <= 4; prio++)
		{
			array<RZ_DConv> band = {};
			foreach (RZ_DConv c : all)
			{
				if (c.m_iPrio == prio && !c.IsGreet() && TopicFree(t, c) && RZ_DialogueCond.Holds(t.m_Ctx, c.m_When))
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
		if ((c.m_iFlags & (RZ_DConv.FLAG_ONCE | RZ_DConv.FLAG_DAILY)) == 0)
			return true;

		int at = m_Memory.TopicAt(t.m_Ctx.m_iPlayer, t.m_Ctx.m_sKey + "|" + c.m_sId);
		if (at <= 0)
			return true;

		if ((c.m_iFlags & RZ_DConv.FLAG_ONCE) != 0)
			return false;

		return RZ_SeasonClock.Now() - at >= RZ_DialogueMemory.DAY;
	}

	//------------------------------------------------------------------------------------------------
	protected void NoteTopic(RZ_DTalk t, RZ_DConv c)
	{
		if ((c.m_iFlags & (RZ_DConv.FLAG_ONCE | RZ_DConv.FLAG_DAILY)) != 0)
			m_Memory.NoteTopic(t.m_Ctx.m_iPlayer, t.m_Ctx.m_sKey + "|" + c.m_sId);
	}

	//------------------------------------------------------------------------------------------------
	//! Our reply ids: topics, choices and More.
	static bool IsOurs(int replyId)
	{
		return replyId == MORE || replyId >= TOPIC_BASE || RZ_DialogueScenes.IsOurs(replyId);
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
			if (!RZ_DialogueCond.Holds(t.m_Ctx, c.m_If))
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
		if (!RZ_DialogueCond.Holds(ctx, c.m_If) || !RZ_DialogueCond.Holds(ctx, t.m_Conv.m_When))
		{
			s.m_aNew.Insert("They didn't catch that.");
			s.m_aNew.InsertAll(s.m_aSaid);
			return true;
		}

		// A price is never taken off a label that hasn't refreshed yet
		if (!c.m_sCostKind.IsEmpty() && RZ_UIW.Now() - t.m_fNodeAt < FRESH_NODE)
			return true;

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

			result = RZ_Checks.Threaten(m_GM, pid, ctx.m_sKey, c.m_iCheckFaction, -1, Difficulty(ctx, c), ctx.m_bStronghold, RZ_Checks.THREAT_INFAMY, c.m_sFavoured);
			m_Memory.AddRegard(pid, ctx.m_sKey, "threatened");
			verdict = FearVerdict(result);
		}
		else if (c.m_sCheck == "lie")
		{
			result = TellLie(s, c);
			verdict = LieVerdict(result);
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

		RZ_ECheck result = RZ_Checks.Lie(m_GM, pid, c.m_iCheckFaction, Difficulty(ctx, c), HasTell(ctx), bonus);
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

			return result;
		}

		// Caught now
		m_Memory.AddRegard(pid, ctx.m_sKey, "lied", LieWeight(ctx));
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

		if (scenes && HasEffect(c.m_aDo, "claim") && c.m_sCheck != "lie")
			scenes.NoteClaim(s, null);

		return goOn;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LandPick(RZ_TalkSession s, RZ_DChoice c, RZ_ECheck result, string verdict, IEntity user)
	{
		RZ_DTalk t = s.m_Tree;

		// The effects, then the next node. Lines the effects say (a built reply's answer, a
		// payment) come after the node's own.
		s.m_aNew.Clear();
		if (result == RZ_ECheck.PASS)
			RunEffects(s, c.m_aDo, user);
		else if (result == RZ_ECheck.SHORT)
			RunEffects(s, c.m_aShort, user);
		else
			RunEffects(s, c.m_aElse, user);

		if (!s.m_Tree)
			return false;

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
		s.m_aNew.InsertAll(lines);
		s.m_aNew.InsertAll(effectLines);
		s.m_aNew.InsertAll(extra);
	}

	//------------------------------------------------------------------------------------------------
	//! A node's lines: a say? whose condition holds replaces the next plain say.
	protected void Lines(RZ_DTalk t, RZ_DNode node, notnull array<string> lines)
	{
		string pending;
		bool hasPending;
		foreach (RZ_DSay item : node.m_aItems)
		{
			if (item.m_bJump || item.m_bDo)
				continue;

			if (!item.m_Cond.IsEmpty())
			{
				if (!hasPending && RZ_DialogueCond.Holds(t.m_Ctx, item.m_Cond))
				{
					pending = item.m_sText;
					hasPending = true;
				}

				continue;
			}

			if (hasPending)
				lines.Insert(RZ_DialogueText.Fill(t.m_Ctx, pending));
			else
				lines.Insert(RZ_DialogueText.Fill(t.m_Ctx, item.m_sText));

			hasPending = false;
		}

		if (hasPending)
			lines.Insert(RZ_DialogueText.Fill(t.m_Ctx, pending));
	}

	// ================================================================ Checks and tags
	//------------------------------------------------------------------------------------------------
	//! Trust (RZ_Checks.TrustScore) with dialogue's bonus: +1 for regard 2 or more, or the built
	//! helped or saved bit this life. The Long Table's rank counts 0.
	int TrustScore(RZ_DCtx ctx, RZ_DChoice c)
	{
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
		else if (c.m_sCheck == "lie")
		{
			int lieScore = RZ_Checks.TrustScore(m_GM, ctx.m_iPlayer, c.m_iCheckFaction, "", 0);
			if (HasTell(ctx))
				tag = "[Lie - they'd see through it]";
			else
				tag = string.Format("[Lie - %1 of %2]", Math.Max(0, lieScore), Difficulty(ctx, c));
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

		// Who may decide: a deciding choice in a scene with `who` is dimmed for everyone else
		if (r.m_bEnabled && t.m_Conv && t.m_Conv.m_SceneWho && HasEffect(c.m_aDo, "decide") && !RZ_DialogueCond.Holds(ctx, t.m_Conv.m_SceneWho))
		{
			r.m_bEnabled = false;
			r.m_sNeed = "only someone " + ctx.m_sShort + " trusts can answer for it";
		}

		r.m_sTag = tag;
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
					m_Memory.Add(pid, RZ_DialogueMemory.CLAIMS, ctx.m_sKey + "|" + e.m_sArg);
					break;
				case "learn":
					m_Memory.Learn(pid, e.m_sArg);
					break;
				case "rumour":
					Rumour(ctx, e.m_sArg);
					break;
				case "call":
					Call(s, e.m_sArg, user);
					break;
				case "sway":
				{
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
				case "offer":
				{
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
