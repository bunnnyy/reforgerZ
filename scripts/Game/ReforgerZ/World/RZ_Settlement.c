enum RZ_ESettlementState
{
	SAFE,		// < 20 infection
	CONTESTED,	// 20 - 50
	INFESTED,	// 50 - 80
	OVERRUN		// 80+
}

//! A town, village or cluster of buildings found on the current map.
//! Settlements are what make the mode map-agnostic: zombies, missions,
//! spawn points and the lab are all placed relative to them.
class RZ_Settlement
{
	string m_sName;
	vector m_vCenter;
	float m_fRadius = 200;
	int m_iSize = 1;			// 1 = village / outskirts, 2 = town, 3 = city
	float m_fInfection = 30;	// 0..100
	bool m_bHasLab;
	float m_fNoSpreadSeconds;	// Quarantine Line: this town doesn't spread infection for a while (not saved)
	float m_fSensorSeconds;		// Install Sensors: twice the specials here for a while (not saved)
	float m_fLedger;			// crawlers killed here lately, halving every 30 played minutes (Zombies/RZ_Crawlers.c; saved)
	float m_fStir;				// 0-100: stirred at 50, falling 20 an hour played (Zombies/RZ_Crawlers.c; saved)

	//------------------------------------------------------------------------------------------------
	void AddInfection(float amount)
	{
		// A town cleared after the cure stays cleared: nothing raises infection in the epilogue
		if (amount > 0 && RZ_WorldFacts.Get() && RZ_WorldFacts.Get().IsEpilogue())
			return;

		m_fInfection = Math.Clamp(m_fInfection + amount, 0, 100);
	}

	//------------------------------------------------------------------------------------------------
	RZ_ESettlementState GetState()
	{
		if (m_fInfection >= 80)
			return RZ_ESettlementState.OVERRUN;
		if (m_fInfection >= 50)
			return RZ_ESettlementState.INFESTED;
		if (m_fInfection >= 20)
			return RZ_ESettlementState.CONTESTED;
		return RZ_ESettlementState.SAFE;
	}

	//------------------------------------------------------------------------------------------------
	string GetStateName()
	{
		switch (GetState())
		{
			case RZ_ESettlementState.SAFE: return "Safe";
			case RZ_ESettlementState.CONTESTED: return "Contested";
			case RZ_ESettlementState.INFESTED: return "Infested";
			case RZ_ESettlementState.OVERRUN: return "Overrun";
		}

		return "Unknown";
	}

	//------------------------------------------------------------------------------------------------
	static float DefaultRadiusForSize(int size)
	{
		if (size >= 3)
			return 450;
		if (size == 2)
			return 280;
		return 160;
	}
}
