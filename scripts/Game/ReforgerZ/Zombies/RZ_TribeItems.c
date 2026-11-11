//! The packs' items and the dead run's opt-out (docs/INFECTED.md, "People"; docs/WORKBENCH_SETUP.md):
//!   RZ_TagAction        on Halcyon's tag item: Tag it (the leader within 8 m, unnoticed; hold 5 s)
//!   RZ_CallBoxAction    on the call box item: Play the recorded call (90 s, as a data noise)
//!   RZ_DeadNameAction   on the notebook item: keep your name out of public text if you die out
//!                       there, or let it be said (the pad's way to the opt-out; /mydead in chat)
//! Each runs on the server.

//------------------------------------------------------------------------------------------------
class RZ_TribeItemHelp
{
	//------------------------------------------------------------------------------------------------
	static RZ_TribePeople People()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetTribes())
			return null;

		return gm.GetTribes().GetPeople();
	}

	//------------------------------------------------------------------------------------------------
	//! The item leaves the world once used.
	static void UseUp(IEntity item)
	{
		if (item)
			RZ_Util.DeleteUnit(item);
	}
}

//------------------------------------------------------------------------------------------------
//! Halcyon's tag: Tag it. A pack's leader, in its prime or older, within 8 m, by day or in its
//! sleep after a feed, while the pack hasn't noticed you (no hunt after you). Set the action's
//! duration to 5 s in the prefab: hold it, then get away. Plant a Tag pays; anyone may use a tag.
class RZ_TagAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Tag it";
		return true;
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
		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		RZ_TribePeople people = RZ_TribeItemHelp.People();
		if (!gm || playerId <= 0 || !people || !people.IsOn())
			return;

		RZ_Tribes tribes = gm.GetTribes();
		vector at = pUserEntity.GetOrigin();
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			IEntity body = t.AlphaBody();
			if (!body || RZ_Util.DistXZ(body.GetOrigin(), at) > 8)
				continue;

			if (t.Stage() < 1)
			{
				gm.NotifyPlayer(playerId, "Tag it", "Too young a lead to be worth a tag. Halcyon want one that's been at it a while.");
				return;
			}

			bool day = tribes.GetPhase() == RZ_ETribePhase.DAY;
			bool noticed = t.m_Hunt && t.m_Hunt.m_iTarget == playerId;
			if ((!day && !t.m_bFed) || noticed)
			{
				gm.NotifyPlayer(playerId, "Tag it", "It's awake and it's seen you. Not now.");
				return;
			}

			people.Tag(t, playerId);
			RZ_TribeItemHelp.UseUp(pOwnerEntity);
			gm.NotifyPlayer(playerId, "Tag it", string.Format("The tag's in, behind the ear. Get away from %1 before it shakes it off its sleep.", t.m_sPlace));
			RZ_TribeContract.OnTagged(gm, t, playerId);
			return;
		}

		gm.NotifyPlayer(playerId, "Tag it", "Nothing here to tag. It has to be the big one, close enough to touch.");
	}
}

//------------------------------------------------------------------------------------------------
//! The call box: Play the recorded call. Halcyon lends it from 50% (RZ_TribePeople.GiveCallBox).
class RZ_CallBoxAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Play the recorded call";
		return true;
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

		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		RZ_TribePeople people = RZ_TribeItemHelp.People();
		if (playerId <= 0 || !people || !people.IsOn())
			return;

		if (people.PlayCallBox(playerId, pOwnerEntity.GetOrigin()))
			RZ_TribeItemHelp.UseUp(pOwnerEntity);
	}
}

//------------------------------------------------------------------------------------------------
//! On the notebook item: "Your dead: keep my name out of it" / "let them say it" (the opt-out,
//! reachable with a pad; on PC also /mydead hidden or /mydead shown).
class RZ_DeadNameAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "If I die: say my name, or don't";
		return true;
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

		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		RZ_TribePeople people = RZ_TribeItemHelp.People();
		if (playerId > 0 && people)
			people.SetHidden(playerId, -1);
	}
}
