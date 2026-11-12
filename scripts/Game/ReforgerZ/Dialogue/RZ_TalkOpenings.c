//! An opening's contact as a speaker (docs/DIALOGUE.md, "Who talks"; Guidance/RZ_Openings.c):
//! someone who knew you before the outbreak, waiting outside your hiding place. They speak first:
//! the opening opens the talk screen on them. With Dialogue > Trees on, the conversations
//! `for opening` (dialogue/openings/openings.rzd) are their greeting, by your background, and the
//! story told on the way, by the step the chain is at (the character flag `opening.step`); without
//! Trees, one line of theirs and what they need now. Their one reply is "What was it you
//! needed?" (choice slot 0). They talk only to the player they waited for. Server only.
class RZ_TalkOpening : RZ_TalkGeneric
{
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
			s.m_sHeader = "Someone";
			s.m_sName = "Someone";
			return;
		}

		s.m_sHeader = o.m_sSpeaker;
		s.m_sName = o.m_sName;
		s.m_sSub = RZ_Util.Capitalize(o.m_sWho);
	}

	//------------------------------------------------------------------------------------------------
	//! With a tree, the conversation's greet says it; without one, their line and the step now.
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		if (s.m_Tree)
			return;

		RZ_Openings openings = RZ_Openings.Get();
		if (openings)
			openings.HandleTalk(playerId, m_iOpening, m_Owner);
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
		ctx.m_sShort = o.m_sName;
		int space = o.m_sName.IndexOf(" ");
		if (space > 0)
			ctx.m_sShort = o.m_sName.Substring(0, space);

		ctx.m_sRole = o.m_sWho;
		ctx.m_sPlace = o.m_sTown;
		ctx.m_bStronghold = true;
		return true;
	}
}
