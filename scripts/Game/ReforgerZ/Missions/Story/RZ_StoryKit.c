//! Helpers the story jobs share (docs/DESIGN.md, "Story blocks"): who may decide a scene's option
//! (eligible and took part, else "It's not your call."), a background read told to that player
//! alone and kept in the journal, a world record with its epilogue line, a fallout errand at the
//! subject, and the mission's people as stand-ins. Server only.
class RZ_StoryKit
{
	//------------------------------------------------------------------------------------------------
	//! Options that change the scene or end the job belong to someone the job was offered to who took
	//! part (a step, or 60 s within 150 m). False, and they're told, if not.
	static bool MayDecide(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (!m.m_Story)
			return m.IsEligible(gm, playerId);

		m.m_Story.m_Deciding.Step(gm, playerId);
		if (m.m_Story.m_Deciding.MayDecide(gm, m, playerId))
			return true;

		gm.NotifyPlayer(playerId, m.m_sTitle, "\"It's not your call. You weren't hired for this.\"");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A background read: one line to that player only, and the journal's "What you've found".
	static bool Read(RZ_GameModeComponent gm, RZ_Mission m, int playerId, string backgrounds, string line)
	{
		if (!RZ_QuestApproach.HasBackground(gm, playerId, backgrounds))
			return false;

		gm.NotifyPlayer(playerId, m.m_sTitle, line);
		if (m.m_Story && !m.m_Story.m_aFound.Contains(line))
			m.m_Story.m_aFound.Insert(line);

		RZ_Guidance.Report(playerId, "read");

		RZ_Log.Info(string.Format("Story: %1 read by %2 (%3)", m.m_sTitle, RZ_Util.PlayerName(playerId), backgrounds));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A world record about this job's subject, with who did it and an epilogue line (and its carrier).
	static void Record(RZ_Mission m, string what, string subject, string value, array<int> players, string epilogue = "", string carrier = "board")
	{
		string key = what;
		if (m.m_Story)
			key = m.m_Story.m_sKey + "." + what;

		RZ_WorldFacts.Get().Memory().Record(key, subject, value, RZ_WorldMemory.SOURCE_STORY, false, players, epilogue, carrier);
		RZ_Log.Info(string.Format("Story: %1 record %2 (%3) = %4", m.m_sTitle, key, subject, value));
		RZ_FinaleAssets.FromRecord(key, subject, value, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A failed story job's fallout: one errand at its subject, dealt to its participants first for 20
	//! minutes, then to anyone, paying at most 30% of the job's base (RZ_Errands.Fallout).
	static void Fallout(RZ_GameModeComponent gm, RZ_Mission m, string card, vector at, string who)
	{
		RZ_Errands errands = gm.GetErrands();
		if (!errands || m.m_bWithdrawn)
			return;

		int cap = RZ_Util.RoundToInt(m.m_iFavorReward * 0.3);
		errands.Fallout(card, at, m.m_aParticipants, cap, m.m_sTitle, who);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest present leader of `faction` within `range` of pos remembers this player (MEM_HELPED,
	//! MEM_CROSSED). The leader, or null.
	static RZ_Leader Remember(RZ_GameModeComponent gm, int faction, vector pos, float range, int playerId, int flag)
	{
		RZ_Leaders leaders = gm.GetLeaders();
		if (!leaders || playerId <= 0)
			return null;

		RZ_Leader l = leaders.FindNearest(faction, pos, range);
		if (l)
			leaders.Remember(l.m_iId, playerId, flag);

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! A story job moves one leader's loyalty, at most 5 either way, never below the seat ledger's
	//! floor (30) and never a stronghold's.
	static void Loyalty(RZ_GameModeComponent gm, RZ_Leader l, int delta)
	{
		RZ_Leaders leaders = gm.GetLeaders();
		if (!leaders || !l || l.IsStronghold())
			return;

		int shift = Math.ClampInt(delta, -5, 5);
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (budget)
			shift = budget.ContractLoyalty(l.m_iScore, false, shift);

		leaders.ShiftLoyalty(l, shift, true);
	}

	//------------------------------------------------------------------------------------------------
	//! A present Rogue leader of `faction` within `range` of pos, not a stronghold's; null if none.
	static RZ_Leader RogueNear(RZ_GameModeComponent gm, int faction, vector pos, float range)
	{
		RZ_Leaders leaders = gm.GetLeaders();
		if (!leaders)
			return null;

		array<RZ_Leader> list = {};
		leaders.GetFactionLeaders(faction, list);
		foreach (RZ_Leader l : list)
		{
			if (l.m_eLoyalty == RZ_ELoyalty.ROGUE && l.IsPresent() && !l.IsStronghold() && RZ_Util.DistXZ(l.m_vPos, pos) <= range)
				return l;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Scale a contract type's weight for the season (the majors' season-weight helper, inside the one
	//! clamp).
	static void SeasonWeight(typename t, float mult)
	{
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (budget)
			budget.SetContractWeight(t.ToString(), mult);
	}

	//------------------------------------------------------------------------------------------------
	//! People taken to Bastion's post: civilians sheltered goes up on that night's call (the season
	//! flag `bastion.sheltered`), and those with the job hear the number read.
	static void Sheltered(RZ_GameModeComponent gm, RZ_Mission m, int people)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		if (!facts || people <= 0)
			return;

		int total = facts.Flag("bastion.sheltered") + people;
		facts.SetFlag("bastion.sheltered", total);
		m.NotifyParticipants(gm, m.m_sTitle, string.Format("Bastion's truck takes them to the post. On the nightly call: \"Civilians sheltered, %1.\" It's %2 more than last night.", total, people));
	}

	//------------------------------------------------------------------------------------------------
	//! Leave: this player stops taking part (no pay, no blame); they're told so.
	static void Leave(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (!m.m_Story)
			return;

		m.m_Story.m_Deciding.Leave(gm, playerId);
		m.m_aParticipants.RemoveItem(playerId);
		gm.NotifyPlayerAs(playerId, m.m_sTitle, "You leave it to them. Nobody pays you for it, and nobody blames you either.", m.NoticeTag());
		RZ_Log.Info(string.Format("Story: %1 left by %2", m.m_sTitle, RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! Amends need clean hands: this player pressed, tricked, switched or hid something in this job.
	static void Dirty(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (m && m.m_Story)
			m.m_Story.m_Deciding.Dirty(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A story job done: whoever took part (a step, or 5 minutes in the area) and isn't online now is
	//! paid the base at their next join (RZ_Errands' pending pay), never more than an online
	//! participant would be. Not for a switch, a leaver or a job that pays nobody.
	static void PayOffline(RZ_GameModeComponent gm, RZ_Mission m)
	{
		RZ_Errands errands = gm.GetErrands();
		RZ_Questlines q = gm.GetQuestlines();
		if (!m.m_Story || !errands || !q || !m.m_bHasGiver || m.m_bSwitched || !RZ_Factions.IsValid(m.m_eGiver))
			return;

		array<string> idents = {};
		m.m_Story.m_Deciding.TookPartIdents(300, idents);
		int fame = RZ_Util.RoundToInt(m.m_iFameReward * m.m_fRewardScale);
		int favor = RZ_Util.RoundToInt(m.m_iFavorReward * m.m_fRewardScale);
		foreach (string ident : idents)
		{
			if (q.OnlineId(ident) > 0)
				continue;

			errands.AddPending(ident, m.m_eGiver, fame, favor, m.m_sTitle);
			RZ_Log.Info(string.Format("Story: %1 pays %2 at their next join (%3 Fame, %4 Favor)", m.m_sTitle, ident, fame, favor));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A Nurse dressed a bite and said nothing at a house's door: the life record "hid_bite", and
	//! "hid_bite.<house>", which that house's people read (HouseHarder) for the rest of the life.
	static void HidBite(RZ_GameModeComponent gm, int playerId, int house)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (!p)
			return;

		RZ_Deeds.AddLifeRecord(p, "hid_bite", true);
		RZ_Log.Info(string.Format("Story: %1 hid a bite at house %2", RZ_Util.PlayerName(playerId), house));
		if (house >= 0)
			RZ_Deeds.AddLifeRecord(p, "hid_bite." + house.ToString(), true);
	}

	//------------------------------------------------------------------------------------------------
	//! How much harder this house's people make a check for this player: 1 if this life hid a bite at
	//! their door, else 0. Never makes a check impossible (difficulty 3 at most).
	static int HouseHarder(RZ_GameModeComponent gm, int playerId, int house)
	{
		if (!gm || house < 0 || playerId <= 0)
			return 0;

		if (RZ_Deeds.HasLifeRecord(gm.GetProfile(playerId), "hid_bite." + house.ToString()))
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A check's difficulty at a house's people: one step harder after a hidden bite there, at most 3.
	static int AtHouse(RZ_GameModeComponent gm, int playerId, int house, int difficulty)
	{
		return Math.MinInt(3, difficulty + HouseHarder(gm, playerId, house));
	}

	//------------------------------------------------------------------------------------------------
	//! A player within `range` of pos with one of these backgrounds, -1 if none.
	static int WithBackground(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players, vector pos, float range, string backgrounds)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= range && RZ_QuestApproach.HasBackground(gm, ids[i], backgrounds))
				return ids[i];
		}

		return -1;
	}
}
