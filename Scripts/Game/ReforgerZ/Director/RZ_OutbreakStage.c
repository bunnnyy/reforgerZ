enum RZ_EOutbreakStage
{
	EARLY_OUTBREAK,	// Power still on, military still fighting, runners are rare
	COLLAPSE,		// Towns falling, military pulling out
	OVERRUN,		// Hordes everywhere, specials common
	AFTERMATH		// The dust settles: fewer but nastier zombies, big roaming hordes
}

//! Tuning for one outbreak stage. Every value has a sensible default, so the
//! game mode runs even if you leave the stage list empty in Workbench.
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(RZ_EOutbreakStage, "m_eStage")]
class RZ_StageSettings
{
	[Attribute("0", UIWidgets.ComboBox, "Stage these settings apply to", "", ParamEnumArray.FromEnum(RZ_EOutbreakStage))]
	RZ_EOutbreakStage m_eStage;

	[Attribute("1", desc: "Zombie density multiplier")]
	float m_fDensityMult = 1;

	[Attribute("0.03", desc: "Chance that a spawned zombie is a special (drops research samples)")]
	float m_fSpecialChance = 0.03;

	[Attribute("0.5", desc: "Infection points per minute that an infested town spreads to its neighbours")]
	float m_fSpreadPerMinute = 0.5;

	[Attribute("3", desc: "Concurrent missions (plus one per 4 players)")]
	int m_iBaseMissions = 3;

	[Attribute("120", desc: "Minutes before advancing to the next stage. 0 = only advance on infection level")]
	float m_fDurationMinutes = 120;

	[Attribute("1", desc: "How often military missions show up (military pulls out as things get worse)")]
	float m_fMilitaryPresence = 1;

	//------------------------------------------------------------------------------------------------
	static RZ_StageSettings CreateDefault(RZ_EOutbreakStage stage)
	{
		RZ_StageSettings s = new RZ_StageSettings();
		s.m_eStage = stage;
		switch (stage)
		{
			case RZ_EOutbreakStage.EARLY_OUTBREAK:
				s.Set(0.6, 0.01, 0.3, 3, 90, 1.0);
				break;
			case RZ_EOutbreakStage.COLLAPSE:
				s.Set(1.0, 0.03, 0.6, 4, 150, 0.6);
				break;
			case RZ_EOutbreakStage.OVERRUN:
				s.Set(1.5, 0.06, 0.9, 4, 180, 0.25);
				break;
			case RZ_EOutbreakStage.AFTERMATH:
				s.Set(0.8, 0.08, 0.2, 5, 0, 0.1);
				break;
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected void Set(float density, float special, float spread, int missions, float duration, float military)
	{
		m_fDensityMult = density;
		m_fSpecialChance = special;
		m_fSpreadPerMinute = spread;
		m_iBaseMissions = missions;
		m_fDurationMinutes = duration;
		m_fMilitaryPresence = military;
	}

	//------------------------------------------------------------------------------------------------
	static string GetStageName(RZ_EOutbreakStage stage)
	{
		switch (stage)
		{
			case RZ_EOutbreakStage.EARLY_OUTBREAK: return "Early Outbreak";
			case RZ_EOutbreakStage.COLLAPSE: return "Collapse";
			case RZ_EOutbreakStage.OVERRUN: return "Overrun";
			case RZ_EOutbreakStage.AFTERMATH: return "Aftermath";
		}

		return "Unknown";
	}

	//------------------------------------------------------------------------------------------------
	static string GetStageAnnouncement(RZ_EOutbreakStage stage)
	{
		switch (stage)
		{
			case RZ_EOutbreakStage.EARLY_OUTBREAK: return "Emergency broadcast: stay indoors. The military has the situation under control.";
			case RZ_EOutbreakStage.COLLAPSE: return "The broadcasts have stopped. Military convoys are leaving. Towns are going dark.";
			case RZ_EOutbreakStage.OVERRUN: return "There is no one coming. The dead outnumber the living.";
			case RZ_EOutbreakStage.AFTERMATH: return "The world has gone quiet. What's left out there has had time to change.";
		}

		return "";
	}
}
