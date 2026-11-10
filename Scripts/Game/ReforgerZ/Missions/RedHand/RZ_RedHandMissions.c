//! The Red Hand (Bandits) contracts. See docs/MISSIONS.md, "The Red Hand".

//------------------------------------------------------------------------------------------------
//! #1 Dead Drop: the betrayal path. The Red Hand buys samples, paying in loot
//! and Favor. Every sample sold is one the lab never sees, and Halcyon finds out.
//! ★ Ex-Con: 25% better pay.
class RZ_Mission_BanditDeadDrop : RZ_Mission
{
	protected IEntity m_Drop;
	protected int m_iSold;

	protected static const int BANDIT_FAME_PER_SAMPLE = 8;
	protected static const int BANDIT_FAVOR_PER_SAMPLE = 15;
	protected static const int INSTITUTE_INFAMY_PER_SAMPLE = 6;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.STRANGER, 0, 0); // paid per sample instead
		m_Req.m_bNotCured = true;
		m_Req.Bonus("excon");
		m_Req.Bonus("raider");
		m_fTimeLeft = 1500;
		m_fParticipationRadius = 0; // only sellers count
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetDeadDropPrefab().IsEmpty())
			return 0;

		// The Red Hand gets bolder as order collapses
		return 0.5 + gm.GetDirector().GetStage() * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 10, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius * 0.9);
		m_Drop = RZ_Util.SpawnPrefab(gm.GetDeadDropPrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Drop)
			return false;

		m_sTitle = "Red Hand dead drop";
		m_sBrief = string.Format("The Red Hand is paying well for infected samples. No questions asked. There's a drop near %1.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDrop(IEntity e)
	{
		return e && e == m_Drop;
	}

	//------------------------------------------------------------------------------------------------
	void OnSold(RZ_GameModeComponent gm, int playerId, int value)
	{
		m_iSold += value;
		if (!m_aParticipants.Contains(playerId))
			m_aParticipants.Insert(playerId);

		float pay = 1;
		if (HasBonus(gm, playerId))
			pay = 1.25;

		int fame = RZ_Util.RoundToInt(BANDIT_FAME_PER_SAMPLE * value * pay);
		int favor = RZ_Util.RoundToInt(BANDIT_FAVOR_PER_SAMPLE * value * pay);
		gm.AddFame(playerId, RZ_EFaction.BANDITS, fame, false);
		gm.AddFavor(playerId, RZ_EFaction.BANDITS, favor);
		gm.AddInfamy(playerId, RZ_EFaction.INSTITUTE, INSTITUTE_INFAMY_PER_SAMPLE * value);
		gm.NotifyPlayer(playerId, "The Red Hand", string.Format("Pleasure doing business. +%1 Fame, +%2 Favor with the Red Hand. Halcyon won't like it.", fame, favor));

		// One crate per sale, bigger sales get two
		gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 2, 4));
		if (value * pay >= 3)
			gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 2, 4));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		// The drop runs until the Red Hand packs up; selling doesn't end it early
		if (m_iSold > 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
		else
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.NotifyAll("Halcyon", string.Format("Someone sold %1 samples to the Red Hand. That's research we'll never get back.", m_iSold));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Drop);
	}
}
