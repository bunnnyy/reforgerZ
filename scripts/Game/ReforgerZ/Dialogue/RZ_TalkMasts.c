//! A mast's keeper on the talk screen (World/RZ_Masts.c): the old radio man who lives at its foot,
//! or the Red Hand crewman who holds it for the book. What it takes to climb is settled here, as
//! replies (docs/MISSION_RULES.md, rule 19: a hand-over is a reply you pick).

//------------------------------------------------------------------------------------------------
class RZ_TalkMastKeeper : RZ_TalkGeneric
{
	static const int BASE = 60;			// + the keeper's own reply id (RZ_Masts.KeeperReplies)

	protected int m_iMast;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkMastKeeper(IEntity owner, int mast)
	{
		m_Owner = owner;
		m_iMast = mast;
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		RZ_Masts masts = RZ_Masts.Get();
		if (!masts)
			return;

		string title = masts.KeeperTitleOf(m_iMast);
		s.m_sHeader = title;
		FromHeader(gm, s, title);
		RZ_Mast m = masts.GetMast(m_iMast);
		if (m && m.m_eTrouble == RZ_EMastTrouble.CREW)
			s.m_iFaction = RZ_EFaction.BANDITS;
		else
			s.m_iFaction = -1;
	}

	//------------------------------------------------------------------------------------------------
	override bool SpeaksFirst(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		RZ_Masts masts = RZ_Masts.Get();
		if (!masts)
			return;

		string line = masts.KeeperGreeting(m_iMast, playerId);
		if (!line.IsEmpty())
			s.m_aNew.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		super.Replies(gm, s, playerId, user, replies);
		RZ_Masts masts = RZ_Masts.Get();
		if (!masts)
			return;

		array<int> ids = {};
		array<string> labels = {};
		array<string> tags = {};
		array<string> needs = {};
		masts.KeeperReplies(m_iMast, playerId, user, ids, labels, tags, needs);
		for (int i = ids.Count() - 1; i >= 0; i--)
		{
			RZ_TalkReply r = Reply(BASE + ids[i], labels[i]);
			r.m_bOwn = true;
			r.m_sTag = tags[i];
			r.m_iRank = RZ_TalkReply.RANK_ON;
			if (!needs[i].IsEmpty())
			{
				r.m_bEnabled = false;
				r.m_sNeed = needs[i];
				r.m_iRank = RZ_TalkReply.RANK_LOCKED;
			}

			replies.InsertAt(r, 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		RZ_Masts masts = RZ_Masts.Get();
		if (masts && replyId > BASE && replyId < BASE + 10)
		{
			string said = masts.KeeperPick(m_iMast, playerId, replyId - BASE, user);
			if (!said.IsEmpty())
				s.m_aNew.Insert(said);

			return;
		}

		super.Pick(gm, s, playerId, replyId, user);
	}
}
