//! The behaviour layer's hearing and scent beyond shots (docs/INFECTED.md, "Hearing, blood and
//! shrieks"). A sound gives a steered group a lead: a spot 15% of the distance off, walked to look
//! (walk-stop), never a fix on you. Players' own movement, vehicles, every noise the heat map hears
//! (doors, searches, bangs, bells, noisemakers: the modded RZ_HeatMap below), a bleeding player's
//! trail, and a young's shriek passed to its own pack. Server only, on the behaviour layer's
//! second tick; only groups the layer drives (a tribe's) take leads.

//------------------------------------------------------------------------------------------------
//! Every heat the map is given is also a sound the behaviour layer hears.
modded class RZ_HeatMap
{
	//------------------------------------------------------------------------------------------------
	override void AddHeat(vector pos, float amount)
	{
		super.AddHeat(pos, amount);
		RZ_Behaviour bh = RZ_Behaviour.Get();
		if (bh && amount > 0)
			bh.GetHearing().OnHeat(pos, amount);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Hearing
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Behaviour m_BH;
	protected ref map<int, vector> m_mLastPos = new map<int, vector>();
	protected ref map<int, float> m_mVehicleAt = new map<int, float>();
	// Blood: each bleeding player's last 6 drops, one every 10 s, kept 60 s after the bleeding stops
	protected ref map<int, ref array<vector>> m_mTrail = new map<int, ref array<vector>>();
	protected ref map<int, float> m_mBledAt = new map<int, float>();
	protected float m_fBloodTimer;
	// Recent heat sounds, so a stream of small heat isn't heard as many sounds
	protected ref array<vector> m_aHeard = {};
	protected ref array<float> m_aHeardAt = {};
	protected ref array<float> m_aHeardRadius = {};
	protected ref map<int, float> m_mShriekToldAt = new map<int, float>();
	protected int m_iLeads;
	protected int m_iBloodLeads;
	protected int m_iShrieks;

	static const float SPRINT_SOUND = 30;
	static const float RUN_SOUND = 20;
	static const float WALK_SOUND = 8;
	static const float CROUCH_SOUND = 3;
	static const float VEHICLE_SOUND = 200;
	static const float DOOR_SOUND = 40;			// a door, a forced lock, a search: small heat at once
	static const float NOISE_PER_HEAT = 10;		// remembered noises (8+ heat at once): heat x 10 m
	static const float NOISE_MAX = 400;
	static const float BLOOD_ERROR = 20;
	static const float SHRIEK_DAY = 200;
	static const float SHRIEK_NIGHT = 300;
	static const float SHRIEK_GAP = 30;			// s per group
	static const float SHRIEK_TOLD = 300;		// m: players this near hear it
	static const float SHRIEK_LINE_GAP = 90;	// s per player

	//------------------------------------------------------------------------------------------------
	void RZ_Hearing(RZ_GameModeComponent gm, RZ_Behaviour bh)
	{
		m_GM = gm;
		m_BH = bh;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second: each player's footsteps or engine, and every 10 s the blood.
	void Tick(array<int> ids, array<IEntity> players)
	{
		float now = m_BH.Now();
		foreach (int i, IEntity p : players)
		{
			int id = ids[i];
			vector at = p.GetOrigin();
			vector was;
			bool had = m_mLastPos.Find(id, was);
			m_mLastPos.Set(id, at);
			if (!had)
				continue;

			float speed = RZ_Util.DistXZ(was, at);
			if (RZ_Util.IsInVehicle(p))
			{
				float last;
				if (speed > 2 && (!m_mVehicleAt.Find(id, last) || now - last >= 5))
				{
					m_mVehicleAt.Set(id, now);
					Sound(at, VEHICLE_SOUND);
				}

				continue;
			}

			float radius = FootSound(p, speed);
			if (radius > 0)
				Sound(at, radius);
		}

		m_fBloodTimer += 1;
		if (m_fBloodTimer >= 10)
		{
			m_fBloodTimer = 0;
			Blood(ids, players, now);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sprinting, running, walking, crouched: 30, 20, 8, 3 m; still or prone, nothing.
	protected static float FootSound(IEntity p, float speed)
	{
		if (speed < 0.5)
			return 0;

		int stance = RZ_Senses.GetStance(p);
		if (stance >= 2)
			return 0;

		if (stance == 1)
			return CROUCH_SOUND;

		if (speed > 4.5)
			return SPRINT_SOUND;

		if (speed > 2.5)
			return RUN_SOUND;

		return WALK_SOUND;
	}

	//------------------------------------------------------------------------------------------------
	//! A heat the map was given (modded RZ_HeatMap): 8 or more at once is a remembered noise (heat x 10 m,
	//! at most 400 m: bells, noisemakers, sirens, a banged door); 2 to 8 is a door, a forced lock or a
	//! search (40 m). Less, and the trickles (presence, a generator), aren't sounds.
	void OnHeat(vector at, float amount)
	{
		if (!RZ_Behaviour.IsOn() || amount < 2)
			return;

		float radius = DOOR_SOUND;
		if (amount >= 8)
			radius = Math.Min(NOISE_MAX, amount * NOISE_PER_HEAT);

		float now = m_BH.Now();
		for (int i = m_aHeard.Count() - 1; i >= 0; i--)
		{
			if (now - m_aHeardAt[i] > 10)
			{
				m_aHeard.Remove(i);
				m_aHeardAt.Remove(i);
				m_aHeardRadius.Remove(i);
				continue;
			}

			if (RZ_Util.DistXZ(m_aHeard[i], at) < 25 && m_aHeardRadius[i] >= radius)
				return;
		}

		m_aHeard.Insert(at);
		m_aHeardAt.Insert(now);
		m_aHeardRadius.Insert(radius);
		Sound(at, radius);
	}

	//------------------------------------------------------------------------------------------------
	//! A sound at `at` heard from `radius` (times each group's kind and habit): a lead with 15% of
	//! the distance as error.
	void Sound(vector at, float radius)
	{
		if (!RZ_Behaviour.IsOn())
			return;

		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Plan || !z.m_Entity || !z.m_Plan.IsDriven())
				continue;

			float d = RZ_Util.DistXZ(z.m_vLastPos, at);
			if (d > radius * 1.6)
				continue;

			if (d > z.m_Plan.HearingRange(m_BH, radius))
				continue;

			if (z.m_Plan.TakeLead(m_BH, at, d * 0.15 + 2))
				m_iLeads++;
		}
	}

	// ================================================================ Blood
	//------------------------------------------------------------------------------------------------
	//! Every 10 s: a bleeding player drops a point of scent (the last 6 kept, 60 s after the bleeding
	//! stops), and a group within its kind's blood radius of the newest point gets a lead there
	//! (+-20 m). It pulls them along your trail; it never gives them you.
	protected void Blood(array<int> ids, array<IEntity> players, float now)
	{
		foreach (int i, IEntity p : players)
		{
			int id = ids[i];
			if (!IsBleeding(p) || RZ_Util.IsInVehicle(p))
				continue;

			m_mBledAt.Set(id, now);
			array<vector> trail = m_mTrail.Get(id);
			if (!trail)
			{
				trail = {};
				m_mTrail.Set(id, trail);
			}

			trail.Insert(p.GetOrigin());
			while (trail.Count() > 6)
			{
				trail.RemoveOrdered(0);
			}
		}

		array<int> gone = {};
		foreach (int pid, array<vector> drops : m_mTrail)
		{
			float bled = m_mBledAt.Get(pid);
			if (now - bled > 60 || drops.IsEmpty() || ids.Find(pid) < 0)
			{
				gone.Insert(pid);
				continue;
			}

			if (RZ_Behaviour.IsOn())
				Scent(drops[drops.Count() - 1]);
		}

		foreach (int g : gone)
		{
			m_mTrail.Remove(g);
			m_mBledAt.Remove(g);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Scent(vector at)
	{
		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Plan || !z.m_Entity || !z.m_Plan.IsDriven())
				continue;

			float d = RZ_Util.DistXZ(z.m_vLastPos, at);
			if (d > z.m_Plan.BloodRange(m_BH))
				continue;

			if (z.m_Plan.TakeLead(m_BH, at, BLOOD_ERROR))
				m_iBloodLeads++;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsBleeding(IEntity p)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(p);
		if (!c)
			return false;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		return dmg && dmg.IsBleeding();
	}

	// ================================================================ Shrieks
	//------------------------------------------------------------------------------------------------
	//! A group became aware of a player (its notice meter filled): with young in it, half the time,
	//! one of them shrieks. Its own pack's groups within 200 m by day (300 at night) get a lead at the
	//! shrieker (+-20 m), and a group shrieks at most once in 30 s; a shriek heard is never passed on,
	//! so it never chains across the map. Players within 300 m hear it (one line in 90 s).
	void MaybeShriek(RZ_GroupPlan plan, IEntity body, int tribe)
	{
		if (!RZ_Behaviour.IsOn() || !plan || !body || tribe < 0)
			return;

		float now = m_BH.Now();
		if (now - plan.m_fShriekAt < SHRIEK_GAP || Math.RandomFloat01() >= 0.5)
			return;

		plan.m_fShriekAt = now;
		m_iShrieks++;
		RZ_BaconBody.Say(body, "SOUND_TARGET");
		vector at = body.GetOrigin();
		float radius = SHRIEK_DAY;
		if (m_BH.IsNight())
			radius = SHRIEK_NIGHT;

		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Plan || z.m_Plan == plan || z.m_iTribe != tribe || !z.m_Plan.IsDriven())
				continue;

			if (RZ_Util.DistXZ(z.m_vLastPos, at) <= radius && z.m_Plan.TakeLead(m_BH, at, BLOOD_ERROR))
				m_iLeads++;
		}

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), at) > SHRIEK_TOLD)
				continue;

			int id = ids[i];
			float told;
			if (m_mShriekToldAt.Find(id, told) && now - told < SHRIEK_LINE_GAP)
				continue;

			m_mShriekToldAt.Set(id, now);
			m_GM.NotifyPlayer(id, "A shriek", "A shriek, close by, and an answer from further off. They know.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "sound leads 12, blood leads 2, shrieks 1"
	string StatusText()
	{
		return string.Format("sound leads %1, blood leads %2, shrieks %3", m_iLeads, m_iBloodLeads, m_iShrieks);
	}
}
