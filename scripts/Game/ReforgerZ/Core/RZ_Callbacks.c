//! The callback queue (docs/DESIGN.md, "Callbacks"): one queue for missions' echoes, errands'
//! follow-ups and dialogue's "later". A callback is words only: a line that comes back to a
//! character some hours of their play later, while they're near its subject.
//!
//! - Per character: 10 waiting per player (Shared > Callbacks waiting per player), due 2-6 hours of
//!   that life's play after it was queued, delivered within 1 km of the subject, at most one per
//!   hour of play. It belongs to the life it was made in.
//! - The system pool (100): callbacks that belong to nobody, due after a number of season days,
//!   told to the first player who comes near the subject.
//! - Each expires after 3 season days (24 real hours in the epilogue week).
//! - At death a waiting one with a board line becomes an unsigned note on the nearest safehouse
//!   board ("For the man who brought the Dvoraks in, if anyone sees him").
//! No check bonuses, no discounts, no gifts. Saved with the season.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_Callback
{
	string ident;		// "" = the system pool
	int life;			// the profile's life number it belongs to (m_iLives)
	string title;		// the speaker or the card's title
	string text;
	string boardNote;	// at death: this goes on the nearest board, unsigned ("" = nothing)
	float x;
	float z;
	float dueSeconds;	// that life's played seconds (m_fLifeSeconds) when it may come; the pool: season days
	float madeDay;		// season days when queued
	int madeUnix;		// real time when queued (the epilogue's 24 hours)
}

//------------------------------------------------------------------------------------------------
class RZ_CallbackSave
{
	ref array<ref RZ_Callback> list = {};
}

//------------------------------------------------------------------------------------------------
class RZ_Callbacks
{
	protected static const float CHECK_SECONDS = 10;
	protected static const float MIN_HOURS = 2;
	protected static const float MAX_HOURS = 6;

	protected RZ_GameModeComponent m_GM;
	protected RZ_SharedSettings m_Settings;
	protected ref array<ref RZ_Callback> m_aList = {};
	protected ref map<string, float> m_mLastAt = new map<string, float>();	// ident -> that life's played seconds at the last one
	protected float m_fTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_Callbacks(RZ_GameModeComponent gm, RZ_SharedSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! Queue a callback for this player's character: `text` from `title`, near `subject`, after
	//! `hours` of their play (< 0: a random 2 to 6). boardNote: what goes on a board if the
	//! character dies first. False when callbacks are off or the player's queue is full.
	bool Queue(int playerId, string title, string text, vector subject, float hours = -1, string boardNote = "")
	{
		if (!m_Settings.m_bCallbacks || playerId <= 0 || text.IsEmpty())
			return false;

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p.m_sIdentity.IsEmpty() || CountFor(p.m_sIdentity) >= m_Settings.m_iCallbacksPerPlayer)
			return false;

		if (hours < 0)
			hours = Math.RandomFloat(MIN_HOURS, MAX_HOURS);

		RZ_Callback c = Make(title, text, subject, boardNote);
		c.ident = p.m_sIdentity;
		c.life = p.m_iLives;
		c.dueSeconds = p.m_fLifeSeconds + hours * 3600;
		m_aList.Insert(c);
		RZ_Log.Info(string.Format("Callbacks: queued for %1 in %2 h of play: %3", RZ_Util.PlayerName(playerId), hours, title));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Queue a callback that belongs to nobody (the system pool): told to the first player near
	//! `subject` once `days` season days have passed. False when off or the pool is full.
	bool QueueWorld(string title, string text, vector subject, float days, string boardNote = "")
	{
		if (!m_Settings.m_bCallbacks || text.IsEmpty() || CountFor("") >= m_Settings.m_iCallbackPool)
			return false;

		RZ_Callback c = Make(title, text, subject, boardNote);
		c.dueSeconds = c.madeDay + Math.Max(0, days);
		m_aList.Insert(c);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Callback Make(string title, string text, vector subject, string boardNote)
	{
		RZ_Callback c = new RZ_Callback();
		c.title = title;
		c.text = text;
		c.boardNote = boardNote;
		c.x = subject[0];
		c.z = subject[2];
		c.madeDay = RZ_SeasonClock.Days();
		c.madeUnix = RZ_SeasonClock.Now();
		return c;
	}

	//------------------------------------------------------------------------------------------------
	int CountFor(string ident)
	{
		int n = 0;
		foreach (RZ_Callback c : m_aList)
		{
			if (c.ident == ident)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aList.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Every few seconds: expire, then tell what's due to players near its subject.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fTimer += dt;
		if (m_fTimer < CHECK_SECONDS)
			return;

		m_fTimer = 0;
		RZ_DoorCallers.Tick(CHECK_SECONDS);
		Expire();
		if (m_aList.IsEmpty() || ids.IsEmpty())
			return;

		float today = RZ_SeasonClock.Days();
		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			RZ_PlayerProfile p = m_GM.GetProfile(playerId);
			if (p.m_sIdentity.IsEmpty())
				continue;

			float last;
			if (m_mLastAt.Find(p.m_sIdentity, last) && p.m_fLifeSeconds >= last && p.m_fLifeSeconds - last < m_Settings.m_fCallbackGapHours * 3600)
				continue;

			int pick = -1;
			foreach (int ci, RZ_Callback c : m_aList)
			{
				if (RZ_Util.DistXZ(ent.GetOrigin(), Vector(c.x, 0, c.z)) > m_Settings.m_fCallbackRadius)
					continue;

				bool mine = c.ident == p.m_sIdentity && c.life == p.m_iLives && p.m_fLifeSeconds >= c.dueSeconds;
				bool world = c.ident.IsEmpty() && today >= c.dueSeconds;
				if (mine || world)
				{
					pick = ci;
					break;
				}
			}

			if (pick < 0)
				continue;

			RZ_Callback due = m_aList[pick];
			m_aList.Remove(pick);
			m_mLastAt.Set(p.m_sIdentity, p.m_fLifeSeconds);
			// A story job's callback at the door: a stand-in says it (Missions/Story/RZ_DoorCallers.c)
			string plain;
			if (RZ_DoorCallers.Strip(due.title, plain))
				RZ_DoorCallers.Say(m_GM, playerId, plain, due.text, Vector(due.x, 0, due.z));
			else
				m_GM.NotifyPlayerAs(playerId, due.title, due.text, RZ_ENoticeTag.AUTO);

			RZ_Log.Info(string.Format("Callbacks: told %1: %2", RZ_Util.PlayerName(playerId), plain));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! After 3 season days (24 real hours in the epilogue) a waiting callback goes; one with a board
	//! line leaves its note.
	protected void Expire()
	{
		float today = RZ_SeasonClock.Days();
		int now = RZ_SeasonClock.Now();
		bool epilogue = RZ_SeasonClock.IsEpilogue();
		for (int i = m_aList.Count() - 1; i >= 0; i--)
		{
			RZ_Callback c = m_aList[i];
			bool old = today - c.madeDay > m_Settings.m_fCallbackDays;
			if (epilogue)
				old = now - c.madeUnix > m_Settings.m_fEpilogueCallbackHours * 3600;

			if (!old)
				continue;

			LeaveNote(c);
			m_aList.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The character `ident` died (its life `life` ended): its waiting callbacks become board notes.
	void OnLifeEnded(string ident, int life)
	{
		if (ident.IsEmpty())
			return;

		for (int i = m_aList.Count() - 1; i >= 0; i--)
		{
			RZ_Callback c = m_aList[i];
			if (c.ident != ident || c.life != life)
				continue;

			LeaveNote(c);
			m_aList.Remove(i);
		}

		m_mLastAt.Remove(ident);
	}

	//------------------------------------------------------------------------------------------------
	protected void LeaveNote(RZ_Callback c)
	{
		if (c.boardNote.IsEmpty() || !m_GM.GetSafehouses() || !m_GM.GetSafehouses().GetBoard())
			return;

		m_GM.GetSafehouses().GetBoard().PostUnsigned(Vector(c.x, 0, c.z), c.boardNote);
	}

	//------------------------------------------------------------------------------------------------
	RZ_CallbackSave Save()
	{
		RZ_CallbackSave data = new RZ_CallbackSave();
		foreach (RZ_Callback c : m_aList)
		{
			data.list.Insert(c);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_CallbackSave data)
	{
		if (!data || !data.list)
			return;

		foreach (RZ_Callback c : data.list)
		{
			if (c)
				m_aList.Insert(c);
		}
	}
}
