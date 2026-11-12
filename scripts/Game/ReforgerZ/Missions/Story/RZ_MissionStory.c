//! The mission style's framework (docs/DESIGN.md, "Story blocks"):
//! a story kit any mission can carry (m_Story, null by default: a mission without one runs exactly as
//! before), who took part and who may decide, who benefits (RZ_Mission_Choice is built on it), truths
//! (the truth deck in RZ_WorldMemory), world records, and reasons: why a contract is offered now
//! (the reason lane, in RZ_MissionManager). No mission's content changes in this wave.
//! Server only.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Missions > Story.
[BaseContainerProps()]
class RZ_MissionStorySettings
{
	[Attribute("1", desc: "The story kit: who took part and who decides, who benefits (choice missions pay a switched side at most the base), truths and world records (docs/DESIGN.md, Story blocks)")]
	bool m_bEnabled;

	[Attribute("1", desc: "The reason lane: when the world posts a reason (a hungry house, a lost truck, a town gone bad), one contract that answers it may start near it, its brief opening with the reason's line. Inside the contract cap and every check")]
	bool m_bReasonLane;

	[Attribute("30", UIWidgets.Slider, "Reason lane gap: real minutes between two reason-started contracts, server-wide", "5 240 5")]
	float m_fReasonGapMinutes;

	[Attribute("3000", UIWidgets.Slider, "A reason-started contract must land within this many metres of the reason's place", "500 8000 100")]
	float m_fReasonRange;

	[Attribute("0.75", UIWidgets.Slider, "The one weight clamp: a contract's leader, faction-need and season multipliers together never go under this", "0.1 1 0.05")]
	float m_fWeightMin;

	[Attribute("1.3", UIWidgets.Slider, "...or over this", "1 3 0.05")]
	float m_fWeightMax;

	[Attribute("0.1", UIWidgets.Slider, "Mirror chance: The Cache and The Armory are also offered to a second giver this often (its people hear 5 minutes later, with a rough area twice as wide). 0 = never", "0 1 0.05")]
	float m_fMirrorChance;

	[Attribute("0", desc: "A PvE server: mirror offers are off (they turn into races between players)")]
	bool m_bPvE;

	[Attribute("0", desc: "Numbered scenes: a talk scene's line lists its options by number and its choice slots read Option 1 to Option 4 (the fallback if the replicated labels don't show in play)")]
	bool m_bNumberedScenes;

	protected static ref RZ_MissionStorySettings s_Defaults;

	//------------------------------------------------------------------------------------------------
	static RZ_MissionStorySettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetMissionStorySettings())
			return gm.GetMissionStorySettings();

		if (!s_Defaults)
			s_Defaults = RZ_MissionStorySettings.Cast(RZ_Util.NewWithDefaults("RZ_MissionStorySettings"));

		return s_Defaults;
	}
}

//------------------------------------------------------------------------------------------------
//! The story kit a mission may carry. Its key is saved in records: never rename it once shipped.
class RZ_MissionStory
{
	string m_sKey;
	ref RZ_Deciding m_Deciding = new RZ_Deciding();
	ref RZ_Beneficiaries m_Parties = new RZ_Beneficiaries();
	ref array<string> m_aReads = {};		// "key|subject": world records it holds while it runs
	ref array<string> m_aFound = {};		// "What you've found", for the journal
	string m_sTold;							// "What they told you": the brief as given
	ref array<int> m_aBlameFactions = {};	// BlameDecider's Infamy this job (for pay owed offline)
	ref array<int> m_aBlameInfamy = {};

	//------------------------------------------------------------------------------------------------
	void RZ_MissionStory(string key)
	{
		m_sKey = key;
	}

	//------------------------------------------------------------------------------------------------
	//! On every tick the mission runs (RZ_Mission.Update): who took part.
	void Update(RZ_Mission m, RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Deciding.Track(gm, m, dt, ids, players);
		int done = m_Deciding.TickHold(gm, m, dt);
		if (done >= 0)
			m.OnHoldDone(gm, m_Deciding.Holder(), done);
	}

	//------------------------------------------------------------------------------------------------
	//! A record this story reads: held (never evicted) until the mission ends.
	void Reads(string key, string subject)
	{
		RZ_WorldFacts.Get().Memory().Pin(key, subject, true);
		m_aReads.Insert(key + "|" + subject);
	}

	//------------------------------------------------------------------------------------------------
	//! The truth behind this job at `subject` (the deck: kept 3 season days, same for everyone). The
	//! first value is "as they said". With the mission, the leader who voices it leans the deck:
	//! Greedy and Paranoid givers hide more, Merciful and Idealist ones less, a Pragmatist says it
	//! straight, Ruthless and Veteran ones leave things out "by procedure" (at most +-50% on the
	//! hidden truths), and a Talker's secret leaks into the rumours.
	string Truth(string subject, notnull array<string> values, notnull array<float> weights, RZ_Mission m = null)
	{
		RZ_Leader giver = Voice(m);
		if (giver)
		{
			float hide = 1;
			if (giver.HasTrait(RZ_ELeaderTrait.GREEDY) || giver.HasTrait(RZ_ELeaderTrait.PARANOID))
				hide *= 1.5;

			if (giver.HasTrait(RZ_ELeaderTrait.MERCIFUL) || giver.HasTrait(RZ_ELeaderTrait.IDEALIST) || giver.HasTrait(RZ_ELeaderTrait.PRAGMATIST))
				hide *= 0.5;

			if (giver.HasTrait(RZ_ELeaderTrait.RUTHLESS) || giver.HasTrait(RZ_ELeaderTrait.VETERAN))
				hide *= 1.25;

			hide = Math.Clamp(hide, 0.5, 1.5);
			for (int i = 1; i < weights.Count(); i++)
			{
				weights[i] = weights[i] * hide;
			}
		}

		string truth = RZ_WorldFacts.Get().Memory().DrawTruth(m_sKey, subject, values, weights);
		if (giver && giver.HasTrait(RZ_ELeaderTrait.TALKER) && !values.IsEmpty() && truth != values[0] && RZ_WorldFacts.Get())
			RZ_WorldFacts.Get().AddRumour(string.Format("Somebody near %1's people has a loose mouth: the job they're putting about isn't what they say it is.", giver.m_sName), 12);

		return truth;
	}

	//------------------------------------------------------------------------------------------------
	//! The leader who voices this job: its own, else the one the manager will give it (the nearest of
	//! the giver's, as RZ_Leaders.AssignVoice does). Null for none.
	protected static RZ_Leader Voice(RZ_Mission m)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!m || !gm || !gm.GetLeaders() || !m.m_bHasGiver)
			return null;

		if (m.m_iLeaderId >= 0)
			return gm.GetLeaders().Get(m.m_iLeaderId);

		return gm.GetLeaders().FindNearest(m.m_Req.m_eGiver, m.m_vPos, 100000, true);
	}

	//------------------------------------------------------------------------------------------------
	//! A truth found (a clue, a background read, a talk check, a third party, a radio line): the
	//! finder and the room (30 m) get the line, and the journal's "What you've found" keeps it for
	//! everyone the job was offered to.
	void Reveal(RZ_GameModeComponent gm, RZ_Mission m, string subject, int playerId, string how, string line)
	{
		RZ_WorldFacts.Get().Memory().FoundTruth(m_sKey, subject, how);
		if (!m_aFound.Contains(line))
			m_aFound.Insert(line);

		RZ_Guidance.Report(playerId, "truth");

		IEntity finder = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!finder)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] == playerId || RZ_Util.DistXZ(p.GetOrigin(), finder.GetOrigin()) <= RZ_Deciding.ROOM)
				gm.NotifyPlayerAs(ids[i], m.m_sTitle, line, m.NoticeTag());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The mission ended: its reads are let go.
	void Cleanup()
	{
		foreach (string read : m_aReads)
		{
			int bar = read.IndexOf("|");
			if (bar > 0)
				RZ_WorldFacts.Get().Memory().Pin(read.Substring(0, bar), read.Substring(bar + 1, read.Length() - bar - 1), false);
		}

		m_aReads.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! Participation and deciding (shared by missions, dialogue scenes, errands, the epilogue's grey
//! choices and the finale): who took part (60 s within 150 m, or a step done), who may decide (took
//! part and was offered it), the 5-second hold announced to the room (30 m) that anyone eligible may
//! object to once per job, Stand with, and who is "with the decider".
class RZ_Deciding
{
	static const float PART_RANGE = 150;
	static const float PART_SECONDS = 60;
	static const float ROOM = 30;
	static const float HOLD_SECONDS = 5;

	protected ref map<string, float> m_mTime = new map<string, float>();	// identity -> seconds within 150 m
	protected ref array<string> m_aStepped = {};
	protected ref array<string> m_aObjected = {};
	protected ref array<string> m_aLeft = {};		// identities who chose Leave: out of the job, no pay, no blame
	protected ref array<string> m_aDirty = {};		// identities whose hands aren't clean here (Press, Trick, a switch, hiding something)
	protected ref array<int> m_aStoodWith = {};
	protected int m_iHolder = -1;
	protected int m_iHoldOption = -1;
	protected float m_fHoldLeft;
	protected vector m_vRoom;

	//------------------------------------------------------------------------------------------------
	void Track(RZ_GameModeComponent gm, RZ_Mission m, float dt, array<int> ids, array<IEntity> players)
	{
		if (m.m_bGlobal)
			return;

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m.m_vPos) > PART_RANGE)
				continue;

			string ident = gm.GetProfile(ids[i]).m_sIdentity;
			float t;
			m_mTime.Find(ident, t);
			m_mTime.Set(ident, t + dt);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player did one of the job's steps (picked up the item, talked to its person).
	void Step(RZ_GameModeComponent gm, int playerId)
	{
		string ident = gm.GetProfile(playerId).m_sIdentity;
		if (!m_aStepped.Contains(ident))
			m_aStepped.Insert(ident);
	}

	//------------------------------------------------------------------------------------------------
	bool TookPart(RZ_GameModeComponent gm, int playerId)
	{
		string ident = gm.GetProfile(playerId).m_sIdentity;
		if (m_aLeft.Contains(ident))
			return false;

		float t;
		m_mTime.Find(ident, t);
		return t >= PART_SECONDS || m_aStepped.Contains(ident);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone the job was offered to (eligible; anyone, for a world event) who took part.
	bool MayDecide(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		return m.IsEligible(gm, playerId) && TookPart(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! With the decider: the decider, their vanilla group within 150 m of `at`, and anyone who chose
	//! Stand with during the hold.
	void WithDecider(RZ_GameModeComponent gm, int decider, vector at, notnull array<int> result)
	{
		result.Insert(decider);
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			int other = ids[i];
			if (result.Contains(other))
				continue;

			if ((RZ_Util.SameGroup(decider, other) && RZ_Util.DistXZ(p.GetOrigin(), at) <= PART_RANGE) || m_aStoodWith.Contains(other))
				result.Insert(other);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Leave (one way, for you only): you stop taking part in this job, with no pay and no blame.
	void Leave(RZ_GameModeComponent gm, int playerId)
	{
		string ident = gm.GetProfile(playerId).m_sIdentity;
		if (!m_aLeft.Contains(ident))
			m_aLeft.Insert(ident);
	}

	//------------------------------------------------------------------------------------------------
	bool HasLeft(string ident)
	{
		return m_aLeft.Contains(ident);
	}

	//------------------------------------------------------------------------------------------------
	//! Before the job pays: whoever chose Leave is out of it.
	void DropLeft(RZ_GameModeComponent gm, notnull array<int> participants)
	{
		for (int i = participants.Count() - 1; i >= 0; i--)
		{
			RZ_PlayerProfile p = gm.GetProfile(participants[i]);
			if (p && m_aLeft.Contains(p.m_sIdentity))
				participants.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Amends need clean hands: Press, Trick, a switch or hiding something voids this job's Infamy
	//! work-off for that player.
	void Dirty(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (p && !m_aDirty.Contains(p.m_sIdentity))
			m_aDirty.Insert(p.m_sIdentity);
	}

	//------------------------------------------------------------------------------------------------
	bool IsDirty(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		return p && m_aDirty.Contains(p.m_sIdentity);
	}

	//------------------------------------------------------------------------------------------------
	bool IsDirtyIdent(string ident)
	{
		return m_aDirty.Contains(ident);
	}

	//------------------------------------------------------------------------------------------------
	//! The identities who took part long enough to be paid if they weren't here at the end: a step
	//! done, or `seconds` in the area. Leavers aren't among them.
	void TookPartIdents(float seconds, notnull array<string> result)
	{
		foreach (string ident, float t : m_mTime)
		{
			if (t >= seconds && !m_aLeft.Contains(ident) && !result.Contains(ident))
				result.Insert(ident);
		}

		foreach (string stepped : m_aStepped)
		{
			if (!m_aLeft.Contains(stepped) && !result.Contains(stepped))
				result.Insert(stepped);
		}
	}

	// ================================================================ The hold
	//------------------------------------------------------------------------------------------------
	//! A scene decision: a 5 s hold, announced to the room. False if one is already running.
	bool StartHold(RZ_GameModeComponent gm, RZ_Mission m, int playerId, int option, string what)
	{
		if (m_iHolder >= 0)
			return false;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me)
			return false;

		m_iHolder = playerId;
		m_iHoldOption = option;
		m_fHoldLeft = HOLD_SECONDS;
		m_vRoom = me.GetOrigin();
		m_aStoodWith.Clear();
		TellRoom(gm, m, string.Format("%1 is about to %2. You can object once, or stand with them.", RZ_Util.PlayerName(playerId), what));
		array<int> roomIds = {};
		array<IEntity> roomPlayers = {};
		RZ_Util.GetAlivePlayers(roomIds, roomPlayers);
		foreach (int r, IEntity rp : roomPlayers)
		{
			if (roomIds[r] != playerId && RZ_Util.DistXZ(rp.GetOrigin(), m_vRoom) <= ROOM)
				RZ_Guidance.Report(roomIds[r], "hold");
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Another eligible participant in the room objects, once per job by identity: the hold is
	//! cancelled. True if it was.
	bool Object(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (m_iHolder < 0 || playerId == m_iHolder || !MayDecide(gm, m, playerId))
			return false;

		string ident = gm.GetProfile(playerId).m_sIdentity;
		if (m_aObjected.Contains(ident))
			return false;

		m_aObjected.Insert(ident);
		TellRoom(gm, m, string.Format("%1 objects. Nothing is decided yet.", RZ_Util.PlayerName(playerId)));
		RZ_Log.Info(string.Format("Story: %1 hold by %2 objected to by %3", m.m_sTitle, RZ_Util.PlayerName(m_iHolder), RZ_Util.PlayerName(playerId)));
		m_iHolder = -1;
		m_iHoldOption = -1;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void StandWith(int playerId)
	{
		if (m_iHolder >= 0 && !m_aStoodWith.Contains(playerId))
			m_aStoodWith.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	int Holder()
	{
		return m_iHolder;
	}

	//------------------------------------------------------------------------------------------------
	//! The hold runs out: the option it decided, -1 while none.
	int TickHold(RZ_GameModeComponent gm, RZ_Mission m, float dt)
	{
		if (m_iHolder < 0)
			return -1;

		m_fHoldLeft -= dt;
		if (m_fHoldLeft > 0)
			return -1;

		int option = m_iHoldOption;
		m_iHoldOption = -1;
		return option;
	}

	//------------------------------------------------------------------------------------------------
	//! The hold is over (OnHoldDone ran): the holder is free.
	void EndHold()
	{
		m_iHolder = -1;
		m_iHoldOption = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected void TellRoom(RZ_GameModeComponent gm, RZ_Mission m, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vRoom) <= ROOM)
				gm.NotifyPlayerAs(ids[i], m.m_sTitle, text, m.NoticeTag());
		}
	}
}

//------------------------------------------------------------------------------------------------
//! One party a job's result can go to.
class RZ_Party
{
	int m_iFaction = -1;		// RZ_EFaction, -1 for nobody or a house
	string m_sName;
	int m_iRivalInfamy;			// Infamy with the giver for those with the decider, when it's a switch
	bool m_bNobody;				// pays nobody; a world record only
}

//------------------------------------------------------------------------------------------------
//! Who benefits (docs/DESIGN.md, "Story blocks"): 2-4 parties. The side you chose pays anyone with you whom
//! it doesn't hunt; a switch pays at most the base and its Fame doesn't count toward season Fame;
//! the giver adds Infamy and its leader doesn't count it as help; everyone else who did the work is
//! paid half the Favor by the giver, with no Fame and no Infamy. "Nobody" pays nobody.
class RZ_Beneficiaries
{
	ref array<ref RZ_Party> m_aParties = {};
	int m_iChosen = -1;
	int m_iDecider = -1;
	int m_iOriginal = -1;		// the giver the job was offered for
	ref array<int> m_aWith = {};
	ref array<int> m_aOthers = {};

	//------------------------------------------------------------------------------------------------
	RZ_Party Add(int faction, string name, int rivalInfamy)
	{
		RZ_Party p = new RZ_Party();
		p.m_iFaction = faction;
		p.m_sName = name;
		p.m_iRivalInfamy = rivalInfamy;
		m_aParties.Insert(p);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Party AddNobody(string name)
	{
		RZ_Party p = Add(-1, name, 0);
		p.m_bNobody = true;
		return p;
	}

	//------------------------------------------------------------------------------------------------
	int Find(int faction)
	{
		foreach (int i, RZ_Party p : m_aParties)
		{
			if (p.m_iFaction == faction)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSwitch()
	{
		if (m_iChosen < 0)
			return false;

		return m_aParties[m_iChosen].m_iFaction != m_iOriginal;
	}

	//------------------------------------------------------------------------------------------------
	//! The decider gave the result to party `idx` at `at`: the mission now pays that side (and
	//! sets the giver's Infamy as its rival when it's a switch); who is with the decider; who did the
	//! work otherwise. The participants become those with the decider whom the side doesn't hunt.
	void Choose(RZ_GameModeComponent gm, RZ_Mission m, int decider, int idx, vector at)
	{
		if (idx < 0 || idx >= m_aParties.Count())
			return;

		m_iChosen = idx;
		m_iDecider = decider;
		m_iOriginal = m.m_Req.m_eGiver;
		RZ_Party p = m_aParties[idx];
		RZ_MissionStory story = m.m_Story;

		m_aWith.Clear();
		story.m_Deciding.WithDecider(gm, decider, at, m_aWith);
		m_aOthers.Clear();
		foreach (int part : m.m_aParticipants)
		{
			if (!m_aWith.Contains(part) && story.m_Deciding.TookPart(gm, part) && m.IsEligible(gm, part))
				m_aOthers.Insert(part);
		}

		array<int> paid = {};
		if (!p.m_bNobody)
		{
			foreach (int w : m_aWith)
			{
				if (!RZ_Reputation.IsHunted(gm.GetTier(w, p.m_iFaction)))
					paid.Insert(w);
			}
		}

		m.m_aParticipants.Clear();
		m.m_aParticipants.InsertAll(paid);
		if (IsSwitch())
		{
			foreach (int dirty : m_aWith)
			{
				story.m_Deciding.Dirty(gm, dirty);
			}
		}

		if (p.m_bNobody)
		{
			m.m_bHasGiver = false;
		}
		else if (IsSwitch())
		{
			m.m_eGiver = p.m_iFaction;
			m.m_bNoLeaderCredit = true;
			m.m_iLeaderId = -1;
			m.m_bSwitched = true;
			m.SetRivalPublic(m_iOriginal, p.m_iRivalInfamy);
		}

		RZ_StoryStats.Picked(story.m_sKey, IsSwitch(), p.m_bNobody);
		string subject = RZ_WorldFacts.Town(m.m_iSettlement);
		RZ_WorldFacts.Get().Memory().Record(story.m_sKey + ".went_to", subject, p.m_sName, RZ_WorldMemory.SOURCE_CHOICE, false, m_aWith);
		RZ_Log.Info(string.Format("Story: %1 resolved by hand-in to %2 (base Fame %3, Favor %4; with %5, others %6; switch %7)", story.m_sKey, p.m_sName,
			m.m_iFameReward, m.m_iFavorReward, m_aWith.Count(), m_aOthers.Count(), IsSwitch()));
	}

	//------------------------------------------------------------------------------------------------
	//! Everyone else who did the work, when the result went another way: half the Favor from the giver,
	//! no Fame, no Infamy.
	void PayOthers(RZ_GameModeComponent gm, RZ_Mission m)
	{
		if (m_iChosen < 0 || !IsSwitch() || !RZ_Factions.IsValid(m_iOriginal))
			return;

		int favor = Math.Max(1, m.m_iFavorReward / 2);
		foreach (int other : m_aOthers)
		{
			gm.AddFavor(other, m_iOriginal, favor);
			gm.NotifyPlayerAs(other, m.m_sTitle, string.Format("\"You held it. Someone else sold it.\" +%1 %2 Favor for the work.", favor, RZ_Factions.GetName(m_iOriginal)), RZ_ENoticeTag.MISSION);
		}
	}
}
