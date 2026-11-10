//! "Heat" is how much attention an area has drawn: players hanging around, vehicles,
//! radio beacons, and (later) gunfire. Heat draws extra zombies, and enough of it
//! sends a horde. Other mods can add heat through RZ_GameModeComponent.ReportNoise().
class RZ_HeatMap
{
	protected ref map<int, float> m_mHeat = new map<int, float>();
	protected float m_fCell = 250;
	protected float m_fDecayPerMinute = 8;
	protected float m_fMaxHeat = 200;
	protected float m_fDecayMult = 1;	// world events (Storm) change these for a while
	protected float m_fAddMult = 1;

	protected static const int GRID_STRIDE = 100000;

	//------------------------------------------------------------------------------------------------
	protected int Key(vector p)
	{
		int gx = Math.Floor(p[0] / m_fCell);
		int gz = Math.Floor(p[2] / m_fCell);
		return gx * GRID_STRIDE + gz;
	}

	//------------------------------------------------------------------------------------------------
	protected vector CellCenter(int key)
	{
		int gx = key / GRID_STRIDE;
		int gz = key % GRID_STRIDE;
		vector p = Vector((gx + 0.5) * m_fCell, 0, (gz + 0.5) * m_fCell);
		return RZ_Util.GroundPos(p);
	}

	//------------------------------------------------------------------------------------------------
	void SetEventModifiers(float decayMult, float addMult)
	{
		m_fDecayMult = decayMult;
		m_fAddMult = addMult;
	}

	//------------------------------------------------------------------------------------------------
	void AddHeat(vector pos, float amount)
	{
		if (amount > 0)
			amount = amount * m_fAddMult;

		int key = Key(pos);
		float v;
		m_mHeat.Find(key, v);
		m_mHeat.Set(key, Math.Clamp(v + amount, 0, m_fMaxHeat));
	}

	//------------------------------------------------------------------------------------------------
	float GetHeat(vector pos)
	{
		float v;
		m_mHeat.Find(Key(pos), v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	void Decay(float dt)
	{
		float amount = m_fDecayPerMinute * m_fDecayMult * dt / 60;
		array<int> keys = {};
		foreach (int key, float v : m_mHeat)
		{
			keys.Insert(key);
		}

		foreach (int key : keys)
		{
			float v = m_mHeat.Get(key) - amount;
			if (v <= 0)
				m_mHeat.Remove(key);
			else
				m_mHeat.Set(key, v);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool GetHottest(out vector pos, out float heat)
	{
		heat = 0;
		int bestKey;
		bool found = false;
		foreach (int key, float v : m_mHeat)
		{
			if (v > heat)
			{
				heat = v;
				bestKey = key;
				found = true;
			}
		}

		if (found)
			pos = CellCenter(bestKey);

		return found;
	}
}
