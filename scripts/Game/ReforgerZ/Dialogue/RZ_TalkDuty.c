//! A duty's person to talk to (docs/DIALOGUE.md, "Who talks"; docs/ERRANDS.md, "Duties"): someone
//! who comes up at a duty's beat (a trader at the Haven's swap table, a passer-by at the Red Hand's
//! toll, an arrival at Bastion's gate) and stands a step behind its work sites while the beat is
//! open. Their conversations are for duty:<beat> ("duty:swap:ring") and duty; a reply that runs
//! call:slot:N does the beat's option N, exactly as doing it with your hands (RZ_DutyScenes.TalkPick),
//! and what follows is said into the talk screen. Only the duty's own player can settle it.
//! Server side.

//------------------------------------------------------------------------------------------------
class RZ_TalkDutyPerson : RZ_TalkGeneric
{
	protected int m_iErrand;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkDutyPerson(IEntity owner, int errandId)
	{
		m_Owner = owner;
		m_iErrand = errandId;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_DutyScenes Scenes(RZ_GameModeComponent gm)
	{
		RZ_Errands errands = gm.GetErrands();
		if (!errands)
			return null;

		return errands.GetScenes();
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_DutyBeat Beat(RZ_GameModeComponent gm)
	{
		RZ_DutyScenes scenes = Scenes(gm);
		if (!scenes)
			return null;

		return RZ_DutyScenes.OpenBeatOf(scenes.ByPerson(m_iErrand));
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		RZ_DutyBeat b = Beat(gm);
		RZ_DutyScene scene = null;
		RZ_DutyScenes scenes = Scenes(gm);
		if (scenes)
			scene = scenes.ByPerson(m_iErrand);

		string name = "Someone";
		if (b && !b.m_sTalkName.IsEmpty())
			name = b.m_sTalkName;

		s.m_sHeader = name;
		s.m_sName = name;
		s.m_sSub = "";
		if (scene)
			s.m_sSub = scene.m_sTitle;

		s.m_iFaction = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! They came up to you: what they've brought or what they want is said first.
	override bool SpeaksFirst(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		RZ_DutyBeat b = Beat(gm);
		if (!b)
		{
			s.m_aNew.Insert("They've said their piece. It's settled now.");
			return;
		}

		if (!b.m_sLine.IsEmpty())
			s.m_aNew.Insert(b.m_sLine);
	}

	//------------------------------------------------------------------------------------------------
	//! duty:<beat> ("duty:swap:ring", "duty:toll:courier", "duty:gate:widow"), duty:<duty>
	//! ("duty:swap"), duty. Memory: duty:<errand>:<beat>. Nobody leans on them through talk: the
	//! duty's own calls do that.
	override bool Describe(RZ_GameModeComponent gm, RZ_TalkSession s, RZ_DCtx ctx)
	{
		RZ_DutyBeat b = Beat(gm);
		if (!b || b.m_sTalk.IsEmpty())
			return false;

		RZ_DutyScene scene = Scenes(gm).ByPerson(m_iErrand);
		ctx.m_aKinds.Insert("duty:" + b.m_sTalk);
		int colon = b.m_sTalk.IndexOf(":");
		if (colon > 0)
			ctx.m_aKinds.Insert("duty:" + b.m_sTalk.Substring(0, colon));

		ctx.m_aKinds.Insert("duty");
		ctx.m_sKey = string.Format("duty:%1:%2", m_iErrand, b.m_sTalk);
		ctx.m_iFaction = -1;
		ctx.m_sName = b.m_sTalkName;
		ctx.m_sShort = b.m_sTalkName;
		ctx.m_sRole = "duty";
		ctx.m_bStronghold = true;
		if (scene && scene.m_iHouse >= 0)
			ctx.m_iHouse = scene.m_iHouse;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Every slot is the beat's: a conversation's call:slot:N runs option N.
	override string SlotJob(RZ_GameModeComponent gm, int slot)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! call:slot:N: the beat's option N, as if done with your hands.
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		RZ_DutyScenes scenes = Scenes(gm);
		if (!scenes || !scenes.ByPerson(m_iErrand))
		{
			s.m_aNew.Insert("It's settled already.");
			return;
		}

		scenes.TalkPick(m_iErrand, replyId, playerId, user);
	}
}
