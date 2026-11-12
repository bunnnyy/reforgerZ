//! A work site's prompt (Missions/Blocks/RZ_WorkSites.c; docs/UI.md, "The hold prompt"): one world
//! prompt on an invisible box at the site, held with vanilla's own hold-progress bar.
//!
//! The prefab's action has a Duration (10 s); the bar runs at Duration / the site's own time, so one
//! prefab serves every hold from 5 to 20 s. Hurt, or moved more than 1.5 m, and the bar starts again
//! (on your machine; the server keeps its own count, RZ_WorkSites.WatchWorkers). When the bar is full
//! the server checks the count and the work is done.

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "A work site's box (docs/DESIGN.md, Work site): what its hold prompt is (label, time, who may do it). Put it with RZ_WorkSiteAction (Duration 10) and an RplComponent on an invisible interaction box.")]
class RZ_WorkSiteComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class RZ_WorkSiteComponent : ScriptComponent
{
	[RplProp()]
	protected int m_iSite;

	[RplProp()]
	protected string m_sLabel;

	[RplProp()]
	protected float m_fSeconds;

	[RplProp()]
	protected float m_fFastSeconds;

	[RplProp()]
	protected string m_sFast;		// "|12|40|": the players with the faster time

	[RplProp()]
	protected int m_iOnly;			// only this player (0 anyone)

	//------------------------------------------------------------------------------------------------
	//! Server only.
	void Set(int site, string label, float seconds, float fastSeconds, string fast, int only)
	{
		m_iSite = site;
		m_sLabel = label;
		m_fSeconds = seconds;
		m_fFastSeconds = fastSeconds;
		m_sFast = fast;
		m_iOnly = only;
		Replication.BumpMe(); // needs an RplComponent on the prefab
	}

	int GetSite() { return m_iSite; }
	string GetLabel() { return m_sLabel; }

	//------------------------------------------------------------------------------------------------
	float SecondsFor(int playerId)
	{
		if (m_fFastSeconds > 0 && m_sFast.Contains("|" + playerId.ToString() + "|"))
			return m_fFastSeconds;

		return m_fSeconds;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFor(int playerId)
	{
		return m_iOnly <= 0 || playerId == m_iOnly;
	}
}

//------------------------------------------------------------------------------------------------
//! The hold prompt. Its label is the site's ("Dig the latrine").
class RZ_WorkSiteAction : ScriptedUserAction
{
	protected static const float DRIFT = 1.5;

	protected RZ_WorkSiteComponent m_Comp;
	protected float m_fStartHealth = -1;
	protected vector m_vStart;

	//------------------------------------------------------------------------------------------------
	protected RZ_WorkSiteComponent Comp()
	{
		if (!m_Comp && GetOwner())
			m_Comp = RZ_WorkSiteComponent.Cast(GetOwner().FindComponent(RZ_WorkSiteComponent));

		return m_Comp;
	}

	//------------------------------------------------------------------------------------------------
	protected static int PlayerOf(IEntity user)
	{
		if (!user)
			return 0;

		if (user == SCR_PlayerController.GetLocalControlledEntity())
			return SCR_PlayerController.GetLocalPlayerId();

		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		RZ_WorkSiteComponent c = Comp();
		if (!c || c.GetLabel().IsEmpty())
			return false;

		outName = c.GetLabel();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		RZ_WorkSiteComponent c = Comp();
		if (!c || c.GetSite() <= 0)
			return false;

		return c.IsFor(PlayerOf(user));
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The bar appeared (on the machine of the one holding it). VERIFY: OnActionStart runs on the
	//! holder's machine when a timed action's bar appears.
	override void OnActionStart(IEntity pUserEntity)
	{
		Anchor(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void Anchor(IEntity user)
	{
		if (!user)
			return;

		m_fStartHealth = RZ_Util.GetCharacterHealth(user);
		m_vStart = user.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	//! The bar fills at Duration / the site's time; hurt or moved, it starts again. VERIFY: a timed
	//! action's progress comes from GetActionProgressScript on the holder's machine, and the prefab's
	//! Duration (10) is what GetActionDuration returns.
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		RZ_WorkSiteComponent c = Comp();
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		if (!c || !me)
			return fProgress + timeSlice;

		if (fProgress <= 0 && m_fStartHealth < 0)
			Anchor(me);

		float health = RZ_Util.GetCharacterHealth(me);
		if ((m_fStartHealth >= 0 && health >= 0 && health < m_fStartHealth - 0.01) || RZ_Util.DistXZ(me.GetOrigin(), m_vStart) > DRIFT)
		{
			Anchor(me);
			return 0;
		}

		float duration = GetActionDuration();
		float want = c.SecondsFor(SCR_PlayerController.GetLocalPlayerId());
		float rate = 1;
		if (duration > 0 && want > 0)
			rate = duration / want;

		return fProgress + timeSlice * rate;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_fStartHealth = -1;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_fStartHealth = -1;
		if (!Replication.IsServer())
			return;

		RZ_WorkSiteComponent c = Comp();
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (c && sites)
			sites.TryComplete(c.GetSite(), pUserEntity);
	}
}
