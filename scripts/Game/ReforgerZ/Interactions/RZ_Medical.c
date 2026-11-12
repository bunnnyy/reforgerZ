//! Medical help between players (docs/DESIGN.md, "Treating each other"). Treating someone else is
//! vanilla's own: hold a bandage, tourniquet, morphine or saline and use it on them (vanilla's heal
//! action on another character). ReforgerZ adds one thing on top: a Nurse or Doctor who treats a
//! bitten player that way slows the infection a little (x the time left, once per life). Nothing
//! else is ours: no prompt, no timer, no items of our own.

//------------------------------------------------------------------------------------------------
//! vanilla's consumables: the one place a medical item's effect lands on its target, whoever used
//! it. Self-treatment and anything that isn't a health item pass straight through.
modded class SCR_ConsumableItemComponent
{
	//------------------------------------------------------------------------------------------------
	override void ApplyItemEffect(IEntity target, IEntity user, ItemUseParameters animParams, IEntity item, bool deleteItem = true)
	{
		// VERIFY: ApplyItemEffect runs on the server when a medic finishes vanilla's heal on another character (SCR_HealingUserAction, then the item-use event)
		if (Replication.IsServer() && target && user && target != user && SCR_ConsumableEffectHealthItems.Cast(GetConsumableEffect()))
			RZ_Medical.OnHealedOther(user, target);

		super.ApplyItemEffect(target, user, animParams, item, deleteItem);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Medical
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_InteractionSettings m_Settings;

	//------------------------------------------------------------------------------------------------
	void RZ_Medical(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_InteractionSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! From vanilla's consumables, on the server: `user` treated `target` with a health item.
	static void OnHealedOther(IEntity user, IEntity target)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !hub.GetMedical())
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		int helper = pm.GetPlayerIdFromControlledEntity(user);
		int patient = pm.GetPlayerIdFromControlledEntity(target);
		if (helper <= 0 || patient <= 0 || helper == patient)
			return;

		hub.GetMedical().SlowBite(helper, patient);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMedic(int playerId)
	{
		return RZ_Interactions.IsBackground(m_GM, playerId, "nurse") || RZ_Interactions.IsBackground(m_GM, playerId, "doctor");
	}

	//------------------------------------------------------------------------------------------------
	//! A Nurse or Doctor treated a player whose infection has taken: the time left x Medic slows a
	//! bite, once per life (a profile flag, m_bMedicSlowed, cleared by StartNewLife and saved, so a
	//! restart can't grant it twice).
	void SlowBite(int helper, int patient)
	{
		if (m_Settings.m_fTreatSlowsInfection <= 1 || !IsMedic(helper))
			return;

		RZ_PlayerProfile profile = m_GM.GetProfile(patient);
		if (!profile || !profile.HasInfectionTaken() || profile.m_bMedicSlowed)
			return;

		profile.m_bMedicSlowed = true;
		profile.m_fInfectionLeft = profile.m_fInfectionLeft * m_Settings.m_fTreatSlowsInfection;
		m_GM.NotifyPlayer(helper, "The bite", string.Format("You clean the bite properly while you're at it. It buys them a little time: about %1 minutes. Not a cure.", RZ_Util.RoundToInt(profile.m_fInfectionLeft / 60)));
		m_GM.NotifyPlayer(patient, "The bite", string.Format("%1 cleaned the bite properly. It burns less. It's still there.", RZ_Util.PlayerName(helper)));
	}
}
