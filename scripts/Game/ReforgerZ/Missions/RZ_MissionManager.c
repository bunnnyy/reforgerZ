//! Keeps world events and faction contracts running.
//!
//! - World events: news on the radio for everyone, not jobs; anyone who takes part shares the reward.
//! - Contracts: offered only in person (docs/MISSIONS.md, "Who gets offered what"). The radio tells
//!   eligible players who to ask (TellOfWork: a Radio-tab entry, never a card, at most one a player
//!   a minute); asking the giver (a quartermaster, a leader, a safehouse radio) hires them
//!   (ListContractsFor, RZ_Mission.Hire). Hired players who meet the requirements and take part
//!   get paid, and so does their group; anyone else who turns up gets nothing.
//!
//! Counts scale with the stage and the player count. Types are chosen by
//! weight, and each type's weight depends on the world state.
class RZ_MissionManager
{
	protected RZ_GameModeComponent m_GM;
	protected ref array<ref RZ_Mission> m_aActive = {};
	protected ref array<typename> m_aTypes = {};
	protected int m_iNextId = 1;
	protected float m_fEventCooldown = 60;
	protected float m_fContractCooldown = 30;
	protected float m_fClock;
	protected ref map<int, float> m_mLastOffer = new map<int, float>();
	protected ref array<vector> m_aPropSpots = {};		// GetPropSpots of every running mission, once per tick
	protected float m_fSpotsAt = -1;
	protected float m_fReasonLaneAt = 300;				// the reason lane's next look (the first after 5 minutes)
	protected ref array<string> m_aReasonsUsed = {};	// "key|subject" reasons that already started a contract

	static const float PROP_CLEAR = 25;	// no building this close (m) to a running mission's props has a lock

	protected static const float EVENT_COOLDOWN = 150;
	protected static const float CONTRACT_COOLDOWN = 75;
	protected static const float OFFER_GAP = 60;			// min seconds between two "who to ask" lines to one player
	protected static const int MAX_CONTRACTS = 10;
	// Ask and you get (MakeFor): a giver asked for work makes a contract of their own
	protected static const float ASK_GAP = 90;			// seconds between two tries for one player and giver
	protected static const int ASK_PER_GIVER = 3;		// that giver's contracts alive at once before asking makes no more
	protected ref map<string, float> m_mAskAt = new map<string, float>();

	//------------------------------------------------------------------------------------------------
	void RZ_MissionManager(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		RZ_Escort.ResetCount();
		RZ_LiveCapture.ResetCount();
		RZ_Mission_LastHelicopter.ResetBoost();
		RZ_MissionRegistry.RegisterAll(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Other mods can add mission types (subclasses of RZ_Mission) here.
	void RegisterType(typename type)
	{
		if (!m_aTypes.Contains(type))
			m_aTypes.Insert(type);
	}

	//------------------------------------------------------------------------------------------------
	array<ref RZ_Mission> GetActive()
	{
		return m_aActive;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Mission FindById(int id)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_iId == id)
				return m;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A running mission other than `asker` uses this town, or a story holds it: the town itself
	//! (RZ_WorldFacts.Town), a safehouse in it, or the lab (the cure holds it during the final
	//! stand). With `asker`, what the asker or the story that started it holds doesn't count, so
	//! a mission can use its own town and its questline's house (RZ_Mission.IsTownBusy).
	bool IsSettlementBusy(int idx, RZ_Mission asker = null)
	{
		if (idx < 0)
			return false;

		foreach (RZ_Mission m : m_aActive)
		{
			if (m != asker && m.m_iSettlement == idx)
				return true;
		}

		if (IsHeldAgainst(RZ_WorldFacts.Town(idx), asker))
			return true;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (h && h.m_iSettlement == idx && IsHeldAgainst(RZ_WorldFacts.Safehouse(i), asker))
					return true;
			}
		}

		return m_GM.HasLab() && m_GM.GetLabSettlement() == idx && IsHeldAgainst(RZ_WorldFacts.LAB, asker);
	}

	//------------------------------------------------------------------------------------------------
	//! Claimed, and not by `asker` or its story (any claim without an asker).
	protected bool IsHeldAgainst(string subject, RZ_Mission asker)
	{
		if (!m_GM.GetFacts().IsClaimed(subject))
			return false;

		return !asker || !asker.IsSubjectFree(m_GM, subject);
	}

	//------------------------------------------------------------------------------------------------
	//! The manager also picks this type at random (registered), so its weight says whether the
	//! world allows it now. Story-only missions (questline stages, arc-only events) aren't.
	bool IsRegistered(typename t)
	{
		return m_aTypes.Contains(t);
	}

	// ================================================================ Can it run now?
	//------------------------------------------------------------------------------------------------
	//! The one check every starter uses before it starts a configured mission (random picks, story
	//! arcs, questline stages, leaders' jobs; docs/DESIGN.md, "World facts"): the requirements'
	//! world gates and facts (stage, cure, Bastion presence, the day, time of day, flags,
	//! questlines), the season's No Vehicles rule, its weight if the manager also picks this type at
	//! random (checkWeight), one of each type at a time, and its claims free. "" if it can, else why not.
	string WhyNot(RZ_Mission m, int playerCount, bool checkWeight = true)
	{
		if (!m)
			return "no mission";

		if (m.NeedsVehicle() && RZ_Seasons.Mods().m_bNoVehicles)
			return "no vehicles this season";

		if (!m.m_Req.IsWorldOk(m_GM, playerCount))
			return "the world isn't right (requirements)";

		// A leader's personal job: one per leader (RZ_Leaders.FindJob), but two leaders may each
		// offer their own of the same kind
		if (!m.m_bInPersonOnly && IsTypeActive(m.Type()))
			return "one of this type is already running";

		array<string> subjects = {};
		m.GetClaims(m_GM, subjects);
		foreach (string subject : subjects)
		{
			if (!m.IsSubjectFree(m_GM, subject))
				return subject + " is in use by " + m_GM.GetFacts().ClaimedBy(subject);
		}

		if (checkWeight && IsRegistered(m.Type()) && m.GetWeight(m_GM) <= 0)
			return "its weight is 0 now";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	bool CanRunNow(RZ_Mission m, int playerCount, bool checkWeight = true)
	{
		return WhyNot(m, playerCount, checkWeight).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! The cure is out (RZ_CureResearch): every running mission that stops with it (m_bNotCured,
	//! and Halcyon's research unless m_bAfterCure) is called off, with nobody paid or blamed. The
	//! drones have gone home, so nothing at the line goes on shooting.
	void OnCured()
	{
		int count = 0;
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_eState != RZ_EMissionState.ACTIVE || !m.m_Req.StopsWithCure())
				continue;

			m.Withdraw("Called off. The cure is out, and nobody needs this done any more.");
			count++;
		}

		if (count > 0)
			RZ_Log.Info(string.Format("The cure is out: %1 running mission(s) called off.", count));
	}

	//------------------------------------------------------------------------------------------------
	//! CanRunNow, then StartMission. What story arcs, questlines and leaders call.
	bool StartIfCan(RZ_Mission m, int playerCount)
	{
		string why = WhyNot(m, playerCount);
		if (!why.IsEmpty())
			return false;

		return StartMission(m);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsTypeActive(typename t)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.Type() == t)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! True while a mission of this type runs (e.g. RZ_Event_Storm, for wanderers to shelter).
	bool IsTypeRunning(typename t)
	{
		return IsTypeActive(t);
	}

	//------------------------------------------------------------------------------------------------
	protected int CountActive(bool contracts)
	{
		int n = 0;
		// Leaders' personal jobs don't take a radio contract's place
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_bContract == contracts && !m.m_bInPersonOnly)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		RZ_StoryStats.Tick(dt, ids.Count());
		RZ_BriefLies.Tick(m_GM, dt, m_aActive);

		for (int i = m_aActive.Count() - 1; i >= 0; i--)
		{
			// A mission the world called off doesn't get one more update (it could still pay out)
			RZ_Mission m = m_aActive[i];
			if (!m.m_bWithdrawn)
				m.Update(m_GM, dt, ids, players);

			if (m.m_eState != RZ_EMissionState.ACTIVE)
			{
				Finish(m);
				m_aActive.Remove(i);
			}
		}

		if (ids.IsEmpty())
			return;

		int baseCount = m_GM.GetDirector().GetStageSettings().m_iBaseMissions;
		int eventTarget = Math.Max(1, baseCount - 1) + ids.Count() / 6;
		int contractTarget = Math.Min(MAX_CONTRACTS, 2 + ids.Count() / 3);

		m_fEventCooldown -= dt;
		if (m_fEventCooldown <= 0 && CountActive(false) < eventTarget)
		{
			m_fEventCooldown = EVENT_COOLDOWN;
			CreateRandom(false, ids);
		}

		m_fContractCooldown -= dt;
		if (m_fContractCooldown <= 0 && CountActive(true) < contractTarget)
		{
			m_fContractCooldown = CONTRACT_COOLDOWN;
			CreateRandom(true, ids);
		}

		ReasonLane(ids);
		TellOfWork(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! Start a mission made elsewhere (e.g. the Turned Player event). Configure() must already
	//! have run. Returns false if Setup() found no valid location.
	bool StartMission(RZ_Mission mission)
	{
		// Every attempt gets its own id: guards a failed setup spawned are never mistaken for a later mission's
		mission.m_iId = m_iNextId;
		m_iNextId++;

		// What it said it needs is held from before Setup (Setup may claim more)
		RZ_WorldFacts facts = m_GM.GetFacts();
		string owner = RZ_WorldFacts.MissionOwner(mission.m_iId);
		array<string> subjects = {};
		mission.GetClaims(m_GM, subjects);
		bool claimed = true;
		foreach (string subject : subjects)
		{
			if (!facts.ClaimFor(subject, owner, mission.m_sClaimParent))
				claimed = false;
		}

		bool placed = claimed && mission.Setup(m_GM);

		// A reason-started contract must land near its reason, or it isn't that reason's
		if (placed && mission.m_vReasonAt != vector.Zero && RZ_Util.DistXZ(mission.m_vPos, mission.m_vReasonAt) > RZ_MissionStorySettings.Get().m_fReasonRange)
			placed = false;

		if (!placed)
		{
			// Whatever it spawned or claimed before it gave up goes again
			mission.m_eState = RZ_EMissionState.FAILED;
			m_GM.GetSpawner().ReleaseMission(mission.m_iId);
			facts.ReleaseAll(owner);
			RZ_StoryGroups.Release(mission.m_iId);
			mission.Cleanup(m_GM);
			return false;
		}

		// Briefs often open with a place or a name ("the chemist's in ...")
		mission.m_sBrief = RZ_Util.Capitalize(mission.m_sBrief);
		// The live place check (docs/PLACES.md): dry, off the line, near the town its brief names
		if (!mission.m_bGlobal)
			RZ_PlaceNeeds.Verify(mission.PlaceNeedKey(), mission.m_vPos, mission.m_sTitle, mission.m_iSettlement);
		// The Tier 0 audit: a job that ended here before says how it went
		RZ_MissionReasons.Tier0Read(mission);
		// A work job: whose mark is on the wall here (the town tally)
		RZ_WorkRows.Read(mission);
		m_aActive.Insert(mission);
		m_fSpotsAt = -1; // its props count from now
		OnStarted(mission);
		// A giver who lies leaves one thing out of the brief (Missions/Story/RZ_Grey.c)
		RZ_BriefLies.OnStarted(m_GM, mission);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnStarted(RZ_Mission mission)
	{
		// On-book contracts are voiced and paid by the giver's nearest Loyal leader
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AssignVoice(mission);

		RZ_Log.Info(string.Format("Mission %1 '%2' at %3 (contract %4)", mission.m_iId, mission.m_sTitle, mission.GetLocationName(m_GM), mission.m_bContract));

		if (!mission.m_bContract)
		{
			m_GM.AnnounceMission(mission);
			if (!mission.m_bGlobal)
				mission.m_iMarkerId = m_GM.AddMapMarker(mission.m_vPos, mission.m_sTitle);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnZombiesKilled(int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			m.OnZombiesKilled(m_GM, settlementIdx, missionId, special, pos, count);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerShot(int playerId, vector pos)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			m.OnPlayerShot(m_GM, playerId, pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_SupplyLines: a truck arrived or was lost.
	void OnTruckEvent(int faction, int what, vector pos, int driver)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			m.OnTruckEvent(m_GM, faction, what, pos, driver);
		}

		// Errand pay that waits for the truck (docs/ERRANDS.md, "Pay: nothing for free")
		if (m_GM.GetErrands())
			m_GM.GetErrands().OnTruck(faction, what, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! A building at pos may not be locked: a running mission has props, items or people within
	//! PROP_CLEAR of it, and the lock isn't part of that mission (Locked Bunker). From
	//! RZ_Search.HasLock, so the doors, the loose loot and the spot pickers all agree.
	bool IsNearMissionProp(vector pos)
	{
		if (m_aActive.IsEmpty())
			return false;

		if (m_fSpotsAt != m_fClock)
		{
			m_fSpotsAt = m_fClock;
			m_aPropSpots.Clear();
			foreach (RZ_Mission m : m_aActive)
			{
				if (m.m_eState == RZ_EMissionState.ACTIVE)
					m.GetPropSpots(m_aPropSpots);
			}
		}

		bool isNear = false;
		foreach (vector spot : m_aPropSpots)
		{
			if (RZ_Util.DistXZ(spot, pos) <= PROP_CLEAR)
			{
				isNear = true;
				break;
			}
		}

		if (!isNear)
			return false;

		string buildingKey = RZ_Interactions.PosKey(pos, 4);
		foreach (RZ_Mission keeper : m_aActive)
		{
			if (keeper.KeepsLock(buildingKey))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds since the server started (missions that leave an effect behind for a while).
	float GetClock()
	{
		return m_fClock;
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDied(int playerId, vector pos)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			m.OnPlayerDied(m_GM, playerId, pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Mission_BanditDeadDrop FindDeadDrop(IEntity drop)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			RZ_Mission_BanditDeadDrop deadDrop = RZ_Mission_BanditDeadDrop.Cast(m);
			if (deadDrop && deadDrop.IsDrop(drop))
				return deadDrop;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Refuses (with a message) players the Red Hand won't deal with.
	bool CanSellToBandits(int playerId, IEntity drop)
	{
		RZ_Mission_BanditDeadDrop deadDrop = FindDeadDrop(drop);
		if (!deadDrop)
			return false;

		if (deadDrop.IsEligible(m_GM, playerId))
			return true;

		m_GM.NotifyPlayerAs(playerId, "The Red Hand", "Nobody comes out to meet you. The Red Hand doesn't deal with you.", RZ_ENoticeTag.MISSION);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void OnSamplesSoldToBandits(int playerId, int value, IEntity drop)
	{
		RZ_Mission_BanditDeadDrop deadDrop = FindDeadDrop(drop);
		if (deadDrop)
			deadDrop.OnSold(m_GM, playerId, value);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_ChoiceDeliveryAction. True if the mission took the item.
	bool OnItemDelivered(int playerId, int missionId, RZ_EFaction faction, IEntity drop)
	{
		RZ_Mission m = FindById(missionId);
		if (!m || m.m_eState != RZ_EMissionState.ACTIVE)
			return false;

		return m.OnItemDelivered(m_GM, playerId, faction, drop);
	}

	// ================================================================ Contract offers
	//------------------------------------------------------------------------------------------------
	//! Jobs only by interaction (docs/MISSIONS.md, "Who gets offered what"): the radio never offers
	//! a job. It tells each eligible player who to ask for one they haven't been offered, as a Radio-tab
	//! entry (RZ_ENoticeTag.HEARD: under Missions, never a card, not a job), at most one a player per
	//! OFFER_GAP and once a contract. Leaders' own jobs are heard of only from them.
	protected void TellOfWork(array<int> ids, array<IEntity> players)
	{
		foreach (int i, int playerId : ids)
		{
			float last;
			if (m_mLastOffer.Find(playerId, last) && m_fClock - last < OFFER_GAP)
				continue;

			foreach (RZ_Mission m : m_aActive)
			{
				if (!m.m_bContract || m.m_bInPersonOnly || m.m_eState != RZ_EMissionState.ACTIVE)
					continue;

				if (m.m_aHeardBy.Contains(playerId) || m.IsOfferedTo(m_GM, playerId) || !m.IsEligible(m_GM, playerId))
					continue;

				HearOf(m, playerId, players[i]);
				m_mLastOffer.Set(playerId, m_fClock);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Bastion wants people for Recover the Bus, east of Levie, a few km north-west. Ask Sergeant
	//! Hale at Fort Kell, or at any Bastion quartermaster." Once per player and contract.
	void HearOf(RZ_Mission m, int playerId, IEntity player)
	{
		if (!m || m.m_aHeardBy.Contains(playerId))
			return;

		m.m_aHeardBy.Insert(playerId);
		vector from = vector.Zero;
		if (player)
			from = player.GetOrigin();

		string area = m.GetLocationName(m_GM);
		if (player && !m.m_bGlobal)
			area = RZ_QuestText.RoughArea(m_GM, from, m.GetAreaFor(m_GM, playerId, from));

		string text = string.Format("%1 wants people for %2, %3. %4", RZ_Factions.GetName(m.m_Req.m_eGiver), m.m_sTitle, area, GiverHint(m, m.m_Req.m_eGiver));
		m_GM.NotifyPlayerAs(playerId, "Heard on the radio", text, RZ_ENoticeTag.HEARD);
	}

	//------------------------------------------------------------------------------------------------
	//! Who to ask for a contract from `faction`, as a sentence: its leader in person, then the
	//! faction's quartermaster, and a Haven safehouse's radio for the Haven's work.
	string GiverHint(RZ_Mission m, int faction)
	{
		string leader = "";
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders && m && m.m_iLeaderId >= 0 && m.m_eGiver == faction)
		{
			RZ_Leader l = leaders.Get(m.m_iLeaderId);
			if (l && l.IsPresent())
				leader = string.Format("%1 at %2", l.m_sName, l.m_sPlace);
		}

		string qm = string.Format("any %1 quartermaster", RZ_Factions.GetShortName(faction));
		if (faction == RZ_EFaction.SURVIVORS)
			qm = qm + " or a safehouse radio";

		if (leader.IsEmpty())
			return string.Format("Ask at %1.", qm);

		return string.Format("Ask %1, or at %2.", leader, qm);
	}

	//------------------------------------------------------------------------------------------------
	//! Open contracts `faction` could offer this player here and hasn't yet (the quartermaster's
	//! greeting). Offers nothing.
	int CountContractsFor(int playerId, RZ_EFaction faction, IEntity player)
	{
		int n = 0;
		foreach (RZ_Mission c : m_aActive)
		{
			if (OffersAt(c, playerId, faction, player) && !c.IsOfferedTo(m_GM, playerId))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The talk screen's jobs at a giver of `faction` here (RZ_TalkJobs, docs/DIALOGUE.md, "Jobs on
	//! the first replies"): `open` the contracts it may offer this player that aren't theirs yet, in
	//! this player's order; `locked` the ones it gives here that this player can't take yet for a
	//! reason they can meet (a tier, a background, standing elsewhere), never another's reserved job.
	//! A leader's personal job isn't here (RZ_Leaders.FindJob). Offers nothing.
	void JobsAt(int playerId, RZ_EFaction faction, IEntity player, notnull array<RZ_Mission> open, notnull array<RZ_Mission> locked)
	{
		if (!player)
			return;

		foreach (RZ_Mission c : m_aActive)
		{
			if (!c.m_bContract || c.m_bInPersonOnly || c.m_eState != RZ_EMissionState.ACTIVE || c.IsOfferedTo(m_GM, playerId))
				continue;

			if (OffersAt(c, playerId, faction, player))
			{
				open.Insert(c);
				continue;
			}

			if (c.m_Req.m_eGiver != faction || !c.m_sReservedFor.IsEmpty() || !c.CanOfferAt(m_GM, player.GetOrigin()))
				continue;

			if (RZ_OfferReach.DecideFor(m_GM, c, faction, player.GetOrigin()) == RZ_OfferReach.ELSEWHERE)
				continue;

			string need = c.LockReason(m_GM, playerId);
			if (!need.IsEmpty() && need != "Not for you now")
				locked.Insert(c);
		}

		RZ_MissionReasons.SortFor(m_GM, open, playerId, player.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	//! Ask and you get (docs/MISSIONS.md, "Who gets offered what"): a giver asked for work with no
	//! open contract this player can take makes one of their own now, one the player qualifies for
	//! and the world can run (the same checks as the random pick, in the same weighted order), so a
	//! newcomer isn't left to the two contracts the shared pool happens to hold. Tried once per
	//! ASK_GAP for a player and giver; never past the server's contract cap, nor ASK_PER_GIVER of that
	//! giver's alive at once. True if one was made. With `at` (where the giver stands), its town is
	//! picked within offer reach of it when one fits (RZ_OfferReach), and that giver always offers it.
	bool MakeFor(int playerId, RZ_EFaction faction, vector at = vector.Zero)
	{
		string askKey = playerId.ToString() + ":" + faction.ToString();
		float askedAt;
		if (m_mAskAt.Find(askKey, askedAt) && m_fClock - askedAt < ASK_GAP)
			return false;

		m_mAskAt.Set(askKey, m_fClock);
		if (CountActive(true) >= MAX_CONTRACTS)
			return false;

		int theirs = 0;
		foreach (RZ_Mission running : m_aActive)
		{
			if (running.m_bContract && !running.m_bInPersonOnly && running.m_Req.m_eGiver == faction)
				theirs++;
		}

		if (theirs >= ASK_PER_GIVER)
			return false;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		array<ref RZ_Mission> candidates = {};
		array<float> weights = {};
		float total = 0;
		foreach (typename t : m_aTypes)
		{
			if (IsTypeActive(t))
				continue;

			RZ_Mission m = RZ_Mission.Cast(t.Spawn());
			if (!m)
				continue;

			m.Configure(m_GM);
			if (!m.m_bContract || m.m_bInPersonOnly || m.m_Req.m_eGiver != faction)
				continue;

			if (!CanRunNow(m, ids.Count(), false) || !m.IsEligible(m_GM, playerId))
				continue;

			float w = m.GetWeight(m_GM);
			if (w <= 0)
				continue;

			w = w * WeightMult(m) * PresenceMult(m);
			candidates.Insert(m);
			weights.Insert(w);
			total += w;
		}

		while (!candidates.IsEmpty())
		{
			float roll = Math.RandomFloat(0, total);
			int pick = candidates.Count() - 1;
			for (int i = 0; i < candidates.Count(); i++)
			{
				roll -= weights[i];
				if (roll <= 0)
				{
					pick = i;
					break;
				}
			}

			RZ_Mission mission = candidates[pick];
			mission.m_vAskedAt = at;
			RZ_OfferReach.SetNear(m_GM, at);
			bool started = StartMission(mission);
			RZ_OfferReach.ClearNear();
			if (started)
			{
				if (at != vector.Zero)
					RZ_Log.Info(string.Format("Reach: made on asking %1 '%2', %3 m from the giver", mission.m_iId, mission.m_sTitle, RZ_Util.RoundToInt(RZ_Util.DistXZ(at, mission.m_vPos))));

				RZ_Log.Info(string.Format("Mission %1 '%2' made on asking: %3 asked %4.", mission.m_iId, mission.m_sTitle, RZ_Util.PlayerName(playerId), RZ_Factions.GetShortName(faction)));
				return true;
			}

			total -= weights[pick];
			candidates.Remove(pick);
			weights.Remove(pick);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A contract `faction` may offer this player here: its own (a leader's personal job only at
	//! their seat), or a mirror's second side once it has heard.
	protected bool OffersAt(RZ_Mission c, int playerId, RZ_EFaction faction, IEntity player)
	{
		if (!c.m_bContract || c.m_eState != RZ_EMissionState.ACTIVE || !c.IsEligible(m_GM, playerId))
			return false;

		bool ours = c.m_Req.m_eGiver == faction;
		if (!ours && c.m_Mirror && c.m_Mirror.m_iFaction == faction)
			ours = c.m_Mirror.IsSide(m_GM, playerId);

		if (!ours || !c.CanOfferAt(m_GM, player.GetOrigin()))
			return false;

		// Offer reach: a contract another giver of the faction has within reach is theirs
		return RZ_OfferReach.DecideFor(m_GM, c, faction, player.GetOrigin()) != RZ_OfferReach.ELSEWHERE;
	}

	//------------------------------------------------------------------------------------------------
	//! The giver offers its open contracts in person (a quartermaster's "Check standing and work", a
	//! leader's "Ask for work"): every one this player qualifies for is offered to them (hired), and
	//! the list comes back as text, each with where it is.
	string ListContractsFor(int playerId, RZ_EFaction faction, IEntity player)
	{
		string text = "";
		array<RZ_Mission> offers = {};
		foreach (RZ_Mission c : m_aActive)
		{
			if (OffersAt(c, playerId, faction, player))
				offers.Insert(c);
		}

		// Ask and you get: nothing open for them, so the giver makes one now
		if (offers.IsEmpty() && MakeFor(playerId, faction, player.GetOrigin()))
		{
			foreach (RZ_Mission made : m_aActive)
			{
				if (OffersAt(made, playerId, faction, player))
					offers.Insert(made);
			}
		}

		// This player's order: near, reason-started, a leader they helped first (docs/DESIGN.md)
		RZ_MissionReasons.SortFor(m_GM, offers, playerId, player.GetOrigin());
		foreach (RZ_Mission m : offers)
		{
			if (m.Hire(playerId))
			{
				RZ_Log.Info(string.Format("Mission %1 '%2' offered to %3 by %4", m.m_iId, m.m_sTitle, RZ_Util.PlayerName(playerId), RZ_Factions.GetShortName(faction)));
				RZ_JobCards.MissionStarted(m_GM, playerId, m, RZ_Factions.GetName(faction), player);
			}

			text += string.Format("%1: %2\n", m.m_sTitle, m.GetWhereTextFor(m_GM, player.GetOrigin(), playerId));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Nearest active mission this player can be paid for (Haven "Neighbours" perk).
	RZ_Mission FindNearestFor(int playerId, vector pos)
	{
		RZ_Mission best;
		float bestDist = float.MAX;
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_bGlobal || m.m_bInPersonOnly || !m.IsEligible(m_GM, playerId))
				continue;

			float d = RZ_Util.DistXZ(pos, m.m_vPos);
			if (d < bestDist)
			{
				bestDist = d;
				best = m;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! What goes in a player's Quests tab: the jobs that are theirs (RZ_Mission.IsOfferedTo: offered
	//! in person, reserved, a stage of a questline they're part of, an arc or puzzle taken by its
	//! clue). Never a plain world event (news, not a job), never one only heard of. Marks nothing.
	void ListFor(int playerId, notnull array<RZ_Mission> result)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_eState == RZ_EMissionState.ACTIVE && m.IsOfferedTo(m_GM, playerId))
				result.Insert(m);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The spawn's Radio entry: the world events going on (news), then the work going that this
	//! player could ask for, each with who to ask. Offers nothing: a job is only ever offered in person.
	string DescribeActiveFor(IEntity player, int playerId)
	{
		string text = "";
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_bContract || m.m_eState != RZ_EMissionState.ACTIVE)
				continue;

			text += string.Format("%1: %2\n", m.m_sTitle, m.GetWhereTextFor(m_GM, player.GetOrigin(), playerId));
		}

		string contracts = "";
		foreach (RZ_Mission c : m_aActive)
		{
			if (!c.m_bContract || c.m_bInPersonOnly || c.m_eState != RZ_EMissionState.ACTIVE || !c.IsEligible(m_GM, playerId) || c.IsOfferedTo(m_GM, playerId))
				continue;

			if (!c.m_aHeardBy.Contains(playerId))
				c.m_aHeardBy.Insert(playerId);

			contracts += string.Format("%1 (%2). %3\n", c.m_sTitle, RZ_Factions.GetShortName(c.m_Req.m_eGiver), GiverHint(c, c.m_Req.m_eGiver));
		}

		if (!contracts.IsEmpty())
			text += "\nWork going, if you ask:\n" + contracts;

		if (text.IsEmpty())
			return "The radio is quiet.";

		return text;
	}

	// ================================================================ Creation
	//------------------------------------------------------------------------------------------------
	protected void CreateRandom(bool contract, array<int> ids)
	{
		array<ref RZ_Mission> candidates = {};
		array<float> weights = {};
		float total = 0;
		foreach (typename t : m_aTypes)
		{
			if (IsTypeActive(t))
				continue;

			RZ_Mission m = RZ_Mission.Cast(t.Spawn());
			if (!m)
				continue;

			m.Configure(m_GM);
			if (m.m_bContract != contract)
				continue;

			// The shared check (world gates and facts, No Vehicles, one of each type, claims); the
			// weight comes next, with the leader and faction-life multipliers
			if (!CanRunNow(m, ids.Count(), false))
				continue;

			// A contract nobody online could take would just sit there
			if (contract && !m.AnyEligible(m_GM, ids))
				continue;

			float w = m.GetWeight(m_GM);
			if (w <= 0)
				continue;

			// The giver's stronghold leader leans toward some kinds of work (traits, +-20%), so does
			// what the faction needs right now (lost trucks, deaths, research), and the majors' season
			// weights; their product is clamped once (Missions > Story > Weight clamp)
			if (contract)
				w = w * WeightMult(m) * PresenceMult(m);

			candidates.Insert(m);
			weights.Insert(w);
			total += w;
		}

		// Try in weighted order until one finds a valid location
		while (!candidates.IsEmpty())
		{
			float roll = Math.RandomFloat(0, total);
			int pick = candidates.Count() - 1;
			for (int i = 0; i < candidates.Count(); i++)
			{
				roll -= weights[i];
				if (roll <= 0)
				{
					pick = i;
					break;
				}
			}

			RZ_Mission mission = candidates[pick];
			if (StartMission(mission))
				return;

			total -= weights[pick];
			candidates.Remove(pick);
			weights.Remove(pick);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's contracts come as often as Bastion is still out on the roads (docs/DESIGN.md,
	//! "Outbreak stages"): x1 at full presence, x0.37 in Aftermath (0.1). Outside the clamp below,
	//! because it's the world's state, not a lean. A contract whose own weight already follows
	//! presence (m_bPresenceWeighted) isn't scaled twice.
	float PresenceMult(RZ_Mission m)
	{
		if (!m || m.m_bPresenceWeighted || m.m_Req.m_eGiver != RZ_EFaction.MILITARY)
			return 1;

		float presence = Math.Clamp(m_GM.GetFacts().BastionPresence(), 0, 1);
		return 0.3 + 0.7 * presence;
	}

	//------------------------------------------------------------------------------------------------
	//! The one clamp on a contract's weight multipliers: leader traits x faction needs x the majors'
	//! season weights, together within x0.75-1.3 (Missions > Story).
	float WeightMult(RZ_Mission m)
	{
		float mult = 1;
		if (m_GM.GetLeaders())
			mult = mult * m_GM.GetLeaders().ContractWeightMult(m);

		if (m_GM.GetFactionLife())
			mult = mult * m_GM.GetFactionLife().ContractWeightMult(m);

		RZ_Shared shared = m_GM.GetShared();
		if (shared)
			mult = mult * shared.GetBudget().ContractWeight(m.Type().ToString());

		RZ_MissionStorySettings ss = RZ_MissionStorySettings.Get();
		return Math.Clamp(mult, ss.m_fWeightMin, Math.Max(ss.m_fWeightMin, ss.m_fWeightMax));
	}

	// ================================================================ The reason lane
	//------------------------------------------------------------------------------------------------
	//! Reasons (World/RZ_WorldMemory.c) start contracts (docs/DESIGN.md, "Story blocks"): when the world has
	//! posted a reason, one registered contract that answers it (RZ_MissionReasons) may start near its
	//! subject, at most one every Reason lane gap server-wide, inside the contract cap and every check
	//! (CanRunNow, someone online eligible, claims). Its brief opens with the reason's line. Random
	//! picks stay for variety.
	protected void ReasonLane(array<int> ids)
	{
		RZ_MissionStorySettings ss = RZ_MissionStorySettings.Get();
		if (!ss.m_bEnabled || !ss.m_bReasonLane || m_fClock < m_fReasonLaneAt)
			return;

		int contractTarget = Math.Min(MAX_CONTRACTS, 2 + ids.Count() / 3);
		if (CountActive(true) >= contractTarget)
			return;

		RZ_WorldMemory mem = m_GM.GetFacts().Memory();
		array<RZ_Reason> reasons = {};
		mem.AllReasons(reasons);
		for (int r = reasons.Count() - 1; r >= 0; r--)
		{
			RZ_Reason reason = reasons[r];
			if (m_aReasonsUsed.Contains(reason.key + "|" + reason.subject))
				continue;

			foreach (typename t : m_aTypes)
			{
				if (IsTypeActive(t) || !RZ_MissionReasons.Answers(t, reason.key))
					continue;

				RZ_Mission m = RZ_Mission.Cast(t.Spawn());
				if (!m)
					continue;

				m.Configure(m_GM);
				if (!m.m_bContract || !CanRunNow(m, ids.Count()) || !m.AnyEligible(m_GM, ids))
					continue;

				m.m_sWhyNow = reason.text;
				m.m_vReasonAt = Vector(reason.x, 0, reason.z);
				if (!StartMission(m))
					continue;

				m_aReasonsUsed.Insert(reason.key + "|" + reason.subject);
				m_fReasonLaneAt = m_fClock + ss.m_fReasonGapMinutes * 60;
				RZ_Log.Info(string.Format("Story: reason %1 (%2) started %3 '%4' at %5", reason.key, reason.subject, m.m_iId, m.m_sTitle, m.GetLocationName(m_GM)));
				return;
			}
		}

		// Nothing answered now: look again in a minute
		m_fReasonLaneAt = m_fClock + 60;
	}

	// ================================================================ Finishing and rewards
	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_Mission m)
	{
		m_GM.GetSpawner().ReleaseMission(m.m_iId);
		m_GM.RemoveMapMarker(m.m_iMarkerId);
		m.m_iMarkerId = -1;

		array<int> paid = {};
		bool success = m.m_eState == RZ_EMissionState.SUCCEEDED;
		// Who was on it before anyone who stepped out is dropped: the notebook's past jobs
		array<int> before = {};
		before.InsertAll(m.m_aParticipants);
		if (m.m_bWithdrawn)
		{
			RZ_JobHistory.MissionEnded(m_GM, m, false, before);
			// Called off by the world: no pay, no blame, no strength or morale change, no leader
			// memory. Only the story that started it hears that it's over
			m.NotifyInterested(m_GM, m.m_sTitle, m.m_sWithdrawText);
			RZ_Log.Info(string.Format("Mission %1 '%2' called off: %3", m.m_iId, m.m_sTitle, m.m_sWithdrawText));
			if (m_GM.GetStoryArcs())
				m_GM.GetStoryArcs().OnMissionFinished(m, false);

			if (m_GM.GetQuestlines())
				m_GM.GetQuestlines().OnMissionFinished(m, false);

			m_GM.GetFacts().ReleaseAll(RZ_WorldFacts.MissionOwner(m.m_iId));
			RZ_StoryGroups.Release(m.m_iId);
			if (m.m_Job)
				m.m_Job.Cleanup();

			m.Cleanup(m_GM);
			return;
		}

		// Leave: whoever stepped out isn't paid or blamed
		if (m.m_Story)
			m.m_Story.m_Deciding.DropLeft(m_GM, m.m_aParticipants);

		RZ_JobHistory.MissionEnded(m_GM, m, success, before);

		if (success)
		{
			m.OnSuccess(m_GM);
			RZ_StoryKit.PayOffline(m_GM, m);
			foreach (int playerId : m.m_aParticipants)
			{
				if (!m.IsEligible(m_GM, playerId))
				{
					m_GM.NotifyPlayerAs(playerId, m.m_sTitle, string.Format("%1 didn't hire you for this. You get nothing from them.", RZ_Factions.GetName(m.m_Req.m_eGiver)), RZ_ENoticeTag.MISSION);
					continue;
				}

				// Jobs only by interaction: nobody offered it to you or your group
				if (!m.IsHired(m_GM, playerId))
				{
					m_GM.NotifyPlayerAs(playerId, m.m_sTitle, string.Format("Nobody asked you to. %1 pays the people it hired.", RZ_Factions.GetName(m.m_Req.m_eGiver)), RZ_ENoticeTag.MISSION);
					continue;
				}

				RewardPlayer(m, playerId);
				paid.Insert(playerId);
			}

			if (!m.m_bContract)
				m_GM.NotifyAllAs("Event over", m.GetSuccessText(m_GM), RZ_ENoticeTag.EVENT);
		}
		else
		{
			m.OnFailed(m_GM);
			// The end card: "Job failed: Recover the Bus", "Quest called off: ..." (RZ_JobCards)
			string how = "failed";
			if (m.m_bWithdrawn)
				how = "called off";

			m.NotifyInterested(m_GM, string.Format("%1 %2: %3", m.KindWord(), how, m.m_sTitle), m.GetFailText(m_GM));
			string fallout = RZ_MissionReasons.FalloutLine(m.Type());
			if (!fallout.IsEmpty() && !m.m_bWithdrawn)
				m.NotifyInterested(m_GM, m.m_sTitle, fallout);
		}

		// Leader memory, loyalty drift and discovery of rogue jobs
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().OnMissionFinished(m, success, paid);

		// Faction strength and morale
		m_GM.OnContractFinished(m, success);

		// The Tier 0 audit: the job's record at its place (`t0.<key>`)
		if (success)
			RZ_MissionReasons.Tier0Ended(m, true, paid);
		else
			RZ_MissionReasons.Tier0Ended(m, false, m.m_aParticipants);

		// A work job done in a town: the town tally
		RZ_WorkRows.Ended(m, success);

		// A found hook at one site in five: a stranger or an old mark (docs/ERRANDS.md, "Strangers")
		if (m_GM.GetErrands() && m_GM.GetErrands().IsOn() && !m.m_bWithdrawn)
			m_GM.GetErrands().GetStrangers().JobEnded(m);

		// The town log: what was done here, and by whom (World/RZ_WorldWriters.c)
		if (success)
			RZ_WorldWriters.MissionEnded(m, true, paid);
		else
			RZ_WorldWriters.MissionEnded(m, false, m.m_aParticipants);

		RZ_ReasonPosts.Ended(m_GM, m, success);
		// A lying brief's outcome names it, either way
		RZ_BriefLies.OnFinished(m_GM, m, success);

		// Who benefits: the others who did the work, when it went another way
		if (success && m.m_Story)
			m.m_Story.m_Parties.PayOthers(m_GM, m);

		if (m.m_Story)
			m.m_Story.Cleanup();

		if (m.m_Choice)
			m.m_Choice.Cleanup();

		if (m.m_Job)
			m.m_Job.Cleanup();

		// Whatever it held (RZ_WorldFacts claims) is free again
		m_GM.GetFacts().ReleaseAll(RZ_WorldFacts.MissionOwner(m.m_iId));
		RZ_StoryGroups.Release(m.m_iId);
		m.Cleanup(m_GM);
	}

	//------------------------------------------------------------------------------------------------
	protected void RewardPlayer(RZ_Mission m, int playerId)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_iMissionsCompleted++;
		profile.m_iLifeMissions++;
		if (m_GM.GetHall())
			m_GM.GetHall().AddMission(playerId);

		string text = m.m_sTitle + " is done.";
		if (m.m_bHasGiver)
		{
			RZ_Background bg = m_GM.GetPlayerBackground(playerId);
			float mult = bg.m_fMissionRewardMult * m.m_fRewardScale;
			if (m.HasBonus(m_GM, playerId))
				mult = mult * m.m_fBonusFameMult;

			// The leader who gave it: Greedy pays more Favor, a Paranoid one counts Fame for less...
			float fameMult = 1;
			float favorMult = 1;
			RZ_Leaders leaders = m_GM.GetLeaders();
			if (leaders && m.m_iLeaderId >= 0 && !m.m_bExposed)
			{
				fameMult = leaders.FameMult(m.m_iLeaderId, playerId, m.m_bAmends);
				favorMult = leaders.FavorMult(m.m_iLeaderId);
			}

			int fame = RZ_Util.RoundToInt(m.m_iFameReward * mult * fameMult);
			int favor = RZ_Util.RoundToInt(m.m_iFavorReward * m.m_fRewardScale * favorMult);
			string giverName = RZ_Factions.GetName(m.m_eGiver);

			// A switch (who benefits) pays at most the base, and its Fame doesn't count toward season Fame
			int fameFlags = 0;
			if (m.m_bSwitched)
			{
				fame = Math.Min(fame, m.m_iFameReward);
				favor = Math.Min(favor, m.m_iFavorReward);
				fameFlags = RZ_RepFlag.SWITCH;
			}

			// Amends work Infamy off through the one forgiveness budget (30 a life); once it's spent,
			// the job pays ordinary Fame, rival shares and all
			bool forgiven = false;
			// Amends need clean hands: Press, Trick, a switch or hiding something voids the work-off
			bool clean = !m.m_Story || !m.m_Story.m_Deciding.IsDirty(m_GM, playerId);
			if (m.m_bAmends && clean && fame > 0 && RZ_Reputation.IsWatchedOrWorse(m_GM.GetTier(playerId, m.m_eGiver)))
			{
				int off = m_GM.Forgive(playerId, m.m_eGiver, fame, "amends contract");
				if (off > 0)
				{
					forgiven = true;
					text += string.Format("\n%1 is willing to forget a little: -%2 Infamy.", giverName, off);
				}
				else
				{
					text += "\n" + m_GM.ForgiveSpentText(m.m_eGiver) + " The work counts as Fame instead.";
				}
			}

			if (!forgiven && fame > 0)
			{
				// An explicit rival replaces the general rival table, so it isn't counted twice
				m_GM.AddFame(playerId, m.m_eGiver, fame, !m.m_bHasRival, fameFlags, m.DeedPhrase(m_GM));
				text += string.Format("\n+%1 Fame with %2.", fame, giverName);
			}

			if (favor > 0)
			{
				m_GM.AddFavor(playerId, m.m_eGiver, favor);
				text += string.Format("\n+%1 Favor (%2 total).", favor, profile.GetFavor(m.m_eGiver));
			}

			int fameGiven = 0;
			if (!forgiven)
				fameGiven = fame;

			RZ_StoryStats.Paid(m, fameGiven, favor);

			if (m.m_bHasRival && m.m_iRivalInfamy > 0)
			{
				// A truce halves it, a feud makes it worse (docs/REPUTATION.md, "Truces and feuds")
				int infamy = Math.Max(1, RZ_Util.RoundToInt(m.m_iRivalInfamy * m.m_fRewardScale * m_GM.RivalMult(m.m_eGiver, m.m_eRival)));
				m_GM.AddInfamy(playerId, m.m_eRival, infamy, "mission rival", RZ_RepFlag.SHARE);
				text += string.Format("\n%1 won't forget this: +%2 Infamy.", RZ_Factions.GetName(m.m_eRival), infamy);
				if (leaders)
					leaders.OnRivalHurt(m.m_eRival, m.m_vPos, playerId);
			}
		}

		// The deeds store: a job done for the giver, against the rival
		if (m.m_bHasGiver)
		{
			int against = -1;
			if (m.m_bHasRival)
				against = m.m_eRival;

			RZ_Deeds.AddJob(m_GM, playerId, m.m_eGiver, against, m.m_sTitle);
		}

		m.OnRewardPlayer(m_GM, playerId);
		m_GM.NotifyPlayerAs(playerId, string.Format("%1 done: %2", m.KindWord(), m.m_sTitle), text, m.NoticeTag());
	}
}
