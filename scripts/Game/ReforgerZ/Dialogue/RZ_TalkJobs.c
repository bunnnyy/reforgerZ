//! Jobs on the first replies (docs/DIALOGUE.md, "Jobs on the first replies"): a giver who has work
//! shows it straight away on their hub, one reply a job, tagged by its kind ("[Job] Recover the
//! Bus", "[Quest] The Other Two: his number"). Picking one has them explain it (the pitch: what,
//! where, the pay, the danger), then "I'll do it." takes it at once (your Quests tab and the start
//! card, RZ_JobCards). A job you can't take yet still shows, locked, with what it needs ("Needs Known
//! with Halcyon"), from the job's own start checks (RZ_Mission.LockReason). With nothing at all, one
//! locked "Got any work for me?" says why. Used by a leader (RZ_TalkLeader) and a quartermaster
//! (RZ_TalkStaff). Server only.
class RZ_TalkJobs
{
	static const int BASE = 10;		// 10-19: the jobs listed, in order
	static const int MAX = 10;
	static const int NOTHING = 20;	// the locked "Got any work for me?" when there's nothing to list
	static const int TAKE = 6;		// "I'll do it.": the pitched job taken
	static const int LEAVE = 7;		// "Not today.": left with them

	//------------------------------------------------------------------------------------------------
	//! "[Job]", "[Quest]", "[Errand]": the tag in front of a reply that starts a job, or opens its pitch.
	static string Tag(string kind)
	{
		return "[" + kind + "]";
	}

	// Why a giver has no job for you now (the locked row's reason)
	static const string WHY_NOTHING = "Nothing today. Ask again later";
	static const string WHY_TAKEN = "You've taken all they have. Check your Quests";

	//------------------------------------------------------------------------------------------------
	//! What a giver says in place of their pitch when they have no job you can take now (a
	//! conversation's node whose job choice is locked, RZ_DialogueTrees.DryPitch): the reason as
	//! their own words, and any other reason after it as a note.
	static void NothingSaid(string why, notnull array<string> lines)
	{
		if (why == WHY_NOTHING)
		{
			lines.Insert("Nothing I can give you today. Ask me again later.");
			return;
		}

		if (why == WHY_TAKEN)
		{
			lines.Insert("You've got everything I had. Finish what you took first.");
			return;
		}

		lines.Insert("There's nothing I can hand you right now.");
		if (!why.IsEmpty())
			lines.Insert("(" + why + ".)");
	}

	//------------------------------------------------------------------------------------------------
	//! A job's row: its title, tagged by its kind; locked with what it needs when you can't take it.
	static RZ_TalkReply Row(int id, string kind, string title, string need)
	{
		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = id;
		r.m_sLabel = title;
		r.m_sTag = Tag(kind);
		r.m_bOwn = true;	// written here: not a built label for RZ_TalkNeeds or RZ_TalkWords
		r.m_bJob = true;
		r.m_iRank = RZ_TalkReply.RANK_NEW;
		if (!need.IsEmpty())
		{
			r.m_bEnabled = false;
			r.m_sNeed = need;
			r.m_iRank = RZ_TalkReply.RANK_LOCKED;
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! The jobs a giver of `faction` has for this player here, as rows (ids[i] is rows[i]'s mission
	//! id, -1 for the "nothing" row): a leader's own job first (`l`, may be null), then the contracts
	//! they can take, nearest and most pressing first, then the ones they can't take yet, locked. With
	//! none at all, one locked row saying why.
	static void Build(RZ_GameModeComponent gm, int playerId, IEntity user, int faction, RZ_Leader l, notnull array<int> ids, notnull array<ref RZ_TalkReply> rows)
	{
		RZ_MissionManager missions = gm.GetMissions();
		RZ_Leaders leaders = gm.GetLeaders();
		if (!missions || !user || !RZ_Factions.IsValid(faction))
			return;

		// They won't deal with you at all: one locked row, saying so
		RZ_ETier tier = gm.GetTier(playerId, faction);
		if (RZ_Reputation.IsHunted(tier))
		{
			Nothing(rows, ids, faction, string.Format("Not while %1 with %2", RZ_Reputation.GetTierName(tier), RZ_Factions.GetShortName(faction)));
			return;
		}

		if (l && leaders && leaders.Attitude(l, playerId) <= -2)
		{
			Nothing(rows, ids, faction, "They won't work with you");
			return;
		}

		array<RZ_Mission> open = {};
		array<RZ_Mission> locked = {};
		// A leader's own job (an Independent or Rogue leader's, a questline's in-person stage) first
		if (l && leaders)
		{
			RZ_Mission own = leaders.FindJob(l);
			if (!own)
				own = leaders.TryCreateJob(l);

			if (own && own.m_eState == RZ_EMissionState.ACTIVE && !own.IsOfferedTo(gm, playerId))
			{
				string ownNeed = own.LockReason(gm, playerId);
				if (ownNeed.IsEmpty())
					open.Insert(own);
				else if (ownNeed != "Not for you now")
					locked.Insert(own);
			}
		}

		array<RZ_Mission> openHere = {};
		array<RZ_Mission> lockedHere = {};
		missions.JobsAt(playerId, faction, user, openHere, lockedHere);
		// Ask and you get: nothing they can take, so the giver makes a contract of their own now
		// (RZ_MissionManager.MakeFor: one they qualify for, at most once per ask gap)
		if (open.IsEmpty() && openHere.IsEmpty() && missions.MakeFor(playerId, faction))
		{
			openHere.Clear();
			lockedHere.Clear();
			missions.JobsAt(playerId, faction, user, openHere, lockedHere);
		}

		open.InsertAll(openHere);
		locked.InsertAll(lockedHere);

		foreach (RZ_Mission m : open)
		{
			if (rows.Count() >= MAX)
				break;

			ids.Insert(m.m_iId);
			rows.Insert(Row(BASE + rows.Count(), m.KindWord(), m.m_sTitle, ""));
		}

		foreach (RZ_Mission lm : locked)
		{
			if (rows.Count() >= MAX)
				break;

			ids.Insert(lm.m_iId);
			rows.Insert(Row(BASE + rows.Count(), lm.KindWord(), lm.m_sTitle, lm.LockReason(gm, playerId)));
		}

		if (!rows.IsEmpty())
			return;

		Nothing(rows, ids, faction, WhyNothing(gm, playerId, faction));
	}

	//------------------------------------------------------------------------------------------------
	//! Nothing to list: why, short. Every job of theirs already yours, or nothing going now.
	protected static string WhyNothing(RZ_GameModeComponent gm, int playerId, int faction)
	{
		foreach (RZ_Mission m : gm.GetMissions().GetActive())
		{
			if (m.m_bContract && m.m_eState == RZ_EMissionState.ACTIVE && m.m_Req.m_eGiver == faction && m.IsOfferedTo(gm, playerId))
				return WHY_TAKEN;
		}

		return WHY_NOTHING;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Nothing(notnull array<ref RZ_TalkReply> rows, notnull array<int> ids, int faction, string why)
	{
		ids.Insert(-1);
		rows.Insert(Row(NOTHING, "Job", RZ_TalkWords.Spoken("Ask for work", faction), why));
	}

	//------------------------------------------------------------------------------------------------
	//! The pitch for a job from someone who isn't its leader (a quartermaster): the leader's own words
	//! when one sits here (`l`), else the faction's; said under `header`. False if it's gone, or no
	//! longer yours to take: they say so.
	static bool Pitch(RZ_GameModeComponent gm, int playerId, int jobId, RZ_Leader l, string header, int faction, IEntity user)
	{
		RZ_Mission job = gm.GetMissions().FindById(jobId);
		if (!job || job.m_eState != RZ_EMissionState.ACTIVE || !job.IsEligible(gm, playerId) || job.IsOfferedTo(gm, playerId) || !gm.GetLeaders())
		{
			gm.NotifyPlayer(playerId, header, RZ_LeaderVoice.Gone(faction));
			return false;
		}

		gm.NotifyPlayer(playerId, header, gm.GetLeaders().Pitch(l, job, playerId, user));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Your answer to a pitched job from someone who isn't its leader: yes takes it at once (your
	//! Quests tab and the start card), no leaves it with them.
	static void Take(RZ_GameModeComponent gm, int playerId, int jobId, bool yes, string header, int faction, string giver, IEntity user)
	{
		if (!yes)
		{
			gm.NotifyPlayer(playerId, header, RZ_LeaderVoice.Declined(faction));
			return;
		}

		RZ_Mission job = gm.GetMissions().FindById(jobId);
		if (!job || job.m_eState != RZ_EMissionState.ACTIVE || !job.IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, header, RZ_LeaderVoice.Gone(faction));
			return;
		}

		if (job.Hire(playerId))
			RZ_Log.Info(string.Format("Mission %1 '%2' taken by %3 from %4", job.m_iId, job.m_sTitle, RZ_Util.PlayerName(playerId), giver));

		gm.NotifyPlayer(playerId, header, RZ_LeaderVoice.Agreed(faction));
		RZ_JobCards.MissionStarted(gm, playerId, job, giver, user);
	}
}
