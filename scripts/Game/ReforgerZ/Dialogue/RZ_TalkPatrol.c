//! A road patrol's leader on the talk screen (Factions/RZ_RoadPatrols.c): road news for anyone, a
//! stretch walked with them for the Known, papers (Bastion) or the road's toll (the Red Hand) for
//! strangers. The Hunted are fired on before anyone talks.

//------------------------------------------------------------------------------------------------
class RZ_TalkPatrol : RZ_TalkGeneric
{
	static const int BASE = 70;			// + the patrol's own reply id (RZ_RoadPatrols.REPLY_*)

	protected int m_iPatrol;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkPatrol(IEntity owner, int patrol)
	{
		m_Owner = owner;
		m_iPatrol = patrol;
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		RZ_RoadPatrols patrols = RZ_RoadPatrols.Get();
		if (!patrols)
			return;

		RZ_RoadPatrol p = patrols.GetPatrol(m_iPatrol);
		if (!p)
			return;

		s.m_iFaction = p.m_eFaction;
		s.m_sHeader = p.Title();
		FromHeader(gm, s, p.Title());
	}

	//------------------------------------------------------------------------------------------------
	override bool SpeaksFirst(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		RZ_RoadPatrols patrols = RZ_RoadPatrols.Get();
		if (!patrols)
			return;

		RZ_RoadPatrol p = patrols.GetPatrol(m_iPatrol);
		if (p)
			s.m_aNew.Insert(patrols.Greeting(p, playerId));
	}

	//------------------------------------------------------------------------------------------------
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		super.Replies(gm, s, playerId, user, replies);
		RZ_RoadPatrols patrols = RZ_RoadPatrols.Get();
		if (!patrols)
			return;

		array<int> ids = {};
		array<string> labels = {};
		array<string> tags = {};
		array<string> needs = {};
		patrols.Replies(m_iPatrol, playerId, ids, labels, tags, needs);
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
		RZ_RoadPatrols patrols = RZ_RoadPatrols.Get();
		if (patrols && replyId > BASE && replyId < BASE + 10)
		{
			string said = patrols.Pick(m_iPatrol, playerId, replyId - BASE, user);
			if (!said.IsEmpty())
				s.m_aNew.Insert(said);

			return;
		}

		super.Pick(gm, s, playerId, replyId, user);
	}
}
