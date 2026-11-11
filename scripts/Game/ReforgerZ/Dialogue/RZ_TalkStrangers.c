//! Strangers as speakers (docs/DIALOGUE.md, "Strangers"): the errands' strangers (docs/ERRANDS.md,
//! "Strangers") greet you with their own call-out and keep their asks as the hub's verbs (their
//! choice slots, as built). With Dialogue > Trees on, the conversations `for stranger:<routine>`
//! add who they are and why they're there, in their own words ({their:town}, {their:house}...: the
//! routine's texts). Server only.

//------------------------------------------------------------------------------------------------
//! The one way into a stranger's visit by its object index (the tag their body carries).
modded class RZ_Strangers
{
	//------------------------------------------------------------------------------------------------
	RZ_StrangerVisit DialogueVisit(int id)
	{
		foreach (RZ_StrangerVisit v : m_aVisits)
		{
			if (v.m_iId == id && !v.m_bDone)
				return v;
		}

		return null;
	}
}

//------------------------------------------------------------------------------------------------
//! A stranger: the generic person's replies (their choice slots), their call-out as the greeting.
class RZ_TalkStranger : RZ_TalkGeneric
{
	protected int m_iVisit;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkStranger(IEntity owner, int visit)
	{
		m_Owner = owner;
		m_iVisit = visit;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_StrangerVisit Visit(RZ_GameModeComponent gm)
	{
		RZ_Errands errands = gm.GetErrands();
		if (!errands || !errands.GetStrangers())
			return null;

		return errands.GetStrangers().DialogueVisit(m_iVisit);
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		s.m_sHeader = "A stranger";
		s.m_sName = "A stranger";
		s.m_sSub = "Nobody's, by the look of them";
		s.m_iFaction = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Their call-out, said again to whoever talks to them.
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		RZ_StrangerVisit v = Visit(gm);
		string line = "";
		if (v && v.m_Kind)
			line = v.Fill(v.m_Kind.CallOut(gm.GetErrands().GetStrangers(), v));

		if (line.IsEmpty())
			line = "\"Oh. You're real. Sorry. I've been talking to myself all day.\"";

		s.m_aNew.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	//! stranger:<routine> (roof, tin, funeral, which_way, door, two_in_back, digger, still_walking,
	//! warm_hands, seed_potatoes, manners, sister, wrong_side, waves_at_line, face_on_notice,
	//! hook_hiding, hook_first, hook_bitten), stranger. Memory: stranger:<visit>. Factionless.
	override bool Describe(RZ_GameModeComponent gm, RZ_TalkSession s, RZ_DCtx ctx)
	{
		RZ_StrangerVisit v = Visit(gm);
		if (!v || !v.m_Kind)
			return false;

		ctx.m_aKinds.Insert("stranger:" + v.m_Kind.m_sKey);
		ctx.m_aKinds.Insert("stranger");
		ctx.m_sKey = "stranger:" + v.m_iId.ToString();
		ctx.m_Stranger = v;
		ctx.m_iFaction = -1;
		ctx.m_sName = "A stranger";
		ctx.m_sShort = "the stranger";
		ctx.m_sRole = "stranger";
		ctx.m_bStronghold = true;	// nobody leans on a passer-by through talk: the routine's own ways do that
		if (v.m_iHouse >= 0)
			ctx.m_iHouse = v.m_iHouse;

		return true;
	}
}
