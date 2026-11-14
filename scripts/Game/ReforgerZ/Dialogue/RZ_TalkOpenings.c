//! An opening's contact as a speaker (docs/DIALOGUE.md, "Who talks"; Guidance/RZ_Openings.c):
//! whoever's outside the door you come out of (someone from before, a stranger, or a wanderer who
//! met an earlier life of yours). They call you over (a subtitle) and "Talk to <name>" is tracked;
//! the talk screen opens only when you talk to them, and then the step that waited for you starts
//! in it, with that step's piece of the story and its ask (RZ_Openings.TalkLines). With Dialogue >
//! Trees on, the conversations `for opening`
//! (dialogue/openings/openings.rzd) add their greeting by your background and topics by the step
//! the chain is at (the character flag `opening.step`). Their one built reply is "What was it you
//! needed?" (choice slot 0). They talk only to the player they waited for. Server only.
class RZ_TalkOpening : RZ_TalkGeneric
{
	static const int HAND_OVER = 38;	// the hand-over reply (RZ_Openings.HandOver)
	static const int WAY = 39;			// your background's way through the favour (RZ_Openings.TakeWay)
	static const int HABIT_BASE = 40;	// + RZ_EPlayerHabit: an answer to who you were (RZ_Openings.TakeHabit)

	protected int m_iOpening;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkOpening(IEntity owner, int opening)
	{
		m_Owner = owner;
		m_iOpening = opening;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Opening Find()
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return null;

		return openings.ById(m_iOpening);
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		RZ_Opening o = Find();
		s.m_iFaction = -1;
		if (!o)
		{
			// The first giver, after sending you on (a negated id), or nobody we know
			string title = "Someone";
			RZ_Openings openings = RZ_Openings.Get();
			if (openings && !openings.SpeakerFor(m_iOpening).IsEmpty())
				title = openings.SpeakerFor(m_iOpening);

			s.m_sHeader = title;
			s.m_sName = title;
			return;
		}

		s.m_sHeader = o.m_sSpeaker;
		s.m_sName = o.m_sName;
		s.m_sSub = RZ_Util.Capitalize(o.m_sWho);
	}

	//------------------------------------------------------------------------------------------------
	//! Their lines (RZ_Openings.TalkLines): when the next step waits for you, its piece of the story,
	//! then its ask; while a step runs, the step again. With a tree, the conversation's greet by your
	//! background says the greeting before them.
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return;

		// Quiet (you came back to them with nothing waiting): nothing first, the step's ask is their
		// one reply. When a step waits for you to talk (Talk to / Back to <them>), or they haven't
		// spoken yet, they say it: the story's piece and the ask
		RZ_Opening o = Find();
		if (s.m_bQuiet && o && o.m_bSpoke && !o.m_bWaitTalk && !o.m_bEnded)
			return;

		openings.TalkLines(playerId, m_iOpening, s.m_Tree != null, s.m_aNew, user);
	}

	//------------------------------------------------------------------------------------------------
	//! When a step waits for you to talk to them (or they have not spoken yet) they speak first (the
	//! step's piece of the story and its ask); otherwise the screen opens on your replies.
	override bool SpeaksFirst(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		RZ_Opening o = Find();
		return o && (o.m_bWaitTalk || !o.m_bSpoke);
	}

	//------------------------------------------------------------------------------------------------
	//! Their replies, and while a hand-over step runs (the cut, the fever, the clock) the hand-over
	//! itself, first and tagged [Errand], locked with what's missing when you don't carry it. Only
	//! picking it hands the thing over: opening the talk never does.
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		super.Replies(gm, s, playerId, user, replies);
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return;

		// The opening's step under way: what its replies move (the tracked job's alone, RZ_Dialogue)
		int step = -1;
		RZ_Opening o = openings.ById(m_iOpening);
		if (o && o.m_iPlayer == playerId)
			step = o.m_iErrand;

		// Beat 1: who you were (three answers, each a habit), first while the question waits
		if (openings.HabitReplies(playerId, m_iOpening))
		{
			for (int h = RZ_EPlayerHabit.LIAR; h >= RZ_EPlayerHabit.STEADY; h--)
			{
				RZ_TalkReply hr = Reply(HABIT_BASE + h, RZ_OpeningFavours.HabitLabel(h));
				hr.m_bOwn = true;
				hr.m_sTag = RZ_TalkJobs.Tag(RZ_Habits.Name(h));
				hr.m_iRank = RZ_TalkReply.RANK_ON;
				hr.m_iFor = RZ_Dialogue.ForErrand(step);
				replies.InsertAt(hr, 0);
			}
		}

		// Beat 2's other way: your background's, tagged with it
		string wayLabel;
		string wayTag;
		string wayNeed;
		if (openings.WayReply(playerId, m_iOpening, user, wayLabel, wayTag, wayNeed))
		{
			RZ_TalkReply wr = Reply(WAY, wayLabel);
			wr.m_bOwn = true;
			wr.m_bJob = true;
			wr.m_sTag = wayTag;
			wr.m_iRank = RZ_TalkReply.RANK_ON;
			wr.m_iFor = RZ_Dialogue.ForErrand(step);
			if (!wayNeed.IsEmpty())
			{
				wr.m_bEnabled = false;
				wr.m_sNeed = wayNeed;
				wr.m_iRank = RZ_TalkReply.RANK_LOCKED;
			}

			replies.InsertAt(wr, 0);
		}

		string label;
		string need;
		if (!openings.HandOverReply(playerId, m_iOpening, user, label, need))
			return;

		RZ_TalkReply r = Reply(HAND_OVER, label);
		r.m_bOwn = true;
		r.m_bJob = true;
		r.m_sTag = RZ_TalkJobs.Tag("Errand");
		r.m_iRank = RZ_TalkReply.RANK_ON;
		r.m_iFor = RZ_Dialogue.ForErrand(step);
		if (!need.IsEmpty())
		{
			r.m_bEnabled = false;
			r.m_sNeed = need;
			r.m_iRank = RZ_TalkReply.RANK_LOCKED;
		}

		replies.InsertAt(r, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (replyId == HAND_OVER)
		{
			if (openings)
				openings.HandOver(playerId, m_iOpening, user, s.m_aNew);

			return;
		}

		if (replyId == WAY)
		{
			if (openings)
				openings.TakeWay(playerId, m_iOpening, user, s.m_aNew);

			return;
		}

		if (replyId > HABIT_BASE && replyId <= HABIT_BASE + RZ_EPlayerHabit.LIAR)
		{
			if (openings)
				openings.TakeHabit(playerId, m_iOpening, replyId - HABIT_BASE, s.m_aNew);

			return;
		}

		super.Pick(gm, s, playerId, replyId, user);
	}

	//------------------------------------------------------------------------------------------------
	//! opening, for the player they waited for only. Memory: opening:<id>. Factionless; nobody
	//! leans on them through talk.
	override bool Describe(RZ_GameModeComponent gm, RZ_TalkSession s, RZ_DCtx ctx)
	{
		RZ_Opening o = Find();
		if (!o || o.m_iPlayer != s.m_iPlayer)
			return false;

		ctx.m_aKinds.Insert("opening");
		ctx.m_sKey = "opening:" + o.m_iId.ToString();
		ctx.m_iFaction = -1;
		ctx.m_sName = o.m_sName;
		ctx.m_sShort = o.m_sShort;
		ctx.m_sRole = o.m_sWho;
		ctx.m_sPlace = o.m_sTown;
		ctx.m_bStronghold = true;
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! The opening's fixer as a speaker (Guidance/RZ_OpeningFixer.c): Ferda Kroupa, who collects for the
//! Red Hand's book, and the neighbour you can warn after a sale. Their replies are the ways through
//! his visit, first in the list; only picking one does anything (docs/MISSION_RULES.md, rule 19).
//! They talk only to the player the visit is for. No conversations of their own. Server only.
class RZ_TalkFixer : RZ_TalkSpeaker
{
	protected IEntity m_Owner;
	protected int m_iCode;		// the visit's id * 10 + the role (RZ_OpeningFixer.ROLE_*)

	//------------------------------------------------------------------------------------------------
	void RZ_TalkFixer(IEntity owner, int code)
	{
		m_Owner = owner;
		m_iCode = code;
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		RZ_OpeningFixer f = RZ_OpeningFixer.Get();
		if (f)
			f.Setup(m_iCode, s);
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		RZ_OpeningFixer f = RZ_OpeningFixer.Get();
		if (f)
			f.Greet(m_iCode, playerId, s.m_aNew);
	}

	//------------------------------------------------------------------------------------------------
	override bool SpeaksFirst(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		RZ_OpeningFixer f = RZ_OpeningFixer.Get();
		return f && f.SpeaksFirst(m_iCode, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		RZ_OpeningFixer f = RZ_OpeningFixer.Get();
		if (f)
			f.Replies(m_iCode, playerId, user, replies);
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		RZ_OpeningFixer f = RZ_OpeningFixer.Get();
		if (f)
			f.Pick(m_iCode, playerId, replyId, user, s.m_aNew);
	}
}
