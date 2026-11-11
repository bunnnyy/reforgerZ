//! Keeps world events and faction contracts running.
//!
//! - World events: announced to everyone; anyone who takes part shares the reward.
//! - Contracts: offered only to eligible players (RZ_MissionRequirements), one
//!   offer per player at a time so the hint queue isn't flooded. There's no
//!   explicit "accept" step: eligible players who take part get paid, and
//!   ineligible players who turn up get nothing.
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

	static const float PROP_CLEAR = 25;	// no building this close (m) to a running mission's props has a lock

	protected static const float EVENT_COOLDOWN = 150;
	protected static const float CONTRACT_COOLDOWN = 75;
	protected static const float OFFER_GAP = 60;			// min seconds between two contract offers to one player
	protected static const int MAX_CONTRACTS = 10;

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

		OfferContracts(ids, players);
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

		if (!claimed || !mission.Setup(m_GM))
		{
			// Whatever it spawned or claimed before it gave up goes again
			mission.m_eState = RZ_EMissionState.FAILED;
			m_GM.GetSpawner().ReleaseMission(mission.m_iId);
			facts.ReleaseAll(owner);
			mission.Cleanup(m_GM);
			return false;
		}

		// Briefs often open with a place or a name ("the chemist's in ...")
		mission.m_sBrief = RZ_Util.Capitalize(mission.m_sBrief);
		m_aActive.Insert(mission);
		m_fSpotsAt = -1; // its props count from now
		OnStarted(mission);
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

		m_GM.NotifyPlayer(playerId, "The Red Hand", "Nobody comes out to meet you. The Red Hand doesn't deal with you.");
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
	//! Each player gets at most one new offer per OFFER_GAP seconds.
	protected void OfferContracts(array<int> ids, array<IEntity> players)
	{
		foreach (int i, int playerId : ids)
		{
			float last;
			if (m_mLastOffer.Find(playerId, last) && m_fClock - last < OFFER_GAP)
				continue;

			foreach (RZ_Mission m : m_aActive)
			{
				if (!m.m_bContract || m.m_bInPersonOnly || m.m_aOfferedTo.Contains(playerId) || !m.IsEligible(m_GM, playerId))
					continue;

				Offer(m, playerId, players[i]);
				m_mLastOffer.Set(playerId, m_fClock);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Offer(RZ_Mission m, int playerId, IEntity player)
	{
		m.m_aOfferedTo.Insert(playerId);
		vector from = vector.Zero;
		if (player)
			from = player.GetOrigin();

		string where = m.GetWhereTextFor(m_GM, from, playerId);

		string bonus = "";
		if (m.HasBonus(m_GM, playerId))
			bonus = "\nYour background gives you an edge on this one.";

		// In the voice of the leader who gives it (intro and outro by trait)
		string brief = m.GetBrief(m_GM);
		if (m_GM.GetLeaders())
			brief = m_GM.GetLeaders().VoiceBrief(m, playerId);

		m_GM.NotifyPlayer(playerId, "Contract: " + RZ_Factions.GetName(m.m_Req.m_eGiver),
			string.Format("%1\n%2\n(%3)%4", m.m_sTitle, brief, where, bonus));
	}

	//------------------------------------------------------------------------------------------------
	//! Eligible open contracts from one faction, for its quartermaster. Marks them as offered.
	string ListContractsFor(int playerId, RZ_EFaction faction, IEntity player)
	{
		string text = "";
		foreach (RZ_Mission m : m_aActive)
		{
			if (!m.m_bContract || m.m_Req.m_eGiver != faction || !m.IsEligible(m_GM, playerId))
				continue;

			// A leader's personal job is only mentioned at their own seat
			if (!m.CanOfferAt(m_GM, player.GetOrigin()))
				continue;

			if (!m.m_aOfferedTo.Contains(playerId))
				m.m_aOfferedTo.Insert(playerId);

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
	//! Text for a newly spawned player: world events, plus contracts they qualify for.
	//! What goes in a player's journal: world events, contracts they could take (as on the radio),
	//! personal jobs offered to them, and anything they're taking part in. Marks nothing as offered.
	void ListFor(int playerId, notnull array<RZ_Mission> result)
	{
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_eState != RZ_EMissionState.ACTIVE)
				continue;

			bool show = !m.m_bContract || m.m_aParticipants.Contains(playerId);
			if (!show && m.m_bInPersonOnly)
				show = m.m_aOfferedTo.Contains(playerId);
			else if (!show)
				show = m.IsEligible(m_GM, playerId);

			if (show)
				result.Insert(m);
		}
	}

	//------------------------------------------------------------------------------------------------
	string DescribeActiveFor(IEntity player, int playerId)
	{
		string text = "";
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_bContract)
				continue;

			text += string.Format("%1: %2\n", m.m_sTitle, m.GetWhereTextFor(m_GM, player.GetOrigin(), playerId));
		}

		string contracts = "";
		foreach (RZ_Mission c : m_aActive)
		{
			if (!c.m_bContract || c.m_bInPersonOnly || !c.IsEligible(m_GM, playerId))
				continue;

			if (!c.m_aOfferedTo.Contains(playerId))
				c.m_aOfferedTo.Insert(playerId);

			contracts += string.Format("%1 (%2): %3\n", c.m_sTitle, RZ_Factions.GetShortName(c.m_Req.m_eGiver), c.GetWhereTextFor(m_GM, player.GetOrigin(), playerId));
		}

		if (!contracts.IsEmpty())
			text += "\nWork for you:\n" + contracts;

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

			// The giver's stronghold leader leans toward some kinds of work (traits, +-20%)
			if (contract && m_GM.GetLeaders())
				w = w * m_GM.GetLeaders().ContractWeightMult(m);

			// ...and so does what the faction needs right now (lost trucks, deaths, research)
			if (contract && m_GM.GetFactionLife())
				w = w * m_GM.GetFactionLife().ContractWeightMult(m);

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

	// ================================================================ Finishing and rewards
	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_Mission m)
	{
		m_GM.GetSpawner().ReleaseMission(m.m_iId);
		m_GM.RemoveMapMarker(m.m_iMarkerId);
		m.m_iMarkerId = -1;

		array<int> paid = {};
		bool success = m.m_eState == RZ_EMissionState.SUCCEEDED;
		if (m.m_bWithdrawn)
		{
			// Called off by the world: no pay, no blame, no strength or morale change, no leader
			// memory. Only the story that started it hears that it's over
			m.NotifyInterested(m_GM, m.m_sTitle, m.m_sWithdrawText);
			RZ_Log.Info(string.Format("Mission %1 '%2' called off: %3", m.m_iId, m.m_sTitle, m.m_sWithdrawText));
			if (m_GM.GetStoryArcs())
				m_GM.GetStoryArcs().OnMissionFinished(m, false);

			if (m_GM.GetQuestlines())
				m_GM.GetQuestlines().OnMissionFinished(m, false);

			m_GM.GetFacts().ReleaseAll(RZ_WorldFacts.MissionOwner(m.m_iId));
			m.Cleanup(m_GM);
			return;
		}

		if (success)
		{
			m.OnSuccess(m_GM);
			foreach (int playerId : m.m_aParticipants)
			{
				if (!m.IsEligible(m_GM, playerId))
				{
					m_GM.NotifyPlayer(playerId, m.m_sTitle, string.Format("%1 didn't hire you for this. You get nothing from them.", RZ_Factions.GetName(m.m_Req.m_eGiver)));
					continue;
				}

				RewardPlayer(m, playerId);
				paid.Insert(playerId);
			}

			if (!m.m_bContract)
				m_GM.NotifyAll("Event over", m.GetSuccessText(m_GM));
		}
		else
		{
			m.OnFailed(m_GM);
			m.NotifyInterested(m_GM, "Mission failed", m.GetFailText(m_GM));
		}

		// Leader memory, loyalty drift and discovery of rogue jobs
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().OnMissionFinished(m, success, paid);

		// Faction strength and morale
		m_GM.OnContractFinished(m, success);

		// Whatever it held (RZ_WorldFacts claims) is free again
		m_GM.GetFacts().ReleaseAll(RZ_WorldFacts.MissionOwner(m.m_iId));
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

			if (m.m_bAmends && RZ_Reputation.IsWatchedOrWorse(m_GM.GetTier(playerId, m.m_eGiver)))
			{
				m_GM.AddInfamy(playerId, m.m_eGiver, -fame);
				text += string.Format("\n%1 is willing to forget a little: -%2 Infamy.", giverName, fame);
			}
			else if (fame > 0)
			{
				// An explicit rival replaces the general rival table, so it isn't counted twice
				m_GM.AddFame(playerId, m.m_eGiver, fame, !m.m_bHasRival);
				text += string.Format("\n+%1 Fame with %2.", fame, giverName);
			}

			if (favor > 0)
			{
				m_GM.AddFavor(playerId, m.m_eGiver, favor);
				text += string.Format("\n+%1 Favor (%2 total).", favor, profile.GetFavor(m.m_eGiver));
			}

			if (m.m_bHasRival && m.m_iRivalInfamy > 0)
			{
				// A truce halves it, a feud makes it worse (docs/REPUTATION.md, "Truces and feuds")
				int infamy = Math.Max(1, RZ_Util.RoundToInt(m.m_iRivalInfamy * m.m_fRewardScale * m_GM.RivalMult(m.m_eGiver, m.m_eRival)));
				m_GM.AddInfamy(playerId, m.m_eRival, infamy);
				text += string.Format("\n%1 won't forget this: +%2 Infamy.", RZ_Factions.GetName(m.m_eRival), infamy);
				if (leaders)
					leaders.OnRivalHurt(m.m_eRival, m.m_vPos, playerId);
			}
		}

		m.OnRewardPlayer(m_GM, playerId);
		m_GM.NotifyPlayer(playerId, "Mission complete", text);
	}
}
