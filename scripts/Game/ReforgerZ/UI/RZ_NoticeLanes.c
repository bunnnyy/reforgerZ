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
//! Notices, one per approach: the gate's own line first (a gate bark, with a rival-colours remark
//! folded in), then a change notice, then a tip; anything lower waits until 30 s after the last
//! approach line, and is dropped once it has waited past 2 minutes.
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
//! What waits on an approach, in order: a change notice, then a tip, then anything lower.
enum RZ_EApproachRank
{
	CHANGE = 1,
	TIP = 2,
	LOWER = 3
}

//------------------------------------------------------------------------------------------------
class RZ_NoticeLanes
{
	//! An approach line holds the rest this long, and a held line older than the drop is dropped.
	static const float APPROACH_GAP = 30;
	static const float APPROACH_DROP = 120;

	//! Errands wait this much longer than the gap, so any other contact due at the same time wins.
	protected static const float ERRAND_EXTRA = 60;

	protected RZ_SharedSettings m_Settings;
	protected ref map<int, float> m_mLastContact = new map<int, float>();	// playerId -> clock
	protected ref map<int, float> m_mLastBark = new map<int, float>();
	protected ref map<string, float> m_mSpeakerBark = new map<string, float>();	// "playerId|speaker" -> clock
	protected ref set<string> m_aPlaces = new set<string>();						// "playerId|place", this session
	protected ref map<int, float> m_mApproachAt = new map<int, float>();			// playerId -> clock of the last approach line
	protected ref map<int, float> m_mChangeWaits = new map<int, float>();			// playerId -> clock a change notice began waiting
	protected ref map<string, float> m_mHeldSince = new map<string, float>();		// "playerId|place" -> clock first held

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

		// Lower than a tip: waits out the approach, dropped once it has waited past 2 minutes
		if (ApproachHolds(playerId, RZ_EApproachRank.LOWER))
		{
			float since;
			if (!m_mHeldSince.Find(key, since))
			{
				m_mHeldSince.Set(key, Now());
				return false;
			}

			if (Now() - since > APPROACH_DROP)
			{
				m_aPlaces.Insert(key);
				m_mHeldSince.Remove(key);
			}

			return false;
		}

		m_mHeldSince.Remove(key);
		m_aPlaces.Insert(key);
		return true;
	}

	// ================================================================ One per approach
	//------------------------------------------------------------------------------------------------
	//! The gate's own line (a gate bark at a place): once per place per session like an ambient line,
	//! never held, and it starts the approach.
	bool TryGate(int playerId, string place)
	{
		if (!m_Settings.m_bLanes || playerId <= 0)
			return true;

		string key = string.Format("%1|%2", playerId, place);
		if (m_aPlaces.Contains(key))
			return false;

		m_aPlaces.Insert(key);
		m_mApproachAt.Set(playerId, Now());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Has this place's once-a-session line been said to this player? Doesn't use it.
	bool PlaceUsed(int playerId, string place)
	{
		if (!m_Settings.m_bLanes || playerId <= 0)
			return false;

		return m_aPlaces.Contains(string.Format("%1|%2", playerId, place));
	}

	//------------------------------------------------------------------------------------------------
	//! Must a line of `rank` wait for the approach? Within 30 s of the last approach line, yes; and
	//! a tip or lower also waits while a change notice does.
	bool ApproachHolds(int playerId, RZ_EApproachRank rank)
	{
		if (!m_Settings.m_bLanes || playerId <= 0)
			return false;

		float last;
		if (m_mApproachAt.Find(playerId, last) && Now() - last < APPROACH_GAP)
			return true;

		if (rank > RZ_EApproachRank.CHANGE)
		{
			float waits;
			if (m_mChangeWaits.Find(playerId, waits) && Now() - waits <= APPROACH_DROP)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A held line of `rank` is now said: inside an approach (the last approach line under 2 minutes
	//! ago), it is the approach's next line, and the next waits 30 s after it.
	void NoteApproachLine(int playerId, RZ_EApproachRank rank)
	{
		if (rank == RZ_EApproachRank.CHANGE)
			m_mChangeWaits.Remove(playerId);

		float last;
		if (m_mApproachAt.Find(playerId, last) && Now() - last <= APPROACH_DROP)
			m_mApproachAt.Set(playerId, Now());
	}

	//------------------------------------------------------------------------------------------------
	//! A change notice is waiting on the approach (tips wait behind it).
	void NoteChangeWaiting(int playerId)
	{
		if (playerId > 0 && !m_mChangeWaits.Contains(playerId))
			m_mChangeWaits.Set(playerId, Now());
	}

	//------------------------------------------------------------------------------------------------
	//! The player left: their session's lanes go (ids aren't reused).
	void Forget(int playerId)
	{
		m_mLastContact.Remove(playerId);
		m_mLastBark.Remove(playerId);
		m_mApproachAt.Remove(playerId);
		m_mChangeWaits.Remove(playerId);
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

		foreach (string hk, float ht : m_mHeldSince)
		{
			if (hk.IndexOf(prefix) == 0)
				drop.Insert(hk);
		}

		foreach (string d : drop)
		{
			m_aPlaces.RemoveItem(d);
			m_mSpeakerBark.Remove(d);
			m_mHeldSince.Remove(d);
		}
	}
}
