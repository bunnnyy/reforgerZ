//! The faults table (docs/ERRANDS.md, "Faults"): small real faults at a place that some routines
//! answer. A Bastion post's relief goes missing or its set runs low; a Halcyon station's freezers
//! run short of fuel or a sensor goes dark; a safehouse's generator runs dry or its barrels run low.
//! Each place rolls its faults at most once a shift (the shifts change at 05:30 and 17:30), and only
//! while a player is within 2 km; they clear at the next shift, or when an errand fixes them.
//! Shown through existing outputs: one guard fewer at a post (RZ_Territory), a line on the
//! faction's secure channel (RZ_FactionLife.GetSecureTraffic), a line in the house's status
//! (RZ_Safehouses). A fault never drains real stock: if nobody helps, the line says so and
//! nothing is lost. Runtime only. Server only.

//------------------------------------------------------------------------------------------------
enum RZ_EFault
{
	RELIEF = 1,		// a Bastion post's relief is missing: one guard fewer at the gate
	SET = 2,		// a Bastion post's radio set is low: its nightly numbers are a hiss
	FUEL = 4,		// a station's freezers or a safehouse's generator are low on fuel
	SENSOR = 8,		// a Halcyon sensor 300-500 m out has gone dark
	WATER = 16		// a safehouse's barrels are low
}

//------------------------------------------------------------------------------------------------
class RZ_Faults
{
	protected static const float ROLL_RADIUS = 2000;

	protected RZ_GameModeComponent m_GM;
	protected ref map<string, int> m_mFaults = new map<string, int>();		// place key -> RZ_EFault bits
	protected ref map<string, int> m_mRolled = new map<string, int>();		// place key -> shift rolled in
	protected int m_iShift;
	protected int m_iHalf = -1;	// 0 the day shift (05:30-17:30), 1 the night shift

	//------------------------------------------------------------------------------------------------
	void RZ_Faults(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	static string HoldingPlace(int holdingId)
	{
		return "h" + holdingId.ToString();
	}

	//------------------------------------------------------------------------------------------------
	static string HousePlace(int idx)
	{
		return "s" + idx.ToString();
	}

	//------------------------------------------------------------------------------------------------
	bool Has(string place, RZ_EFault fault)
	{
		int bits;
		m_mFaults.Find(place, bits);
		return (bits & fault) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! An errand fixed it (a battery in the set, a can in the generator).
	void Clear(string place, RZ_EFault fault)
	{
		int bits;
		if (!m_mFaults.Find(place, bits))
			return;

		bits = bits - (bits & fault);
		m_mFaults.Set(place, bits);
		RZ_Log.Info(string.Format("Errands: fault %1 at %2 fixed", FaultName(fault), place));
	}

	//------------------------------------------------------------------------------------------------
	static string FaultName(int fault)
	{
		switch (fault)
		{
			case RZ_EFault.RELIEF: return "relief";
			case RZ_EFault.SET: return "set";
			case RZ_EFault.FUEL: return "fuel";
			case RZ_EFault.SENSOR: return "sensor";
			case RZ_EFault.WATER: return "water";
		}

		return "none";
	}

	//------------------------------------------------------------------------------------------------
	//! From the errands' 30 s tick: a new shift clears every fault; then each place within 2 km of a
	//! player that hasn't rolled this shift rolls its own kinds once, each at `chance`.
	void Tick(array<IEntity> players, float chance)
	{
		float hour = RZ_GameTime.GetHour();
		if (hour < 0)
			return;

		int half = 1;
		if (hour >= 5.5 && hour < 17.5)
			half = 0;

		if (half != m_iHalf)
		{
			if (m_iHalf >= 0)
				m_iShift++;

			m_iHalf = half;
			m_mFaults.Clear();
		}

		if (players.IsEmpty() || chance <= 0)
			return;

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				int kinds = 0;
				if (h.m_eFaction == RZ_EFaction.MILITARY)
					kinds = RZ_EFault.RELIEF | RZ_EFault.SET;
				else if (h.m_eFaction == RZ_EFaction.INSTITUTE)
					kinds = RZ_EFault.FUEL | RZ_EFault.SENSOR;

				if (kinds != 0)
					Roll(HoldingPlace(h.m_iId), h.m_vPos, kinds, players, chance);
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse sh = houses.Get(i);
				if (sh && sh.m_iPopulation > 0)
					Roll(HousePlace(i), sh.m_vPos, RZ_EFault.FUEL | RZ_EFault.WATER, players, chance);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Roll(string place, vector pos, int kinds, array<IEntity> players, float chance)
	{
		int rolled;
		if (m_mRolled.Find(place, rolled) && rolled == m_iShift)
			return;

		if (RZ_Util.NearestDistance(pos, players) > ROLL_RADIUS)
			return;

		m_mRolled.Set(place, m_iShift);
		int bits = 0;
		for (int bit = 1; bit <= RZ_EFault.WATER; bit = bit * 2)
		{
			if ((kinds & bit) != 0 && Math.RandomFloat01() < chance)
				bits = bits | bit;
		}

		m_mFaults.Set(place, bits);
		if (bits != 0)
			RZ_Log.Info(string.Format("Errands: faults at %1 this shift: %2", place, Describe(bits)));
	}

	//------------------------------------------------------------------------------------------------
	static string Describe(int bits)
	{
		string text = "";
		for (int bit = 1; bit <= RZ_EFault.WATER; bit = bit * 2)
		{
			if ((bits & bit) == 0)
				continue;

			if (!text.IsEmpty())
				text += ", ";

			text += FaultName(bit);
		}

		return text;
	}

	// ================================================================ What the faults show
	//------------------------------------------------------------------------------------------------
	//! RZ_Territory: guards at a holding this shift (one fewer while its relief is missing).
	int Guards(int holdingId, int guards)
	{
		if (Has(HoldingPlace(holdingId), RZ_EFault.RELIEF))
			return Math.Max(0, guards - 1);

		return guards;
	}

	//------------------------------------------------------------------------------------------------
	//! RZ_FactionLife.GetSecureTraffic: what a faction's own channel says about its faults now.
	void SecureLines(int f, notnull array<string> result)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return;

		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h.m_eFaction != f)
				continue;

			string place = HoldingPlace(h.m_iId);
			if (f == RZ_EFaction.MILITARY && Has(place, RZ_EFault.SET))
				result.Insert(string.Format("\"%1, say again. You're all hiss. Change your battery and call it in.\"", h.m_sName));

			if (f == RZ_EFaction.MILITARY && Has(place, RZ_EFault.RELIEF))
				result.Insert(string.Format("\"%1: relief not arrived. One on the gate until further notice.\"", h.m_sName));

			if (f == RZ_EFaction.INSTITUTE && Has(place, RZ_EFault.FUEL))
				result.Insert(string.Format("\"%1 station: freezer fuel below one night. Samples at risk from 02:00.\"", h.m_sName));

			if (f == RZ_EFaction.INSTITUTE && Has(place, RZ_EFault.SENSOR))
				result.Insert(string.Format("\"%1 station: perimeter sensor four is dark. No readings since the shift change.\"", h.m_sName));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! RZ_Safehouses' status line: what a house says about its own faults, "" for none.
	string HouseLine(int idx)
	{
		string place = HousePlace(idx);
		string text = "";
		if (Has(place, RZ_EFault.FUEL))
			text = "The generator's on its last cupful. The lamps go out at dusk unless someone finds a can.";

		if (Has(place, RZ_EFault.WATER))
		{
			if (!text.IsEmpty())
				text += " ";

			text += "The water barrels are down to the sludge.";
		}

		return text;
	}
}
