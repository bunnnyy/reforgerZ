//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Noisemaker (bottle, firecracker, alarm clock): adds heat where it lands or when its timer runs out. Add RZ_ArmNoisemakerAction for timed ones.")]
class RZ_NoisemakerComponentClass : ScriptComponentClass
{
}

//! Put this on an item prefab. Two ways to set it off (both can be on):
//!  - Thrown: the server sees it was carried, then lying more than a few metres
//!    from where the carrier stood. For that the item has to be throwable
//!    (e.g. based on a vanilla grenade without the explosion). Checked on the
//!    server tick, so the noise comes up to one tick after it lands.
//!  - Armed: RZ_ArmNoisemakerAction ("Wind the alarm clock", "Light the fuse")
//!    starts a timer, then it makes noise once or in pulses.
//! Each burst of noise is heat at its position (RZ_GameModeComponent.ReportNoise),
//! which pulls zombies toward it.
class RZ_NoisemakerComponent : ScriptComponent
{
	[Attribute("25", desc: "Heat per burst of noise")]
	protected float m_fHeat;

	[Attribute("0", desc: "Seconds from arming (or landing) to the first burst")]
	protected float m_fDelay;

	[Attribute("1", desc: "Number of bursts (an alarm clock rings for a while)")]
	protected int m_iPulses;

	[Attribute("8", desc: "Seconds between bursts")]
	protected float m_fPulseInterval;

	[Attribute("1", desc: "Goes off when thrown and it lands away from the thrower")]
	protected bool m_bOnThrow;

	[Attribute("1", desc: "Delete the item after the last burst (bottle smashed, firecracker spent)")]
	protected bool m_bDeleteAfter;

	[Attribute("Set it off", desc: "Label for RZ_ArmNoisemakerAction")]
	protected string m_sArmLabel;

	protected bool m_bArmed;
	protected int m_iPulsesLeft;
	protected bool m_bWasCarried;
	protected vector m_vCarrierPos;

	protected static ref array<RZ_NoisemakerComponent> s_aAll;
	protected static const float THROW_MIN_DISTANCE = 4;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!s_aAll)
			s_aAll = new array<RZ_NoisemakerComponent>();

		s_aAll.Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);

		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(Pulse);

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	string GetArmLabel()
	{
		return m_sArmLabel;
	}

	//------------------------------------------------------------------------------------------------
	bool IsArmed()
	{
		return m_bArmed;
	}

	//------------------------------------------------------------------------------------------------
	//! Server only. Starts the timer; does nothing if already armed.
	void Arm()
	{
		if (m_bArmed)
			return;

		m_bArmed = true;
		m_iPulsesLeft = Math.Max(1, m_iPulses);
		GetGame().GetCallqueue().CallLater(Pulse, Math.Max(100, m_fDelay * 1000), false);
	}

	//------------------------------------------------------------------------------------------------
	float GetDelay()
	{
		return m_fDelay;
	}

	//------------------------------------------------------------------------------------------------
	protected void Pulse()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		RZ_GameModeComponent.ReportNoise(owner.GetOrigin(), m_fHeat);
		m_iPulsesLeft--;
		if (m_iPulsesLeft > 0)
		{
			GetGame().GetCallqueue().CallLater(Pulse, Math.Max(1000, m_fPulseInterval * 1000), false);
			return;
		}

		m_bArmed = false;
		if (m_bDeleteAfter)
			SCR_EntityHelper.DeleteEntityAndChildren(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Server tick: was it carried last time and now lies away from the carrier?
	protected void PollThrown()
	{
		if (!m_bOnThrow || m_bArmed)
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		IEntity root = owner.GetRootParent(); // carried items are children of the character (or of the vehicle they sit in)
		if (root && root != owner)
		{
			m_bWasCarried = true;
			m_vCarrierPos = root.GetOrigin();
			return;
		}

		if (!m_bWasCarried)
			return;

		m_bWasCarried = false;
		if (RZ_Util.DistXZ(owner.GetOrigin(), m_vCarrierPos) >= THROW_MIN_DISTANCE)
			Arm();
	}

	//------------------------------------------------------------------------------------------------
	//! Called from the game mode's server tick.
	static void TickAll()
	{
		if (!s_aAll)
			return;

		for (int i = s_aAll.Count() - 1; i >= 0; i--)
		{
			RZ_NoisemakerComponent n = s_aAll[i];
			if (!n)
			{
				s_aAll.Remove(i);
				continue;
			}

			n.PollThrown();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! "Wind the alarm clock" / "Light the fuse": put it in the noisemaker's ActionsManagerComponent.
//! Used from the ground (drop it where you want the noise, then set it off).
class RZ_ArmNoisemakerAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Set it off";
		RZ_NoisemakerComponent comp = RZ_NoisemakerComponent.Cast(GetOwner().FindComponent(RZ_NoisemakerComponent));
		if (comp)
			outName = comp.GetArmLabel();

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

		RZ_NoisemakerComponent comp = RZ_NoisemakerComponent.Cast(pOwnerEntity.FindComponent(RZ_NoisemakerComponent));
		if (!comp || comp.IsArmed())
			return;

		comp.Arm();
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (gm && playerId > 0)
			gm.NotifyPlayer(playerId, "Noisemaker", string.Format("Set. You have about %1 seconds to get clear.", RZ_Util.RoundToInt(comp.GetDelay())));
	}
}
