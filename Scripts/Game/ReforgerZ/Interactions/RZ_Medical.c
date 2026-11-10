//! Medical help between players (docs/DESIGN.md, "World interactions"). "Treat" on another
//! player's character: a first-aid kit and 20 seconds next to them, and they're patched up.
//! Nurses and Doctors are much faster and can do it with just a bandage, and slow a bitten
//! player's infection a little (once per life). Blood and saline are the vanilla items, given
//! the vanilla way; carrying the wounded is whatever the game version offers (ReforgerZ adds none).

//------------------------------------------------------------------------------------------------
//! "Treat <name>": on the player character prefab's ActionsManagerComponent.
class RZ_TreatOtherAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Treat their wounds";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		return owner && user && owner != user && RZ_Util.IsCharacterAlive(owner);
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer() || pOwnerEntity == pUserEntity)
			return;

		RZ_Interactions hub = RZ_Interactions.Get();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!hub || playerId <= 0)
			return;

		hub.GetMedical().StartTreat(playerId, pUserEntity, pOwnerEntity);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Medical
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_InteractionSettings m_Settings;
	protected ref map<string, float> m_mTreatedAt = new map<string, float>();
	protected ref array<string> m_aSlowed = {};

	//------------------------------------------------------------------------------------------------
	void RZ_Medical(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_InteractionSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! A new life for the patient: the once-per-life slowing can happen again.
	void OnPlayerSpawned(int playerId)
	{
		m_aSlowed.RemoveItem(m_GM.GetProfile(playerId).m_sIdentity);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMedic(int playerId)
	{
		return RZ_Interactions.IsBackground(m_GM, playerId, "nurse") || RZ_Interactions.IsBackground(m_GM, playerId, "doctor");
	}

	//------------------------------------------------------------------------------------------------
	protected float SecondsFor(int playerId)
	{
		float seconds = m_Settings.m_fTreatSeconds;
		if (RZ_Interactions.IsBackground(m_GM, playerId, "doctor"))
			return seconds * m_Settings.m_fDoctorMult;

		if (RZ_Interactions.IsBackground(m_GM, playerId, "nurse"))
			return seconds * m_Settings.m_fNurseMult;

		return seconds;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasSupplies(int playerId, IEntity user)
	{
		if (RZ_ItemHelper.Has(user, m_Settings.m_sMedkitWords))
			return true;

		return IsMedic(playerId) && RZ_ItemHelper.Has(user, m_Settings.m_sBandageWords);
	}

	//------------------------------------------------------------------------------------------------
	void StartTreat(int playerId, IEntity user, IEntity target)
	{
		if (!m_Settings.m_bTreatOthers || !target || m_Hub.HasTask(playerId))
			return;

		int patient = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(target);
		if (patient <= 0)
			return;

		string ident = m_GM.GetProfile(patient).m_sIdentity;
		float last;
		if (m_mTreatedAt.Find(ident, last) && m_Hub.GetClock() - last < m_Settings.m_fTreatCooldownMinutes * 60)
		{
			m_GM.NotifyPlayer(playerId, "Treat", "They've just been seen to. More won't help yet.");
			return;
		}

		if (!HasSupplies(playerId, user))
		{
			string need = "You need a first-aid kit.";
			if (IsMedic(playerId))
				need = "You need a first-aid kit or at least a bandage.";

			m_GM.NotifyPlayer(playerId, "Treat", need);
			return;
		}

		float seconds = SecondsFor(playerId);
		RZ_TimedTask t = m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_TREAT, -1, patient, seconds, "the treatment", false);
		t.m_Target = target;
		m_GM.NotifyPlayer(playerId, "Treat", string.Format("You kneel and get to work. About %1 seconds; stay with them.", RZ_Util.RoundToInt(seconds)));
		m_GM.NotifyPlayer(patient, "Treat", string.Format("%1 is patching you up. Hold still.", RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	void CompleteTreat(int playerId, IEntity user, IEntity target, int patient)
	{
		if (!target || !RZ_Util.IsCharacterAlive(target))
			return;

		bool medic = IsMedic(playerId);
		bool used = RZ_ItemHelper.Take(user, m_Settings.m_sMedkitWords, 1) > 0;
		if (!used && medic)
			used = RZ_ItemHelper.Take(user, m_Settings.m_sBandageWords, 1) > 0;

		if (!used)
		{
			m_GM.NotifyPlayer(playerId, "Treat", "You reach for the kit and it's not there.");
			return;
		}

		RZ_Util.HealCharacter(target);
		RZ_PlayerProfile profile = m_GM.GetProfile(patient);
		m_mTreatedAt.Set(profile.m_sIdentity, m_Hub.GetClock());

		string extra = "";
		if (medic && profile.IsInfected() && !m_aSlowed.Contains(profile.m_sIdentity))
		{
			m_aSlowed.Insert(profile.m_sIdentity);
			profile.m_fInfectionLeft = profile.m_fInfectionLeft * m_Settings.m_fTreatSlowsInfection;
			extra = string.Format(" You clean the bite properly. It buys them a little time: about %1 minutes. Not a cure.", RZ_Util.RoundToInt(profile.m_fInfectionLeft / 60));
		}

		m_GM.NotifyPlayer(playerId, "Treat", "Cleaned, dressed, done." + extra);
		m_GM.NotifyPlayer(patient, "Treat", string.Format("%1 has patched you up.", RZ_Util.PlayerName(playerId)));
	}
}
