//! The behaviour layer's hooks into Bacon Zombies 1.8.0 (docs/INFECTED.md, "How they move and hunt").
//! Every Bacon-specific line of the layer is in this file. Each hook only reads fields our code set
//! on the character (RZ_BaconBody); with none set (mode OFF), Bacon behaves exactly as before. Lab
//! tests B2, B3, B8, B18 (tools/zombie_lab, RZL_Hooks.c is where these started).

//------------------------------------------------------------------------------------------------
//! The fields our code sets on a Bacon character, and the stamps the hooks leave.
modded class Bacon_622120A5448725E3_InfectedCharacter
{
	int m_iRZMode;				// RZ_EBodyMode: 0 OFF (Bacon's), 1 STEER, 2 ENGAGE, 3 RECOVER
	bool m_bRZSlot;				// L2: Roam walks to m_vRZSlot
	vector m_vRZSlot;
	float m_fRZStance;			// L6: > 0 is our dynamic stance
	int m_iRZClimb;				// L11: 0 Bacon's, 2 up to m_fRZClimbLimit (grab height), 3 never
	float m_fRZClimbLimit;
	float m_fRZClimbRefusedAt;	// world seconds of the last refused climb (a stuck tick)
	float m_fRZMeleeAt;			// world seconds Bacon's melee test last passed (hit attribution)
	bool m_bRZMute;				// L15

	//------------------------------------------------------------------------------------------------
	//! L2: our slot, when our code set one; Roam walks there with or without a group waypoint.
	override vector ComputeLoiterPointInRadius(float maxRadius = 25)
	{
		if (m_bRZSlot)
			return m_vRZSlot;

		return super.ComputeLoiterPointInRadius(maxRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! L6: our dynamic stance (B7: 0.6 and 0.7 read back exactly).
	override void UpdateStance()
	{
		if (m_fRZStance > 0)
		{
			GetCharacterController().SetDynamicStance(m_fRZStance);
			return;
		}

		super.UpdateStance();
	}

	//------------------------------------------------------------------------------------------------
	//! L9: the share gate. Steered bodies never share; an engaged one only shares a target it saw
	//! itself within 2 s, which ends the real-position chain (B2, B5).
	override void AIBroadcastTarget(IEntity target)
	{
		if (m_iRZMode == 1 || m_iRZMode == 3)
			return;

		if (m_iRZMode == 2 && !RZ_BaconBody.SawRecently(this, target, 2))
			return;

		super.AIBroadcastTarget(target);
	}
}

//------------------------------------------------------------------------------------------------
//! L8: the target intake gate. A body our code drives ignores gunfire, shared targets and doors:
//! our own senses hear for it (B2, B18).
modded class Bacon_622120A5448725E3_AIGetShooter
{
	//------------------------------------------------------------------------------------------------
	override protected bool ProcessEvent(AIDangerEvent dangerEvent, out IEntity target)
	{
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(m_Owner);
		if (ch && (ch.m_iRZMode == 1 || ch.m_iRZMode == 3))
			return false;

		return super.ProcessEvent(dangerEvent, target);
	}
}

//------------------------------------------------------------------------------------------------
//! L11: the climb gate. Runs the engine's climb test and refuses a grab point above the kind's
//! limit (B8: limits are grab heights; a refused body stops dead, so a refusal is a stuck tick).
modded class Bacon_622120A5448725E3_AIRequestClimb
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Character || m_Character.m_iRZClimb == 0 || m_Character.IsInVehicle() || !m_Character.GetPhysics())
			return super.EOnTaskSimulate(owner, dt);

		if (m_Character.GetPhysics().GetVelocity().Length() != 0)
		{
			SetVariableOut("CanClimb", false);
			return ENodeResult.FAIL;
		}

		CharacterCommandClimbResult res = new CharacterCommandClimbResult();
		CharacterCommandClimb.DoClimbTest(m_Character, m_ClimbSettings, res);
		if (!res.m_bIsClimbOver && !res.m_bIsClimb)
		{
			SetVariableOut("CanClimb", false);
			return ENodeResult.FAIL;
		}

		float grab = RZ_BaconBody.GrabHeight(m_Character, res);
		if (m_Character.m_iRZClimb == 3 || grab > m_Character.m_fRZClimbLimit)
		{
			m_Character.m_fRZClimbRefusedAt = RZ_BaconBody.Now();
			SetVariableOut("CanClimb", false);
			return ENodeResult.FAIL;
		}

		return super.EOnTaskSimulate(owner, dt);
	}
}

//------------------------------------------------------------------------------------------------
//! L12, read only: stamps when Bacon's melee test passed, so a player's health drop can be put on
//! the body that swung (B14: IsMeleeAttack() never reads true on the server).
modded class Bacon_622120A5448725E3_AICanMeleeTarget
{
	//------------------------------------------------------------------------------------------------
	override bool TestFunction(AIAgent owner)
	{
		bool ok = super.TestFunction(owner);
		if (!ok)
			return false;

		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(owner.GetControlledEntity());
		if (ch)
			ch.m_fRZMeleeAt = RZ_BaconBody.Now();

		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! L15: a muted body says nothing on its own (stalkers; B3: muted bodies played 0 sounds).
modded class Bacon_622120A5448725E3_AIInfectedTaskSpeak
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(owner.GetControlledEntity());
		if (ch && ch.m_bRZMute)
			return ENodeResult.SUCCESS;

		return super.EOnTaskSimulate(owner, dt);
	}
}

//------------------------------------------------------------------------------------------------
//! Our side of the hooks: setting and reading the fields on one body. Every other file of the
//! layer goes through here, so nothing else names a Bacon class.
class RZ_BaconBody
{
	//------------------------------------------------------------------------------------------------
	static float Now()
	{
		return GetGame().GetWorld().GetWorldTime() * 0.001;
	}

	//------------------------------------------------------------------------------------------------
	//! A Bacon body our code can drive (false for any other zombie mod's characters).
	static bool IsBacon(IEntity e)
	{
		return Bacon_622120A5448725E3_InfectedCharacter.Cast(e) != null;
	}

	//------------------------------------------------------------------------------------------------
	static void SetMode(IEntity e, int mode)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (inf)
			inf.m_iRZMode = mode;
	}

	//------------------------------------------------------------------------------------------------
	//! The body's mode (0 OFF for anything that isn't a Bacon body).
	static int GetMode(IEntity e)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (!inf)
			return 0;

		return inf.m_iRZMode;
	}

	//------------------------------------------------------------------------------------------------
	static void SetSlot(IEntity e, vector slot)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (!inf)
			return;

		inf.m_bRZSlot = true;
		inf.m_vRZSlot = slot;
	}

	//------------------------------------------------------------------------------------------------
	static void ClearSlot(IEntity e)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (inf)
			inf.m_bRZSlot = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Puts Bacon's own loiter point somewhere (a take-back: Roam stops running to where a lost
	//! target really was, B5).
	static void SetLoiter(IEntity e, vector pos)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (inf)
			inf.SetLoiterPoint(pos);
	}

	//------------------------------------------------------------------------------------------------
	static void SetStance(IEntity e, float stance)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (inf)
			inf.m_fRZStance = stance;
	}

	//------------------------------------------------------------------------------------------------
	//! limit < 0: never climbs; otherwise up to that grab height (m).
	static void SetClimb(IEntity e, float limit)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (!inf)
			return;

		if (limit < 0)
		{
			inf.m_iRZClimb = 3;
			return;
		}

		inf.m_iRZClimb = 2;
		inf.m_fRZClimbLimit = limit;
	}

	//------------------------------------------------------------------------------------------------
	static float ClimbRefusedAt(IEntity e)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (!inf)
			return -1;

		return inf.m_fRZClimbRefusedAt;
	}

	//------------------------------------------------------------------------------------------------
	static float MeleeAt(IEntity e)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (!inf)
			return -1;

		return inf.m_fRZMeleeAt;
	}

	//------------------------------------------------------------------------------------------------
	static void SetMute(IEntity e, bool mute)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (inf)
			inf.m_bRZMute = mute;
	}

	//------------------------------------------------------------------------------------------------
	//! A sound from the body's own set: "SOUND_IDLE" or "SOUND_TARGET" (a tell before a strike).
	static void Say(IEntity e, string sound)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (inf)
			inf.PlaySoundServer(sound);
	}

	//------------------------------------------------------------------------------------------------
	//! Has this observer itself seen the target within `seconds`?
	static bool SawRecently(IEntity observer, IEntity target, float seconds)
	{
		if (!observer || !target)
			return false;

		PerceptionComponent perc = PerceptionComponent.Cast(observer.FindComponent(PerceptionComponent));
		if (!perc)
			return false;

		BaseTarget bt = perc.FindTargetPerceptionObject(target);
		if (!bt)
			return false;

		float since = bt.GetTimeSinceSeen();
		return since >= 0 && since < seconds;
	}

	//------------------------------------------------------------------------------------------------
	//! The climb's grab point, in metres above the character's feet.
	static float GrabHeight(IEntity character, CharacterCommandClimbResult res)
	{
		vector grab = res.m_ClimbGrabPointLS;
		if (res.m_ClimbParent)
			grab = res.m_ClimbParent.CoordToParent(res.m_ClimbGrabPointLS);

		return grab[1] - character.GetOrigin()[1];
	}
}
