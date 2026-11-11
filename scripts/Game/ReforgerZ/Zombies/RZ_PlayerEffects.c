[ComponentEditorProps(category: "ReforgerZ", description: "ReforgerZ attack effects: plays the infected's close moves on the owning client (a hold you kick free of with Jump, a stagger, a knockdown). The server decides each one (Zombies/Behaviour/RZ_AttackEffects.c). Add to the player controller prefab, next to RZ_PlayerUIComponent (docs/INFECTED.md, Attacks).")]
class RZ_PlayerEffectsComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! The key our cap on the player's own speed is stored under (SCR_ChimeraCharacter.SetSpeedLimit
//! keeps one cap per source).
class RZ_PlayerEffectKey
{
}

//------------------------------------------------------------------------------------------------
//! The infected's close moves on the player's own machine (docs/INFECTED.md, "Attacks").
//!
//! Server to client: owner-only RPCs (a hold, a stagger, a knockdown, or clear). A player's movement
//! is predicted on their own machine, so a cap on their speed, a forced stance or movement off must
//! be set there. Every effect ends on a timer here, and is cleared when the character dies or
//! changes (a respawn). Client to server: one RPC, "kicked free", which the server checks against
//! the hold it gave. Breaking a hold is the vanilla Jump action (Space; the pad's jump button), so
//! it works on a pad and a console with no new binding; the prompt is the vanilla hint, with its
//! timer. On a listen server the owner is this machine, so calls go straight to the handler.
class RZ_PlayerEffectsComponent : ScriptComponent
{
	protected ref RZ_PlayerEffectKey m_Key = new RZ_PlayerEffectKey();
	protected IEntity m_Char;				// weak: the character the effects are on
	protected float m_fHoldUntil = -1;
	protected float m_fHoldCap = 1;
	protected int m_iKicksNeeded;
	protected int m_iKicks;
	protected float m_fStaggerUntil = -1;
	protected float m_fStaggerCap = 1;
	protected float m_fDownUntil = -1;
	protected float m_fCapSet = 1;
	protected bool m_bMovementOff;
	protected bool m_bListening;
	protected bool m_bTicking;
	protected bool m_bHint;

	protected static const int TICK_MS = 100;

	//------------------------------------------------------------------------------------------------
	//! A player's component (server), or null.
	static RZ_PlayerEffectsComponent GetFor(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return null;

		return RZ_PlayerEffectsComponent.Cast(pc.FindComponent(RZ_PlayerEffectsComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsLocalOwner()
	{
		return GetOwner() == GetGame().GetPlayerController();
	}

	//------------------------------------------------------------------------------------------------
	protected int OwnerPlayerId()
	{
		PlayerController pc = PlayerController.Cast(GetOwner());
		if (!pc)
			return -1;

		return pc.GetPlayerId();
	}

	//------------------------------------------------------------------------------------------------
	protected static float Now()
	{
		return GetGame().GetWorld().GetWorldTime() * 0.001;
	}

	// ================================================================ Server: send
	//------------------------------------------------------------------------------------------------
	//! Held: the speed capped at fraction for up to seconds, until kicks presses of Jump.
	void SendHold(float seconds, float fraction, int kicks)
	{
		if (IsLocalOwner())
			RpcDo_Hold(seconds, fraction, kicks);
		else
			Rpc(RpcDo_Hold, seconds, fraction, kicks);
	}

	//------------------------------------------------------------------------------------------------
	void SendStagger(float seconds, float fraction)
	{
		if (IsLocalOwner())
			RpcDo_Stagger(seconds, fraction);
		else
			Rpc(RpcDo_Stagger, seconds, fraction);
	}

	//------------------------------------------------------------------------------------------------
	void SendKnockdown(float seconds)
	{
		if (IsLocalOwner())
			RpcDo_Knockdown(seconds);
		else
			Rpc(RpcDo_Knockdown, seconds);
	}

	//------------------------------------------------------------------------------------------------
	//! The hold ended on the server (the one holding died or was left behind).
	void SendClear()
	{
		if (IsLocalOwner())
			RpcDo_Clear();
		else
			Rpc(RpcDo_Clear);
	}

	// ================================================================ Server: receive
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_KickedFree()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetBehaviour())
			return;

		gm.GetBehaviour().GetEffects().OnKickedFree(OwnerPlayerId());
	}

	// ================================================================ Client: receive
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Hold(float seconds, float fraction, int kicks)
	{
		if (!Bind())
			return;

		m_fHoldUntil = Now() + seconds;
		m_fHoldCap = fraction;
		m_iKicksNeeded = Math.Max(1, kicks);
		m_iKicks = 0;
		if (!m_bListening)
		{
			GetGame().GetInputManager().AddActionListener("CharacterJump", EActionTrigger.DOWN, OnKick);
			m_bListening = true;
		}

		ShowHoldHint(seconds);
		Apply();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Stagger(float seconds, float fraction)
	{
		if (!Bind())
			return;

		m_fStaggerUntil = Now() + seconds;
		m_fStaggerCap = fraction;
		Apply();
	}

	//------------------------------------------------------------------------------------------------
	//! Knocked down: forced prone (the stance's own animation) with movement off for a moment.
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Knockdown(float seconds)
	{
		if (!Bind())
			return;

		CharacterControllerComponent ctrl = Controller();
		if (!ctrl)
			return;

		ctrl.SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOPRONE); // VERIFY: a stance change set on the owner's own character drops it prone with the vanilla animation and replicates
		ctrl.SetDisableMovementControls(true); // VERIFY: movement controls off on the owner's character for a moment (vanilla does this for item placement); back on below
		m_bMovementOff = true;
		m_fDownUntil = Now() + seconds;
		Apply();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Clear()
	{
		EndHold();
		Apply();
	}

	// ================================================================ Client: the effects
	//------------------------------------------------------------------------------------------------
	//! The character the effects go on: this machine's controlled, living character. A new one (a
	//! respawn) clears everything left on the last.
	protected bool Bind()
	{
		IEntity ent = GetGame().GetPlayerController().GetControlledEntity();
		if (!ent || !RZ_Util.IsCharacterAlive(ent) || RZ_Util.IsInVehicle(ent))
			return false;

		if (ent != m_Char)
		{
			ClearAll();
			m_Char = ent;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected CharacterControllerComponent Controller()
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(m_Char);
		if (!c)
			return null;

		return c.GetCharacterController();
	}

	//------------------------------------------------------------------------------------------------
	//! The cap from every effect running (the lowest), the knockdown's end, and the ticker.
	protected void Apply()
	{
		float now = Now();
		float cap = 1;
		if (m_fHoldUntil > now)
			cap = Math.Min(cap, m_fHoldCap);

		if (m_fStaggerUntil > now)
			cap = Math.Min(cap, m_fStaggerCap);

		SetCap(cap);
		if (m_bMovementOff && now >= m_fDownUntil)
		{
			CharacterControllerComponent ctrl = Controller();
			if (ctrl)
				ctrl.SetDisableMovementControls(false);

			m_bMovementOff = false;
		}

		bool running = m_fHoldUntil > now || m_fStaggerUntil > now || m_bMovementOff;
		if (running && !m_bTicking)
		{
			m_bTicking = true;
			GetGame().GetCallqueue().CallLater(Update, TICK_MS, true);
		}
		else if (!running && m_bTicking)
		{
			m_bTicking = false;
			GetGame().GetCallqueue().Remove(Update);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCap(float cap)
	{
		if (Math.AbsFloat(cap - m_fCapSet) < 0.01)
			return;

		m_fCapSet = cap;
		SCR_ChimeraCharacter c = SCR_ChimeraCharacter.Cast(m_Char);
		if (c)
			c.SetSpeedLimit(m_Key, cap, true); // VERIFY: a speed cap set on the owner's own character slows the player (not slow motion) and needs no server copy
	}

	//------------------------------------------------------------------------------------------------
	//! Ten times a second while an effect runs: timers, and the character still alive and the same.
	protected void Update()
	{
		IEntity ent;
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
			ent = pc.GetControlledEntity();

		if (!m_Char || ent != m_Char || !RZ_Util.IsCharacterAlive(m_Char))
		{
			ClearAll();
			return;
		}

		if (m_fHoldUntil >= 0 && Now() >= m_fHoldUntil)
			EndHold();

		Apply();
	}

	//------------------------------------------------------------------------------------------------
	//! Jump while held: enough presses and you're free (the server is told).
	protected void OnKick(float value = 0, EActionTrigger reason = 0)
	{
		if (m_fHoldUntil < 0 || Now() >= m_fHoldUntil)
			return;

		m_iKicks++;
		if (m_iKicks < m_iKicksNeeded)
			return;

		EndHold();
		Apply();
		if (Replication.IsServer())
			RpcAsk_KickedFree();
		else
			Rpc(RpcAsk_KickedFree);
	}

	//------------------------------------------------------------------------------------------------
	protected void EndHold()
	{
		m_fHoldUntil = -1;
		m_iKicks = 0;
		if (m_bListening)
		{
			GetGame().GetInputManager().RemoveActionListener("CharacterJump", EActionTrigger.DOWN, OnKick);
			m_bListening = false;
		}

		if (m_bHint)
		{
			SCR_HintManagerComponent.HideHint();
			m_bHint = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The hold's prompt: the vanilla hint, with its timer (it shows on a pad and a TV as any hint does).
	protected void ShowHoldHint(float seconds)
	{
		string text = string.Format("Kick free: press Jump %1 times.", m_iKicksNeeded);
		if (m_iKicksNeeded == 1)
			text = "Kick free: press Jump.";

		m_bHint = SCR_HintManagerComponent.ShowCustomHint(text, "Something has your ankle", seconds, false, EFieldManualEntryId.NONE, true); // VERIFY: a custom hint with its timer shows over the HUD in play
	}

	//------------------------------------------------------------------------------------------------
	//! Everything off: the cap, movement back on, the listener and the hint.
	protected void ClearAll()
	{
		EndHold();
		m_fStaggerUntil = -1;
		m_fDownUntil = -1;
		if (m_bMovementOff)
		{
			CharacterControllerComponent ctrl = Controller();
			if (ctrl)
				ctrl.SetDisableMovementControls(false);

			m_bMovementOff = false;
		}

		SetCap(1);
		m_Char = null;
		m_fCapSet = 1;
		if (m_bTicking)
		{
			m_bTicking = false;
			GetGame().GetCallqueue().Remove(Update);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_PlayerEffectsComponent()
	{
		if (m_bListening && GetGame() && GetGame().GetInputManager())
			GetGame().GetInputManager().RemoveActionListener("CharacterJump", EActionTrigger.DOWN, OnKick);

		if (m_bTicking && GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(Update);
	}
}
