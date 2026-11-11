//! The talk screen's adapters (Dialogue/RZ_Dialogue.c): who a person is, how they greet you, what
//! you can say to them, and what each reply runs. Server side.

//------------------------------------------------------------------------------------------------
//! The base: a person's name and sub-line from the title their lines come under
//! ("Ines Calder, council head in Morton" is Ines Calder, then the rest).
class RZ_TalkSpeaker
{
	//------------------------------------------------------------------------------------------------
	//! The adapter for a person: a leader's own, else the generic one.
	static RZ_TalkSpeaker For(RZ_GameModeComponent gm, IEntity owner)
	{
		RZ_ClueComponent clue = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		if (clue && clue.GetMissionId() == RZ_Leaders.TAG && gm.GetLeaders())
		{
			RZ_Leader l = gm.GetLeaders().Get(clue.GetClueIndex());
			if (l)
				return new RZ_TalkLeader(l);
		}

		return new RZ_TalkGeneric(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Before the greeting: whatever is known of the person (name, sub-line, faction, header).
	void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The first title caught becomes the header; the name and sub-line come from it if unset.
	void FromHeader(RZ_GameModeComponent gm, RZ_TalkSession s, string header)
	{
		string lower = header;
		lower.ToLower();
		if (s.m_iFaction < 0)
			s.m_iFaction = RZ_NoticeStyle.FactionIn(lower);

		if (!s.m_sName.IsEmpty())
			return;

		int comma = header.IndexOf(", ");
		if (comma <= 0)
		{
			s.m_sName = header;
			return;
		}

		s.m_sName = header.Substring(0, comma);
		string rest = header.Substring(comma + 2, header.Length() - comma - 2);
		if (s.m_iFaction >= 0 && s.m_sSub.IsEmpty())
			s.m_sSub = RZ_Factions.GetName(s.m_iFaction) + " - " + rest;
		else if (s.m_sSub.IsEmpty())
			s.m_sSub = rest;
	}

	//------------------------------------------------------------------------------------------------
	void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
	}

	//------------------------------------------------------------------------------------------------
	void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
	}

	//------------------------------------------------------------------------------------------------
	void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_TalkReply Reply(int id, string label)
	{
		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = id;
		r.m_sLabel = label;
		return r;
	}
}

//------------------------------------------------------------------------------------------------
//! Every person with the puzzle components: wanderers, safehouse occupants, named people at
//! holdings, the Long Table's people, the people missions and questlines spawn. The greeting is
//! their Talk action; the replies are their visible choice slots (ids 0-7) and use slots (10-17).
class RZ_TalkGeneric : RZ_TalkSpeaker
{
	protected IEntity m_Owner;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkGeneric(IEntity owner)
	{
		m_Owner = owner;
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		if (m_Owner)
			RZ_PuzzleActionHelper.DoRead(gm, m_Owner, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		if (!m_Owner)
			return;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(m_Owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!comp)
			return;

		for (int slot = 0; slot < 8; slot++)
		{
			string choice = comp.GetSlotLabel(true, slot);
			if (!choice.IsEmpty())
				replies.Insert(Reply(slot, choice));
		}

		for (int u = 0; u < 8; u++)
		{
			string use = comp.GetSlotLabel(false, u);
			if (!use.IsEmpty())
				replies.Insert(Reply(10 + u, use));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		if (!m_Owner)
			return;

		if (replyId >= 10)
			RZ_PuzzleActionHelper.DoUse(gm, m_Owner, playerId, replyId - 10, user);
		else
			RZ_PuzzleActionHelper.DoChoice(gm, m_Owner, playerId, replyId, user);
	}
}

//------------------------------------------------------------------------------------------------
//! A faction leader, on their own body or at their seat (a quartermaster's "Talk to whoever's in
//! charge"): the sub-line is the faction, their title and what they think of you; the replies are
//! asking for work, for news, and what they make of you (RZ_Leaders.HandleChoice 0-2).
class RZ_TalkLeader : RZ_TalkSpeaker
{
	protected RZ_Leader m_Leader;
	protected IEntity m_Seat;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkLeader(RZ_Leader l)
	{
		m_Leader = l;
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		if (!m_Leader)
			return;

		m_Seat = s.m_Speaker;
		s.m_sHeader = m_Leader.GetHeader();
		s.m_sName = m_Leader.m_sName;
		s.m_iFaction = m_Leader.m_eFaction;
		s.m_sSub = string.Format("%1 - %2 - %3", RZ_Factions.GetName(m_Leader.m_eFaction), m_Leader.m_sTitle,
			gm.GetLeaders().AttitudeWords(m_Leader, playerId));
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		if (m_Leader && gm.GetLeaders())
			gm.GetLeaders().HandleTalk(playerId, m_Leader.m_iId, m_Seat);
	}

	//------------------------------------------------------------------------------------------------
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		replies.Insert(Reply(0, "Ask for work"));
		replies.Insert(Reply(1, "Heard anything?"));
		replies.Insert(Reply(2, "What do you make of me?"));
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		if (m_Leader && gm.GetLeaders())
			gm.GetLeaders().HandleChoice(playerId, m_Leader.m_iId, replyId, m_Seat, user);

		// What they think of you can change with the answer
		if (m_Leader && gm.GetLeaders())
			s.m_sSub = string.Format("%1 - %2 - %3", RZ_Factions.GetName(m_Leader.m_eFaction), m_Leader.m_sTitle,
				gm.GetLeaders().AttitudeWords(m_Leader, playerId));
	}
}
