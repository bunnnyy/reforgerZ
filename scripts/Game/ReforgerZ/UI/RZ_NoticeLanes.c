//! Notice lanes and the interruption budget (docs/DESIGN.md, "Notice lanes"), per player, on the
//! server. They sit on top of the built notice pacing (the client's cards by importance, docs/UI.md
//! "Notices"): pacing decides how a notice is shown, the lanes decide whether something that
//! speaks unasked may speak at all.
//!
//! | Lane     | Rule                                                                              |
//! | Critical | always shown (urgent notices: bites, the final stand)                             |
//! | Talk     | owner-only, at once (the talk screen, answers to what you asked)                  |
//! | Contact  | unasked contacts: at most one per Contact gap (4 min) across radio offers, leader |
//! |          | pings, wanderers, the Long Table's host and errands, with errands last            |
//! | Bark     | one a minute, one per speaker per 10 minutes                                      |
//! | Ambient  | approach and ambient lines: once per place per session                            |
//!
//! The budget is a ceiling only: it can stop a contact, never start one. A caller asks Try*()
//! when it would speak, and stays quiet (to try again later) on false.
//! Server only; runtime only (a restart resets it).

//------------------------------------------------------------------------------------------------
enum RZ_ENoticeLane
{
	CRITICAL,
	TALK,
	CONTACT,
	BARK,
	AMBIENT
}

//------------------------------------------------------------------------------------------------
//! Who makes an unasked contact.
enum RZ_EContact
{
	RADIO_OFFER,
	LEADER,
	WANDERER,
	HOST,		// the Long Table's host on the road
	ERRAND		// errands come last
}

//------------------------------------------------------------------------------------------------
class RZ_NoticeLanes
{
	//! Errands wait this much longer than the gap, so any other contact due at the same time wins.
	protected static const float ERRAND_EXTRA = 60;

	protected RZ_SharedSettings m_Settings;
	protected ref map<int, float> m_mLastContact = new map<int, float>();	// playerId -> clock
	protected ref map<int, float> m_mLastBark = new map<int, float>();
	protected ref map<string, float> m_mSpeakerBark = new map<string, float>();	// "playerId|speaker" -> clock
	protected ref set<string> m_aPlaces = new set<string>();						// "playerId|place", this session

	//------------------------------------------------------------------------------------------------
	void RZ_NoticeLanes(RZ_SharedSettings settings)
	{
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! The lanes, null without a game mode (callers then just speak, as before).
	static RZ_NoticeLanes Get()
	{
		RZ_Shared shared = RZ_Shared.Get();
		if (!shared)
			return null;

		return shared.GetLanes();
	}

	//------------------------------------------------------------------------------------------------
	//! The lane a notice tag belongs to: urgent is Critical; everything else that's sent as a notice
	//! is paced by the client and isn't held here.
	static RZ_ENoticeLane LaneOf(RZ_ENoticeTag tag)
	{
		if (tag == RZ_ENoticeTag.URGENT)
			return RZ_ENoticeLane.CRITICAL;

		return RZ_ENoticeLane.TALK;
	}

	//------------------------------------------------------------------------------------------------
	protected static float Now()
	{
		return RZ_SeasonClock.Now();
	}

	//------------------------------------------------------------------------------------------------
	//! May an unasked contact of `kind` reach this player now? Doesn't use the budget.
	bool MayContact(int playerId, RZ_EContact kind)
	{
		if (!m_Settings.m_bLanes || playerId <= 0)
			return true;

		float gap = Math.Max(0, m_Settings.m_fContactGapMinutes) * 60;
		if (kind == RZ_EContact.ERRAND)
			gap += ERRAND_EXTRA;

		float last;
		if (!m_mLastContact.Find(playerId, last))
			return true;

		return Now() - last >= gap;
	}

	//------------------------------------------------------------------------------------------------
	//! An unasked contact reached this player: the next waits a gap.
	void NoteContact(int playerId, RZ_EContact kind)
	{
		if (playerId > 0)
			m_mLastContact.Set(playerId, Now());
	}

	//------------------------------------------------------------------------------------------------
	//! MayContact, and if so NoteContact. True: speak now.
	bool TryContact(int playerId, RZ_EContact kind)
	{
		if (!MayContact(playerId, kind))
			return false;

		NoteContact(playerId, kind);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A bark from `speaker` (a stable key) at this player: one a minute, one per speaker per 10.
	bool TryBark(int playerId, string speaker)
	{
		if (!m_Settings.m_bLanes || playerId <= 0)
			return true;

		float now = Now();
		float last;
		if (m_mLastBark.Find(playerId, last) && now - last < m_Settings.m_fBarkGapSeconds)
			return false;

		string key = string.Format("%1|%2", playerId, speaker);
		if (m_mSpeakerBark.Find(key, last) && now - last < m_Settings.m_fBarkSpeakerMinutes * 60)
			return false;

		m_mLastBark.Set(playerId, now);
		m_mSpeakerBark.Set(key, now);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! An approach or ambient line at `place` (a stable key): once per place per session.
	bool TryAmbient(int playerId, string place)
	{
		if (!m_Settings.m_bLanes || playerId <= 0)
			return true;

		string key = string.Format("%1|%2", playerId, place);
		if (m_aPlaces.Contains(key))
			return false;

		m_aPlaces.Insert(key);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The player left: their session's lanes go (ids aren't reused).
	void Forget(int playerId)
	{
		m_mLastContact.Remove(playerId);
		m_mLastBark.Remove(playerId);
		string prefix = playerId.ToString() + "|";
		array<string> drop = {};
		foreach (string k : m_aPlaces)
		{
			if (k.IndexOf(prefix) == 0)
				drop.Insert(k);
		}

		foreach (string sk, float t : m_mSpeakerBark)
		{
			if (sk.IndexOf(prefix) == 0)
				drop.Insert(sk);
		}

		foreach (string d : drop)
		{
			m_aPlaces.RemoveItem(d);
			m_mSpeakerBark.Remove(d);
		}
	}
}
