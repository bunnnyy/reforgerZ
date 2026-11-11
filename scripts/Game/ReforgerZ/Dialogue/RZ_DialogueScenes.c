//! Dialogue's multiplayer layer (docs/DIALOGUE.md, "Several players"): who hears what, deciding
//! together, tallies, vouching, call-outs and point-outs. Server only, with Dialogue > Trees.
//! - Overheard: a `public` node's lines and the chosen label go to other players within 8 m, owner-only,
//!   at most once every 10 s per listener. Tags are stripped.
//! - Decide: a choice with `decide:<value>` in a conversation with a `scene` doesn't land at once: a 5 s
//!   hold, told to the room (30 m). Anyone else talking to that speaker may Object (once per scene) or
//!   Stand with. Unobjected, the pick runs and the scene's record is written (`decided` reads it);
//!   world effects run once. Personal effects go only to the picker.
//! - Tally: `sway:<name>` adds one contribution per player to a count its owner reads.
//! - Vouch: Trusted or better with the speaker's faction, within 8 m of someone talking to them: +1
//!   at their persuade and lie checks there for 5 minutes. A lie told under it, found out, costs the
//!   voucher +5 Infamy and regard (RZ_Lies). Once per pair per hour.
//! - Call it out: for 20 s after someone near picks a choice that tells the speaker something (a claim
//!   or a lie). A lie: caught at once. True: the caller takes regard *slander*. Once per pair per hour.
//! - Point them out: Known or better with the speaker's faction, near someone in that faction's kit:
//!   -1 at their next look (two at most). Their own colours: regard *slander* for the pointer.
class RZ_DSceneHold
{
	int m_iDecider;
	string m_sScene;
	IEntity m_Speaker;		// weak
	int m_iChoice;
	int m_iResult;			// RZ_ECheck
	string m_sVerdict;
	float m_fLeft;
	vector m_vRoom;
	ref array<int> m_aStood = {};
}

//------------------------------------------------------------------------------------------------
class RZ_DClaimSeen
{
	IEntity m_Speaker;		// weak
	float m_fAt;
	RZ_Lie m_Lie;			// the stored lie, null if it was true
	string m_sSpeakerKey;
	int m_iFaction;
}

//------------------------------------------------------------------------------------------------
class RZ_DialogueScenes
{
	static const int VOUCH = 90;
	static const int CALLOUT = 91;
	static const int POINT = 92;
	static const int OBJECT = 93;
	static const int STAND = 94;
	static const float HEAR = 8;
	static const float ROOM = 30;
	static const float HEAR_GAP = 10;
	static const float HOLD = 5;
	static const float VOUCH_SECONDS = 300;
	static const float PAIR_GAP = 3600;
	static const float CALLOUT_SECONDS = 20;

	protected RZ_GameModeComponent m_GM;
	protected float m_fClock;
	protected ref array<ref RZ_DSceneHold> m_aHolds = {};
	protected ref map<int, int> m_mVoucher = new map<int, int>();			// vouched player -> voucher
	protected ref map<int, float> m_mVouchUntil = new map<int, float>();
	protected ref map<string, float> m_mPairs = new map<string, float>();	// "kind|a|b" -> clock
	protected ref map<string, float> m_mHeard = new map<string, float>();	// listener -> clock
	protected ref map<int, ref RZ_DClaimSeen> m_mClaims = new map<int, ref RZ_DClaimSeen>();
	protected ref array<string> m_aObjected = {};						// "scene|ident"

	//------------------------------------------------------------------------------------------------
	void RZ_DialogueScenes(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOurs(int replyId)
	{
		return replyId >= VOUCH && replyId <= STAND;
	}

	//------------------------------------------------------------------------------------------------
	protected bool PairFree(string kind, int a, int b)
	{
		float at;
		string key = string.Format("%1|%2|%3", kind, m_GM.GetProfile(a).m_sIdentity, m_GM.GetProfile(b).m_sIdentity);
		return !m_mPairs.Find(key, at) || m_fClock - at >= PAIR_GAP;
	}

	//------------------------------------------------------------------------------------------------
	protected void UsePair(string kind, int a, int b)
	{
		m_mPairs.Set(string.Format("%1|%2|%3", kind, m_GM.GetProfile(a).m_sIdentity, m_GM.GetProfile(b).m_sIdentity), m_fClock);
	}

	//------------------------------------------------------------------------------------------------
	//! Other players in a session with the same speaker within `radius` of `at`.
	protected void Others(RZ_TalkSession s, float radius, notnull array<int> result)
	{
		RZ_Dialogue dialogue = m_GM.GetDialogue();
		if (!dialogue || !s.m_Speaker)
			return;

		array<RZ_TalkSession> with = {};
		dialogue.SessionsWith(s.m_Speaker, with);
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayer);
		foreach (RZ_TalkSession o : with)
		{
			if (o.m_iPlayer == s.m_iPlayer)
				continue;

			IEntity them = GetGame().GetPlayerManager().GetPlayerControlledEntity(o.m_iPlayer);
			if (me && them && RZ_Util.DistXZ(me.GetOrigin(), them.GetOrigin()) <= radius)
				result.Insert(o.m_iPlayer);
		}
	}

	// ================================================================ The hub's social verbs
	//------------------------------------------------------------------------------------------------
	//! Vouch, Call it out, Point them out, Object and Stand with, when they apply.
	void HubVerbs(RZ_TalkSession s, notnull array<ref RZ_TalkReply> replies)
	{
		int f = s.m_iFaction;
		int me = s.m_iPlayer;
		RZ_DSceneHold hold = HoldAt(s.m_Speaker);
		if (hold && hold.m_iDecider != me)
		{
			if (!m_aObjected.Contains(hold.m_sScene + "|" + m_GM.GetProfile(me).m_sIdentity))
				replies.Insert(Verb(OBJECT, "Object", "once"));

			if (!hold.m_aStood.Contains(me))
				replies.Insert(Verb(STAND, "Stand with " + RZ_Util.PlayerName(hold.m_iDecider), "shares it"));
		}

		array<int> near = {};
		Others(s, HEAR, near);
		if (near.IsEmpty())
			return;

		int other = near[0];
		if (RZ_Factions.IsValid(f) && RZ_Reputation.IsAtLeast(m_GM.GetTier(me, f), RZ_ETier.TRUSTED) && PairFree("vouch", me, other) && !m_mVoucher.Contains(other))
			replies.Insert(Verb(VOUCH, string.Format("\"%1's with me.\"", RZ_Util.PlayerName(other)), "Vouch: you answer for them"));

		RZ_DClaimSeen claim;
		if (m_mClaims.Find(other, claim) && claim.m_Speaker == s.m_Speaker && m_fClock - claim.m_fAt <= CALLOUT_SECONDS && PairFree("callout", me, other))
			replies.Insert(Verb(CALLOUT, string.Format("\"%1's lying.\"", RZ_Util.PlayerName(other)), "Call it out"));

		RZ_Hostility hostility = m_GM.GetHostility();
		if (RZ_Factions.IsValid(f) && hostility && hostility.GetDisguise().KitLevel(other, f) > 0 && RZ_Reputation.IsAtLeast(m_GM.GetTier(me, f), RZ_ETier.KNOWN) && PairFree("point", me, other))
			replies.Insert(Verb(POINT, "\"That one's not yours.\"", "Point them out"));
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TalkReply Verb(int id, string label, string tag)
	{
		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = id;
		r.m_sLabel = label;
		r.m_sTag = tag;
		r.m_bOwn = true;
		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! One of the verbs above. Lines go into s.m_aNew.
	void PickVerb(RZ_TalkSession s, int id, IEntity user)
	{
		int me = s.m_iPlayer;
		int f = s.m_iFaction;
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (id == OBJECT || id == STAND)
		{
			RZ_DSceneHold hold = HoldAt(s.m_Speaker);
			if (!hold)
			{
				s.m_aNew.Insert("It's already settled.");
				return;
			}

			if (id == STAND)
			{
				hold.m_aStood.Insert(me);
				s.m_aNew.Insert(string.Format("You stand with %1.", RZ_Util.PlayerName(hold.m_iDecider)));
				return;
			}

			string key = hold.m_sScene + "|" + m_GM.GetProfile(me).m_sIdentity;
			if (m_aObjected.Contains(key))
				return;

			m_aObjected.Insert(key);
			TellRoom(hold.m_vRoom, string.Format("%1 objects. Nothing is decided yet.", RZ_Util.PlayerName(me)));
			RZ_Log.Info(string.Format("Dialogue: scene %1 hold by %2 objected to by %3", hold.m_sScene, RZ_Util.PlayerName(hold.m_iDecider), RZ_Util.PlayerName(me)));
			m_aHolds.RemoveItem(hold);
			return;
		}

		array<int> near = {};
		Others(s, HEAR, near);
		if (near.IsEmpty())
		{
			s.m_aNew.Insert("Nobody's there to say it about.");
			return;
		}

		int other = near[0];
		string speakerKey = "";
		if (s.m_Tree)
			speakerKey = s.m_Tree.m_Ctx.m_sKey;

		if (id == VOUCH)
		{
			UsePair("vouch", me, other);
			m_mVoucher.Set(other, me);
			m_mVouchUntil.Set(other, m_fClock + VOUCH_SECONDS);
			s.m_aNew.Insert(string.Format("\"%1's with me.\" A nod. You answer for them now.", RZ_Util.PlayerName(other)));
			m_GM.NotifyPlayer(other, s.m_sHeader, string.Format("%1 vouches for you here.", RZ_Util.PlayerName(me)));
			RZ_Log.Info(string.Format("Dialogue: %1 vouched for %2", RZ_Util.PlayerName(me), RZ_Util.PlayerName(other)));
			return;
		}

		if (id == CALLOUT)
		{
			UsePair("callout", me, other);
			RZ_DClaimSeen claim;
			m_mClaims.Find(other, claim);
			m_mClaims.Remove(other);
			RZ_WorldFacts facts = m_GM.GetFacts();
			if (claim && claim.m_Lie && facts)
			{
				facts.Memory().Lies().CatchNow(m_GM, claim.m_Lie, "call-out");
				s.m_aNew.Insert(string.Format("\"%1's lying.\" The look on their face says you're right.", RZ_Util.PlayerName(other)));
				return;
			}

			if (trees && !speakerKey.IsEmpty())
				trees.GetMemory().AddRegard(me, speakerKey, "slander");

			s.m_aNew.Insert("\"Calling an honest man a liar. Nice.\"");
			return;
		}

		if (id == POINT)
		{
			UsePair("point", me, other);
			RZ_Disguise disguise = m_GM.GetHostility().GetDisguise();
			if (disguise.IsOwnColours(other, f))
			{
				if (trees && !speakerKey.IsEmpty())
					trees.GetMemory().AddRegard(me, speakerKey, "slander");

				s.m_aNew.Insert("\"That one's ours. Who are you again?\"");
				return;
			}

			disguise.PointOut(other, f);
			s.m_aNew.Insert("\"That one's not yours.\" Eyes turn. They'll look harder at that face.");
		}
	}

	// ================================================================ Vouches and claims
	//------------------------------------------------------------------------------------------------
	//! +1 at persuade and lie checks while a voucher stands within 8 m.
	int VouchBonus(int playerId)
	{
		int voucher;
		float until;
		if (!m_mVoucher.Find(playerId, voucher) || !m_mVouchUntil.Find(playerId, until) || m_fClock > until)
			return 0;

		IEntity a = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		IEntity b = GetGame().GetPlayerManager().GetPlayerControlledEntity(voucher);
		if (!a || !b || RZ_Util.DistXZ(a.GetOrigin(), b.GetOrigin()) > HEAR)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The identity who vouches for this player now, "" none.
	string Voucher(int playerId)
	{
		if (VouchBonus(playerId) <= 0)
			return "";

		int voucher;
		m_mVoucher.Find(playerId, voucher);
		return m_GM.GetProfile(voucher).m_sIdentity;
	}

	//------------------------------------------------------------------------------------------------
	//! A choice that told the speaker something: call-outs are open for 20 s.
	void NoteClaim(RZ_TalkSession s, RZ_Lie lie)
	{
		RZ_DClaimSeen c = new RZ_DClaimSeen();
		c.m_Speaker = s.m_Speaker;
		c.m_fAt = m_fClock;
		c.m_Lie = lie;
		c.m_iFaction = s.m_iFaction;
		m_mClaims.Set(s.m_iPlayer, c);
	}

	// ================================================================ Overheard
	//------------------------------------------------------------------------------------------------
	//! A public node: its lines and the chosen label to others within 8 m (tags stripped).
	void Overheard(RZ_TalkSession s, string label, array<string> lines)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayer);
		if (!me)
			return;

		// On air: everyone within the channel's reach, no name, no tags (nothing personal on air)
		bool onAir = RZ_TalkRadio.Cast(s.m_Adapter) != null;
		float reach = HEAR;
		string text = "";
		if (onAir)
		{
			reach = RZ_TalkRadio.REACH;
			if (!label.IsEmpty())
				text = "On the open channel, someone: " + StripTags(label);
		}
		else if (!label.IsEmpty())
		{
			text = string.Format("%1, to %2: %3", RZ_Util.PlayerName(s.m_iPlayer), s.m_sName, StripTags(label));
		}

		foreach (string l : lines)
		{
			if (!text.IsEmpty())
				text += "\n";

			text += l;
		}

		if (text.IsEmpty())
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			int other = ids[i];
			if (other == s.m_iPlayer || RZ_Util.DistXZ(p.GetOrigin(), me.GetOrigin()) > reach)
				continue;

			float last;
			string key = other.ToString();
			if (m_mHeard.Find(key, last) && m_fClock - last < HEAR_GAP)
				continue;

			m_mHeard.Set(key, m_fClock);
			if (onAir)
				m_GM.NotifyPlayer(other, "The open channel", text);
			else
				m_GM.NotifyPlayer(other, "Overheard", text);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string StripTags(string label)
	{
		int b = label.IndexOf("[");
		if (b > 0)
			return label.Substring(0, b).Trim();

		return label;
	}

	// ================================================================ Deciding together
	//------------------------------------------------------------------------------------------------
	//! The scene's record key: per speaker, or once for the season.
	static string SceneKey(RZ_DConv c, RZ_DCtx ctx)
	{
		if (!c || c.m_sScene.IsEmpty())
			return "";

		if (c.m_sSceneScope == "season")
			return c.m_sScene;

		return c.m_sScene + "@" + ctx.m_sKey;
	}

	//------------------------------------------------------------------------------------------------
	//! A decision pending at this speaker, or null.
	RZ_DSceneHold HoldAt(IEntity speaker)
	{
		foreach (RZ_DSceneHold h : m_aHolds)
		{
			if (h.m_Speaker == speaker)
				return h;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Start the 5 s hold on a deciding pick. False if one is already running at this speaker.
	bool StartHold(RZ_TalkSession s, string scene, int choice, int result, string verdict, string what)
	{
		if (HoldAt(s.m_Speaker))
			return false;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayer);
		if (!me)
			return false;

		RZ_DSceneHold h = new RZ_DSceneHold();
		h.m_iDecider = s.m_iPlayer;
		h.m_sScene = scene;
		h.m_Speaker = s.m_Speaker;
		h.m_iChoice = choice;
		h.m_iResult = result;
		h.m_sVerdict = verdict;
		h.m_fLeft = HOLD;
		h.m_vRoom = me.GetOrigin();
		m_aHolds.Insert(h);
		TellRoom(h.m_vRoom, string.Format("%1 is about to answer: %2. Anyone talking to them can object, or stand with them.", RZ_Util.PlayerName(s.m_iPlayer), StripTags(what)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TellRoom(vector at, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), at) <= ROOM)
				m_GM.NotifyPlayer(ids[i], "The room", text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The holds that ran out land (the decider's pick runs and the record is written).
	void Tick(float dt)
	{
		m_fClock += dt;
		RZ_Dialogue dialogue = m_GM.GetDialogue();
		for (int i = m_aHolds.Count() - 1; i >= 0; i--)
		{
			RZ_DSceneHold h = m_aHolds[i];
			h.m_fLeft -= dt;
			if (h.m_fLeft > 0)
				continue;

			m_aHolds.Remove(i);
			if (dialogue)
				dialogue.FinishHold(h);
		}
	}
}
