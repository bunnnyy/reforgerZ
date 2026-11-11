//! Radio speakers (docs/DIALOGUE.md, "The open channel"): conversations with nobody's body. A radio
//! item with RZ_RadioCallAction offers "Call on the open channel"; it opens the talk screen on the
//! open channel (`for radio:open`), with Dialogue > Trees on. You stay within talking range of the
//! radio. A `public` node on air is heard by everyone within the channel's reach, as one line with
//! the tags stripped and no name: nothing personal goes on air. Nobody is threatened on air.

//------------------------------------------------------------------------------------------------
//! "Call on the open channel": on a radio item's ActionsManagerComponent (the wind-up radio, any
//! radio you want to call from). Used from the ground, like the scrambled channel's action.
class RZ_RadioCallAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Call on the open channel";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Shown only with the talk screen and Trees on (both read on every machine).
	override bool CanBeShownScript(IEntity user)
	{
		return RZ_Dialogue.IsOn() && RZ_DialogueSettings.Get().m_bTrees;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0 || !gm.GetDialogue())
			return;

		gm.GetDialogue().OpenRadio(playerId, pOwnerEntity, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
//! The open channel as a speaker: no body, no faction, no verbs. Its memory key is the channel.
class RZ_TalkRadio : RZ_TalkSpeaker
{
	static const float REACH = 1500;	// a public node on air is heard this far from the caller

	protected IEntity m_Radio;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkRadio(IEntity radio)
	{
		m_Radio = radio;
	}

	//------------------------------------------------------------------------------------------------
	//! Anything that isn't a character is talked to as a radio.
	static bool IsRadio(IEntity e)
	{
		return e && !ChimeraCharacter.Cast(e);
	}

	//------------------------------------------------------------------------------------------------
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		s.m_sHeader = "The open channel";
		s.m_sName = "The open channel";
		s.m_sSub = "Radio - whoever is listening";
		s.m_iFaction = -1;
	}

	//------------------------------------------------------------------------------------------------
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		s.m_aNew.Insert("Static, then a click. Somebody is listening, or nobody is.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Describe(RZ_GameModeComponent gm, RZ_TalkSession s, RZ_DCtx ctx)
	{
		ctx.m_aKinds.Insert("radio:open");
		ctx.m_aKinds.Insert("radio");
		ctx.m_sKey = "radio:open";
		ctx.m_iFaction = -1;
		ctx.m_sName = "The open channel";
		ctx.m_sShort = "the channel";
		ctx.m_sRole = "radio";
		ctx.m_bRadio = true;
		ctx.m_bStronghold = true;	// nobody is threatened on air
		return true;
	}
}
