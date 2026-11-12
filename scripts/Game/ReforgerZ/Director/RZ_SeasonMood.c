//! The season mood (docs/WRITING.md, "The season mood"): one server-wide value every voice reads,
//! and the government broadcast's own table. It runs on stages and cure percent, not days, and adds
//! no simulation. Each faction's temper moves its own lines one step with its morale. Server only,
//! owned by the world facts (RZ_WorldFacts.SeasonMood), runtime only.

//------------------------------------------------------------------------------------------------
enum RZ_EMood
{
	EARLY,		// anything else: the default
	GRIM,		// Collapse or later, average infection 45+, or the 45-minute stall
	BLEAK,		// the long stall, or Overrun or Aftermath with the cure under 50%
	HOPE,		// the broadcast's breakthrough window
	EPILOGUE	// after the cure
}

//------------------------------------------------------------------------------------------------
class RZ_SeasonMood
{
	protected static const float CHECK_SECONDS = 300;		// a mood check every 5 minutes: two in a row (10) to change
	protected static const float HOLD_SECONDS = 3600;		// a mood holds at least 60 online minutes

	protected RZ_GameModeComponent m_GM;
	protected RZ_EMood m_eMood = RZ_EMood.EARLY;
	protected RZ_EMood m_ePending = RZ_EMood.EARLY;
	protected int m_iPendingChecks;
	protected float m_fCheckTimer = CHECK_SECONDS;
	protected float m_fHeld = HOLD_SECONDS;
	protected ref array<int> m_aTemper = {0, 0, 0, 0};	// -1 lighter, 0, +1 bleaker, per faction
	protected bool m_bStarted;

	//------------------------------------------------------------------------------------------------
	void RZ_SeasonMood(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! Online time only (the facts tick while someone plays).
	void Tick(float dt)
	{
		m_fHeld += dt;
		m_fCheckTimer += dt;
		if (m_fCheckTimer < CHECK_SECONDS)
			return;

		m_fCheckTimer = 0;
		UpdateTempers();
		RZ_EMood want = Ladder();
		RZ_VoiceSettings vs = RZ_VoiceSettings.Get();
		if (vs.m_iForceMood > 0)
			want = vs.m_iForceMood - 1;

		if (!m_bStarted)
		{
			m_bStarted = true;
			Set(want);
			return;
		}

		if (want == m_eMood)
		{
			m_iPendingChecks = 0;
			return;
		}

		// Hope and Epilogue start at once; the rest only after two checks in a row, and never
		// before the mood has held 60 online minutes
		if (want == RZ_EMood.HOPE || want == RZ_EMood.EPILOGUE || vs.m_iForceMood > 0)
		{
			Set(want);
			return;
		}

		if (want != m_ePending)
		{
			m_ePending = want;
			m_iPendingChecks = 1;
			return;
		}

		m_iPendingChecks++;
		if (m_iPendingChecks >= 2 && m_fHeld >= HOLD_SECONDS)
			Set(want);
	}

	//------------------------------------------------------------------------------------------------
	protected void Set(RZ_EMood mood)
	{
		if (mood != m_eMood)
			RZ_Log.Info(string.Format("Voice: season mood %1 (was %2)", Name(mood), Name(m_eMood)));

		m_eMood = mood;
		m_ePending = mood;
		m_iPendingChecks = 0;
		m_fHeld = 0;
	}

	//------------------------------------------------------------------------------------------------
	RZ_EMood Get()
	{
		return m_eMood;
	}

	//------------------------------------------------------------------------------------------------
	//! The ladder, first match wins: Epilogue, Hope, Bleak, Grim, Early. At the finale gate research
	//! stops by design: the stall doesn't count there, and the mood reads one step lighter.
	RZ_EMood Ladder()
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (!facts)
			return RZ_EMood.EARLY;

		if (facts.IsCured() || facts.IsEpilogue())
			return RZ_EMood.EPILOGUE;

		RZ_Broadcast broadcast = m_GM.GetBroadcast();
		if (broadcast && broadcast.InBreakthroughWindow())
			return RZ_EMood.HOPE;

		bool gate = facts.AtFinaleGate();
		float stall = 0;
		if (broadcast && !gate)
			stall = broadcast.StallSeconds();

		RZ_EOutbreakStage stage = facts.Stage();
		float cure = facts.CurePercent();
		RZ_EMood mood = RZ_EMood.EARLY;
		if (stall >= RZ_Broadcast.LONG_STALL_SECONDS || (stage >= RZ_EOutbreakStage.OVERRUN && cure < 50))
			mood = RZ_EMood.BLEAK;
		else if (stage >= RZ_EOutbreakStage.COLLAPSE || m_GM.GetDirector().GetAverageInfection() >= 45 || stall >= RZ_Broadcast.STALL_SECONDS)
			mood = RZ_EMood.GRIM;

		// Everyone can see the end from the gate
		if (gate && mood == RZ_EMood.GRIM)
			mood = RZ_EMood.EARLY;
		else if (gate && mood == RZ_EMood.BLEAK)
			mood = RZ_EMood.GRIM;

		return mood;
	}

	//------------------------------------------------------------------------------------------------
	//! Morale moves one faction one step, with gaps so it doesn't flicker: bleaker below 30 (back
	//! above 38), lighter above 75 (back below 67).
	protected void UpdateTempers()
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (!life || !life.IsActive())
			{
				m_aTemper[f] = 0;
				continue;
			}

			float morale = life.GetMorale(f);
			int t = m_aTemper[f];
			if (t == 1 && morale > 38)
				t = 0;
			else if (t == -1 && morale < 67)
				t = 0;

			if (morale < 30)
				t = 1;
			else if (morale > 75)
				t = -1;

			m_aTemper[f] = t;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The mood a faction's lines use: the season mood moved one step by its temper. Steps run
	//! Hope/Early, Grim, Bleak; the Epilogue isn't shifted.
	RZ_EMood For(int faction)
	{
		if (!RZ_Factions.IsValid(faction) || m_eMood == RZ_EMood.EPILOGUE)
			return m_eMood;

		int t = m_aTemper[faction];
		if (t > 0)
		{
			if (m_eMood == RZ_EMood.GRIM)
				return RZ_EMood.BLEAK;

			if (m_eMood == RZ_EMood.EARLY || m_eMood == RZ_EMood.HOPE)
				return RZ_EMood.GRIM;
		}
		else if (t < 0)
		{
			if (m_eMood == RZ_EMood.BLEAK)
				return RZ_EMood.GRIM;

			if (m_eMood == RZ_EMood.GRIM)
				return RZ_EMood.EARLY;
		}

		return m_eMood;
	}

	//------------------------------------------------------------------------------------------------
	//! One joke line in N: Early, Hope and the Epilogue 3, Grim 5, Bleak 10.
	static int JokeOneIn(RZ_EMood mood)
	{
		if (mood == RZ_EMood.BLEAK)
			return 10;

		if (mood == RZ_EMood.GRIM)
			return 5;

		return 3;
	}

	//------------------------------------------------------------------------------------------------
	static string Name(RZ_EMood mood)
	{
		switch (mood)
		{
			case RZ_EMood.GRIM: return "grim";
			case RZ_EMood.BLEAK: return "bleak";
			case RZ_EMood.HOPE: return "hope";
			case RZ_EMood.EPILOGUE: return "epilogue";
		}

		return "early";
	}

	//------------------------------------------------------------------------------------------------
	static int Bit(RZ_EMood mood)
	{
		return 1 << mood;
	}
}

//------------------------------------------------------------------------------------------------
//! Grief (docs/WRITING.md, "Grief"): after a death the server knows about, wry and joke lines are
//! skipped near it for a while, and the faction's radio goes plain. Runtime only: a restart forgets.
class RZ_ToneGuard
{
	protected static const int RING = 8;
	protected static const float FACE_RADIUS = 1500;
	protected static const float FACE_SECONDS = 600;
	protected static const float HOUSE_SECONDS = 1800;
	protected static const float RADIO_SECONDS = 1200;

	protected static ref array<vector> s_aPos = {};
	protected static ref array<float> s_aUntil = {};
	protected static ref array<float> s_aRadioUntil = {0, 0, 0, 0};

	//------------------------------------------------------------------------------------------------
	protected static float Now()
	{
		return RZ_UIW.Now();
	}

	//------------------------------------------------------------------------------------------------
	//! A death people would grieve: a leader, a wanderer, a named person, a truck crew, a Turned
	//! Player. `faction` (-1 none) goes quiet on its radio too. A death in a safehouse quiets that
	//! house's people for 30 minutes (house = true).
	static void Death(vector pos, int faction, bool house = false)
	{
		float seconds = FACE_SECONDS;
		if (house)
			seconds = HOUSE_SECONDS;

		if (s_aPos.Count() >= RING)
		{
			s_aPos.Remove(0);
			s_aUntil.Remove(0);
		}

		s_aPos.Insert(pos);
		s_aUntil.Insert(Now() + seconds);
		if (RZ_Factions.IsValid(faction))
			s_aRadioUntil[faction] = Now() + RADIO_SECONDS;
	}

	//------------------------------------------------------------------------------------------------
	//! A holding's named person killed (polled every 5 s from the reputation watch): grief like a
	//! leader's (1.5 km for 10 minutes, the faction's radio plain for 20), and a newcomer takes the
	//! role under a new name (RZ_Territory.ReplacePerson).
	static void NamedDeaths(RZ_GameModeComponent gm)
	{
		RZ_Errands errands = gm.GetErrands();
		if (!errands || !errands.GetPeople())
			return;

		array<string> keys = {};
		array<vector> at = {};
		errands.GetPeople().TakeDeadAtHoldings(keys, at);
		foreach (int i, string key : keys)
		{
			RZ_NamedPerson np = errands.GetPeople().Get(key);
			if (!np)
				continue;

			Death(at[i], np.m_iFaction);
			string dead = np.m_sName;
			if (gm.GetTerritory())
				dead = gm.GetTerritory().ReplacePerson(np.m_iPlace, np.m_iIndex);

			RZ_Log.Info(string.Format("Voice: grief for %1, %2 at %3", dead, np.RoleText(), np.m_sPlace));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A joke or wry line may be said face to face here now.
	static bool AllowsJoke(vector pos)
	{
		float now = Now();
		foreach (int i, vector p : s_aPos)
		{
			if (s_aUntil[i] > now && RZ_Util.DistXZ(p, pos) <= FACE_RADIUS)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's own radio may joke now.
	static bool RadioAllowsJoke(int faction)
	{
		if (!RZ_Factions.IsValid(faction))
			return true;

		return s_aRadioUntil[faction] <= Now();
	}
}
