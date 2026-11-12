// The power station's looks, once (tools/runtime_test/run.sh): the dressing beside the control, the
// generator's sound while it turns, everything down with nobody near, and the street lamps' list
// for the clients, dark and powered. Data and look-only props: the station's step is put back after.
modded class RZ_PowerLooks
{
	//------------------------------------------------------------------------------------------------
	int RZT_SoundsUp()
	{
		int up = 0;
		foreach (IEntity e : m_aSounds)
		{
			if (e)
				up++;
		}

		return up;
	}

	bool RZT_DressingUp() { return m_Dressing != null; }
	bool RZT_DressingWanted() { return m_bDress && !m_sDressing.IsEmpty(); }
	int RZT_SoundsWanted()
	{
		if (m_sSound.IsEmpty())
			return 0;

		return m_aSoundPos.Count();
	}
}

modded class RZ_PowerGrid
{
	//------------------------------------------------------------------------------------------------
	string RZT_SelfTest()
	{
		string bad = "";
		// Someone near a dark station: the dressing, no sound
		m_Looks.Sync(true, false, false);
		if (m_Looks.RZT_DressingUp() != m_Looks.RZT_DressingWanted())
			bad += " dressing";
		if (m_Looks.RZT_SoundsUp() != 0)
			bad += " sound-while-dark";

		// The generator turning
		m_Looks.Sync(true, false, true);
		int turning = m_Looks.RZT_SoundsUp();
		if (turning != m_Looks.RZT_SoundsWanted())
			bad += " sound";

		// It stops: the sound goes, the dressing stays
		m_Looks.Sync(true, false, false);
		if (m_Looks.RZT_SoundsUp() != 0 || m_Looks.RZT_DressingUp() != m_Looks.RZT_DressingWanted())
			bad += " stop";

		string seen = "no dressing";
		if (m_Looks.RZT_DressingUp())
			seen = "dressing up";

		// Nobody near: all down
		m_Looks.Sync(false, true, true);
		if (m_Looks.RZT_DressingUp() || m_Looks.RZT_SoundsUp() != 0)
			bad += " gone";

		// The lamps' list: dark, then powered (the station and its towns)
		int step = m_iStep;
		m_iStep = STEP_DARK;
		PushLamps();
		string dark = m_GM.GetPowerLamps();
		m_iStep = STEP_POWERED;
		PushLamps();
		string lit = m_GM.GetPowerLamps();
		m_iStep = step;
		PushLamps();
		array<string> circles = {};
		lit.Split(";", circles, true);
		if (m_Settings.m_bStreetLamps && (dark != "L" || circles.Count() != m_aRegion.Count() + 2))
			bad += " lamps";

		string verdict = "OK";
		if (!bad.IsEmpty())
			verdict = "FAILED (" + bad.Trim() + ")";

		return string.Format("Power test: %1, %2 sound(s) while it turns, lamps dark [%3] and powered in %4 place(s): %5", seen, turning, dark, circles.Count() - 1, verdict);
	}
}
