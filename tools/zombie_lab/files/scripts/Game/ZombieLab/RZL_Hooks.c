// Test-only (not part of the mod): the infected behaviour addendum's hooks into Bacon Zombies 1.8.0
// (/root/rz_tools/behaviour_addendum.md, section 2), as the zombie lab's B tests use them. Each hook
// counts its calls and only reads fields the lab set on the character; with none set, Bacon behaves
// exactly as before (the phase 0 experiments see no change).

//------------------------------------------------------------------------------------------------
//! Call counters for B2, and the clock the hooks stamp.
class RZL_Hooks
{
	static int s_iLoiter;			// InfectedCharacter.ComputeLoiterPointInRadius (L2)
	static int s_iLoiterSlot;		// ... answered with our slot
	static int s_iStance;			// InfectedCharacter.UpdateStance (L6)
	static int s_iStanceOurs;
	static int s_iBroadcast;		// InfectedCharacter.AIBroadcastTarget (L9)
	static int s_iBroadcastBlocked;
	static int s_iShooter;			// AIGetShooter.EOnTaskSimulate
	static int s_iEvents;			// AIGetShooter.ProcessEvent (L8)
	static int s_iEventsBlocked;
	static int s_iClimb;			// AIRequestClimb.EOnTaskSimulate (L11)
	static int s_iClimbTests;
	static int s_iClimbRefused;
	static int s_iMelee;			// AICanMeleeTarget.TestFunction (L12)
	static int s_iMeleeHeld;
	static int s_iLodWait;			// GetAILODWait (L13)
	static int s_iLodWaitOurs;
	static int s_iSpeak;			// AIInfectedTaskSpeak (L15)
	static int s_iSpeakMuted;
	static int s_iSounds;			// InfectedCharacter.PlaySoundServer
	static int s_iUpdateLoiter;		// AIUpdateLoiterPoint
	static int s_iMoveParams;		// SCR_CreateAIExtraMoveParams on a Bacon body (L23)
	static int s_iMoveParamsOurs;
	static int s_iHeavyContacts;	// InfectedCharacterHeavy.EOnContact with a destructible

	//------------------------------------------------------------------------------------------------
	static float Now()
	{
		return GetGame().GetWorld().GetWorldTime() * 0.001;
	}

	//------------------------------------------------------------------------------------------------
	static string Line()
	{
		string a = string.Format("loiter=%1|loiterSlot=%2|stance=%3|stanceOurs=%4|broadcast=%5|broadcastBlocked=%6|shooter=%7|events=%8|eventsBlocked=%9", s_iLoiter, s_iLoiterSlot, s_iStance, s_iStanceOurs, s_iBroadcast, s_iBroadcastBlocked, s_iShooter, s_iEvents, s_iEventsBlocked);
		string b = string.Format("|climb=%1|climbTests=%2|climbRefused=%3|melee=%4|meleeHeld=%5|lodWait=%6|lodWaitOurs=%7|speak=%8|speakMuted=%9", s_iClimb, s_iClimbTests, s_iClimbRefused, s_iMelee, s_iMeleeHeld, s_iLodWait, s_iLodWaitOurs, s_iSpeak, s_iSpeakMuted);
		string c = string.Format("|sounds=%1|updateLoiter=%2|moveParams=%3|moveParamsOurs=%4|heavyContacts=%5", s_iSounds, s_iUpdateLoiter, s_iMoveParams, s_iMoveParamsOurs, s_iHeavyContacts);
		return a + b + c;
	}

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_iLoiter = 0;
		s_iLoiterSlot = 0;
		s_iStance = 0;
		s_iStanceOurs = 0;
		s_iBroadcast = 0;
		s_iBroadcastBlocked = 0;
		s_iShooter = 0;
		s_iEvents = 0;
		s_iEventsBlocked = 0;
		s_iClimb = 0;
		s_iClimbTests = 0;
		s_iClimbRefused = 0;
		s_iMelee = 0;
		s_iMeleeHeld = 0;
		s_iLodWait = 0;
		s_iLodWaitOurs = 0;
		s_iSpeak = 0;
		s_iSpeakMuted = 0;
		s_iSounds = 0;
		s_iUpdateLoiter = 0;
		s_iMoveParams = 0;
		s_iMoveParamsOurs = 0;
		s_iHeavyContacts = 0;
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

//------------------------------------------------------------------------------------------------
//! The fields our code sets (the addendum's "fields on the character") and what the hooks saw.
modded class Bacon_622120A5448725E3_InfectedCharacter
{
	int m_iRZLMode;				// 0 off (Bacon's), 1 steer (intake and sharing closed), 2 engage (share gate)
	bool m_bRZLSlot;			// L2: Roam walks to m_vRZLSlot
	vector m_vRZLSlot;
	float m_fRZLStance;			// L6: > 0 is our dynamic stance
	int m_iRZLClimb;			// L11: 0 Bacon's, 1 Bacon's but logged, 2 limit m_fRZLClimbLimit, 3 never
	float m_fRZLClimbLimit;
	bool m_bRZLMeleeHold;		// L12
	float m_fRZLWait;			// L13: > 0 is our think-loop wait (s)
	bool m_bRZLMute;			// L15
	float m_fRZLPathCost;		// L23: > 0 is our straight-path cost in a chase

	int m_iRZLLoiterN;
	float m_fRZLLoiterAt;
	float m_fRZLBroadcastAt;
	IEntity m_RZLBroadcastTarget;
	float m_fRZLShooterAt;
	float m_fRZLShooterOkAt;
	float m_fRZLUpdLoiterAt;
	int m_iRZLEventsBlocked;
	int m_iRZLLoops;
	int m_iRZLSounds;
	float m_fRZLClimbAt;
	float m_fRZLClimbGrab;
	int m_iRZLClimbOk;
	int m_iRZLClimbRefused;
	int m_iRZLCanMelee;			// AICanMeleeTarget said yes (Bacon then requests its melee action)

	//------------------------------------------------------------------------------------------------
	override vector ComputeLoiterPointInRadius(float maxRadius = 25)
	{
		RZL_Hooks.s_iLoiter++;
		m_iRZLLoiterN++;
		m_fRZLLoiterAt = RZL_Hooks.Now();
		if (m_bRZLSlot)
		{
			RZL_Hooks.s_iLoiterSlot++;
			return m_vRZLSlot;
		}

		return super.ComputeLoiterPointInRadius(maxRadius);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateStance()
	{
		RZL_Hooks.s_iStance++;
		if (m_fRZLStance > 0)
		{
			RZL_Hooks.s_iStanceOurs++;
			GetCharacterController().SetDynamicStance(m_fRZLStance);
			return;
		}

		super.UpdateStance();
	}

	//------------------------------------------------------------------------------------------------
	override void AIBroadcastTarget(IEntity target)
	{
		RZL_Hooks.s_iBroadcast++;
		m_fRZLBroadcastAt = RZL_Hooks.Now();
		m_RZLBroadcastTarget = target;
		if (m_iRZLMode == 1 || (m_iRZLMode == 2 && !RZL_Hooks.SawRecently(this, target, 2)))
		{
			RZL_Hooks.s_iBroadcastBlocked++;
			return;
		}

		super.AIBroadcastTarget(target);
	}

	//------------------------------------------------------------------------------------------------
	override void PlaySoundServer(string soundEventName)
	{
		RZL_Hooks.s_iSounds++;
		m_iRZLSounds++;
		super.PlaySoundServer(soundEventName);
	}
}

//------------------------------------------------------------------------------------------------
modded class Bacon_622120A5448725E3_InfectedCharacterHeavy
{
	//------------------------------------------------------------------------------------------------
	bool RZL_DestroysThings()
	{
		return m_bDestroysThings;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (SCR_DestructibleEntity.Cast(other))
			RZL_Hooks.s_iHeavyContacts++;
		super.EOnContact(owner, other, contact);
	}
}

//------------------------------------------------------------------------------------------------
//! L8: the target intake gate. A steered body ignores gunfire, shared targets and doors.
modded class Bacon_622120A5448725E3_AIGetShooter
{
	//------------------------------------------------------------------------------------------------
	override protected bool ProcessEvent(AIDangerEvent dangerEvent, out IEntity target)
	{
		RZL_Hooks.s_iEvents++;
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(m_Owner);
		if (ch && ch.m_iRZLMode == 1)
		{
			RZL_Hooks.s_iEventsBlocked++;
			ch.m_iRZLEventsBlocked++;
			return false;
		}

		return super.ProcessEvent(dangerEvent, target);
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		RZL_Hooks.s_iShooter++;
		ENodeResult r = super.EOnTaskSimulate(owner, dt);
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(m_Owner);
		if (ch)
		{
			ch.m_fRZLShooterAt = RZL_Hooks.Now();
			if (r == ENodeResult.SUCCESS)
				ch.m_fRZLShooterOkAt = ch.m_fRZLShooterAt;
		}
		return r;
	}
}

//------------------------------------------------------------------------------------------------
//! L11: the climb gate. Runs the same climb test and refuses a grab point above the limit.
modded class Bacon_622120A5448725E3_AIRequestClimb
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		RZL_Hooks.s_iClimb++;
		if (!m_Character || m_Character.m_iRZLClimb == 0 || m_Character.IsInVehicle() || !m_Character.GetPhysics())
			return super.EOnTaskSimulate(owner, dt);

		int mode = m_Character.m_iRZLClimb;
		if (m_Character.GetPhysics().GetVelocity().Length() != 0)
		{
			if (mode == 1)
				return super.EOnTaskSimulate(owner, dt);

			SetVariableOut("CanClimb", false);
			return ENodeResult.FAIL;
		}

		CharacterCommandClimbResult res = new CharacterCommandClimbResult();
		CharacterCommandClimb.DoClimbTest(m_Character, m_ClimbSettings, res);
		RZL_Hooks.s_iClimbTests++;
		if (!res.m_bIsClimbOver && !res.m_bIsClimb)
		{
			if (mode == 1)
				return super.EOnTaskSimulate(owner, dt);

			SetVariableOut("CanClimb", false);
			return ENodeResult.FAIL;
		}

		float grab = RZL_Hooks.GrabHeight(m_Character, res);
		m_Character.m_fRZLClimbGrab = grab;
		m_Character.m_fRZLClimbAt = RZL_Hooks.Now();
		if (mode == 1)
		{
			m_Character.m_iRZLClimbOk++;
			return super.EOnTaskSimulate(owner, dt);
		}

		if (mode == 3 || grab > m_Character.m_fRZLClimbLimit)
		{
			RZL_Hooks.s_iClimbRefused++;
			m_Character.m_iRZLClimbRefused++;
			SetVariableOut("CanClimb", false);
			return ENodeResult.FAIL;
		}

		m_Character.m_iRZLClimbOk++;
		return super.EOnTaskSimulate(owner, dt);
	}
}

//------------------------------------------------------------------------------------------------
//! L12: the melee gate (holds Bacon's swing while our wind-up runs).
modded class Bacon_622120A5448725E3_AICanMeleeTarget
{
	//------------------------------------------------------------------------------------------------
	override bool TestFunction(AIAgent owner)
	{
		RZL_Hooks.s_iMelee++;
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(owner.GetControlledEntity());
		if (ch && ch.m_bRZLMeleeHold)
		{
			RZL_Hooks.s_iMeleeHeld++;
			return false;
		}

		bool ok = super.TestFunction(owner);
		if (ok && ch)
			ch.m_iRZLCanMelee++;
		return ok;
	}
}

//------------------------------------------------------------------------------------------------
//! L13: the think rate (the wait at the top of every behaviour-tree loop).
modded class Bacon_622120A5448725E3_GetAILODWait
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		RZL_Hooks.s_iLodWait++;
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(owner.GetControlledEntity());
		if (ch)
		{
			ch.m_iRZLLoops++;
			if (ch.m_fRZLWait > 0)
			{
				RZL_Hooks.s_iLodWaitOurs++;
				SetVariableOut("LODWait", ch.m_fRZLWait);
				return ENodeResult.SUCCESS;
			}
		}

		return super.EOnTaskSimulate(owner, dt);
	}
}

//------------------------------------------------------------------------------------------------
//! L15: the voice (a muted body says nothing on its own).
modded class Bacon_622120A5448725E3_AIInfectedTaskSpeak
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		RZL_Hooks.s_iSpeak++;
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(owner.GetControlledEntity());
		if (ch && ch.m_bRZLMute)
		{
			RZL_Hooks.s_iSpeakMuted++;
			return ENodeResult.SUCCESS;
		}

		return super.EOnTaskSimulate(owner, dt);
	}
}

//------------------------------------------------------------------------------------------------
//! Stamps when a body's loiter point was set (Roam's suspect branch, Combat and the waypoint branch).
modded class Bacon_622120A5448725E3_AIUpdateLoiterPoint
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		RZL_Hooks.s_iUpdateLoiter++;
		ENodeResult r = super.EOnTaskSimulate(owner, dt);
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(owner.GetControlledEntity());
		if (ch)
			ch.m_fRZLUpdLoiterAt = RZL_Hooks.Now();
		return r;
	}
}

//------------------------------------------------------------------------------------------------
//! L23: the straight-path cost of Bacon's chase move, per body.
modded class SCR_CreateAIExtraMoveParams
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		Bacon_622120A5448725E3_InfectedCharacter ch = Bacon_622120A5448725E3_InfectedCharacter.Cast(owner.GetControlledEntity());
		if (!ch)
			return super.EOnTaskSimulate(owner, dt);

		RZL_Hooks.s_iMoveParams++;
		if (ch.m_fRZLPathCost <= 0)
			return super.EOnTaskSimulate(owner, dt);

		RZL_Hooks.s_iMoveParamsOurs++;
		float keep = m_fStraightPathCostModifier;
		m_fStraightPathCostModifier = ch.m_fRZLPathCost;
		ENodeResult r = super.EOnTaskSimulate(owner, dt);
		m_fStraightPathCostModifier = keep;
		return r;
	}
}
