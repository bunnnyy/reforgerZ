//! An opening's contact as a speaker (docs/DIALOGUE.md, "Who talks"; Guidance/RZ_Openings.c):
//! whoever's outside the door you come out of (someone from before, a stranger, or a wanderer who
//! met an earlier life of yours). They speak first:
//! the opening opens the talk screen on them as each step starts, with that step's piece of the
//! story (RZ_Openings.TalkLines). With Dialogue > Trees on, the conversations `for opening`
//! (dialogue/openings/openings.rzd) add their greeting by your background and topics by the step
//! the chain is at (the character flag `opening.step`). Their one built reply is "What was it you
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
	//! Their lines (RZ_Openings.TalkLines): when they speak first, the step's piece of the story,
	//! then the step's ask follows; when you come back, the step again. With a tree, the
	//! conversation's greet by your background says the greeting before them.
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return;

		// Quiet (you came back to them): nothing first, the step's ask is their one reply
		if (s.m_bQuiet)
			return;

		openings.TalkLines(playerId, m_iOpening, s.m_Tree != null, s.m_aNew);
	}

	//------------------------------------------------------------------------------------------------
	//! As each step starts they speak first (the step's piece of the story and its ask); when you
	//! come back on your own, the screen opens on your replies (What was it you needed?).
	override bool SpeaksFirst(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		RZ_Opening o = Find();
		return o && o.m_bAnnounce;
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
