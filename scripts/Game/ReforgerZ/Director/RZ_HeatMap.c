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
	protected float m_fEnvAddMult = 1;	// time and weather (RZ_Environment): rain dampens noise
	protected float m_fEnvSpread;		// night: this share also reaches the four neighbouring squares
	protected bool m_bFogBank;			// the Fog Bank event: heat added inside it is muffled
	protected vector m_vFogPos;
	protected float m_fFogRadius;
	protected float m_fFogMult = 1;

	protected static const int GRID_STRIDE = 100000;
	protected static const float NOISE_EVENT_MIN = 8;	// heat this big at once counts as a noise (bottle, firecracker, alarm clock)
	protected static const int NOISE_EVENTS_KEPT = 16;

	// Recent loud noises, so a mission can make infected turn toward one (unarmed missions)
	protected ref array<vector> m_aNoisePos = {};
	protected ref array<float> m_aNoiseAt = {};
	protected ref array<float> m_aNoiseAmount = {};
	protected ref array<int> m_aNoiseId = {};
	protected int m_iNextNoiseId;
	protected float m_fClock;

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
	//! Time and weather (RZ_Environment): addMult scales heat added (rain 0.7), spread is the
	//! share that also reaches the neighbouring squares (night: they hear further).
	void SetEnvironmentModifiers(float addMult, float spread)
	{
		m_fEnvAddMult = addMult;
		m_fEnvSpread = Math.Clamp(spread, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The Fog Bank event (RZ_Environment): heat added within radius of pos is multiplied by mult.
	void SetFogBank(bool on, vector pos, float radius, float mult)
	{
		m_bFogBank = on;
		m_vFogPos = pos;
		m_fFogRadius = radius;
		m_fFogMult = Math.Clamp(mult, 0.1, 1);
	}

	//------------------------------------------------------------------------------------------------
	void AddHeat(vector pos, float amount)
	{
		// A noise leads infected however much the weather muffles the heat it leaves
		if (amount >= NOISE_EVENT_MIN)
			RecordNoise(pos, amount);

		if (amount > 0)
			amount = amount * m_fAddMult * m_fEnvAddMult;

		if (amount > 0 && m_bFogBank && RZ_Util.DistXZ(pos, m_vFogPos) <= m_fFogRadius)
			amount = amount * m_fFogMult;

		int key = Key(pos);
		AddToCell(key, amount);

		if (amount <= 0 || m_fEnvSpread <= 0)
			return;

		float side = amount * m_fEnvSpread;
		AddToCell(key + 1, side);
		AddToCell(key - 1, side);
		AddToCell(key + GRID_STRIDE, side);
		AddToCell(key - GRID_STRIDE, side);
	}

	//------------------------------------------------------------------------------------------------
	protected void RecordNoise(vector pos, float amount)
	{
		m_aNoisePos.Insert(pos);
		m_aNoiseAt.Insert(m_fClock);
		m_aNoiseAmount.Insert(amount);
		m_iNextNoiseId++;
		m_aNoiseId.Insert(m_iNextNoiseId);
		while (m_aNoisePos.Count() > NOISE_EVENTS_KEPT)
		{
			m_aNoisePos.RemoveOrdered(0);
			m_aNoiseAt.RemoveOrdered(0);
			m_aNoiseAmount.RemoveOrdered(0);
			m_aNoiseId.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The newest remembered noise after afterId (each noise has a rising id), within radius of
	//! center and maxAge seconds: crawlers drift to each noise once (Zombies/RZ_Crawlers.c).
	bool NewestNoise(int afterId, vector center, float radius, float maxAge, out int id, out vector pos)
	{
		for (int i = m_aNoisePos.Count() - 1; i >= 0; i--)
		{
			if (m_aNoiseId[i] <= afterId)
				return false;

			if (m_fClock - m_aNoiseAt[i] > maxAge || RZ_Util.DistXZ(m_aNoisePos[i], center) > radius)
				continue;

			id = m_aNoiseId[i];
			pos = m_aNoisePos[i];
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The loudest noise (a noisemaker, an alarm, a burst of fire) within radius of center in the
	//! last maxAge seconds. Infected that a mission controls can be sent toward it. False if none.
	bool FindRecentNoise(vector center, float radius, float maxAge, out vector pos)
	{
		float best = 0;
		bool found = false;
		foreach (int i, vector p : m_aNoisePos)
		{
			if (m_fClock - m_aNoiseAt[i] > maxAge || RZ_Util.DistXZ(p, center) > radius)
				continue;

			if (m_aNoiseAmount[i] > best)
			{
				best = m_aNoiseAmount[i];
				pos = p;
				found = true;
			}
		}

		return found;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddToCell(int key, float amount)
	{
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
		m_fClock += dt;
		float amount = m_fDecayPerMinute * m_fDecayMult * dt / 60;
		array<int> keys = {};
		foreach (int cellKey, float cellHeat : m_mHeat)
		{
			keys.Insert(cellKey);
		}

		foreach (int key : keys)
		{
			float remaining = m_mHeat.Get(key) - amount;
			if (remaining <= 0)
				m_mHeat.Remove(key);
			else
				m_mHeat.Set(key, remaining);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The hottest square whose centre is within range of one of the given entities (living players).
	bool GetHottestNear(array<IEntity> players, float range, out vector pos, out float heat)
	{
		heat = 0;
		int bestKey;
		bool found = false;
		foreach (int key, float v : m_mHeat)
		{
			if (v <= heat)
				continue;

			int gx = key / GRID_STRIDE;
			int gz = key % GRID_STRIDE;
			vector flat = Vector((gx + 0.5) * m_fCell, 0, (gz + 0.5) * m_fCell);
			if (RZ_Util.NearestDistance(flat, players) > range)
				continue;

			heat = v;
			bestKey = key;
			found = true;
		}

		if (found)
			pos = CellCenter(bestKey);

		return found;
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
