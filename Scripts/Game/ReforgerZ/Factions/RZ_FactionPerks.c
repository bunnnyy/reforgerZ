//! Everything factions do to and for players outside missions (docs/REPUTATION.md):
//! quartermasters, perks, hunter squads, bounties. Server only, owned by the game mode.
//!
//! Implemented perks:
//!   Haven Known          tier message (safehouses let you in: flavour for now)
//!   Haven Trusted        Neighbours: direction to the nearest job, on spawn and every 10 min
//!   Bastion Trusted      Requisition: once an hour, ammo or rations at half Favor (RZ_RequisitionAction)
//!   Red Hand Known       toll pass (flavour message for now)
//!   Halcyon Trusted      sample scanner: specials within 300 m, every minute
//!   Halcyon Champion     +25% research from your samples (RZ_CureResearch)
//!   Halcyon Hunted       the lab takes your samples without Fame (RZ_CureResearch)
//! Consequences:
//!   Hunted / Kill on Sight by Bastion or the Red Hand: hunter squads
//!   Kill on Sight by anyone: public bounty, claimed by whoever is near when you die
class RZ_FactionPerks
{
	protected RZ_GameModeComponent m_GM;
	protected float m_fClock;

	// Quartermasters
	protected ref array<vector> m_aQmPos = {};
	protected ref array<bool> m_aHasQm = {};
	protected ref array<IEntity> m_aSpawnedQms = {};
	protected ref map<string, float> m_mGreeted = new map<string, float>();

	// Perk timers
	protected ref map<string, float> m_mRequisitionAt = new map<string, float>();
	protected float m_fScanTimer;
	protected float m_fNeighbourTimer;

	// Consequences
	protected ref array<IEntity> m_aHunters = {};
	protected ref array<float> m_aHunterExpiry = {};
	protected float m_fHunterTimer;
	protected float m_fBountyTimer;
	protected ref map<int, vector> m_mLastAlivePos = new map<int, vector>();

	protected static const float QM_GREET_RADIUS = 15;
	protected static const float QM_GREET_COOLDOWN = 300;
	protected static const float SCAN_INTERVAL = 60;
	protected static const float SCAN_RADIUS = 300;
	protected static const float NEIGHBOUR_INTERVAL = 600;
	protected static const float REQUISITION_COOLDOWN = 3600;
	protected static const float HUNTER_DISTANCE = 400;
	protected static const float HUNTER_LIFETIME = 900;
	protected static const float BOUNTY_CLAIM_RADIUS = 150;
	protected static const int BOUNTY_FAVOR = 50;
	protected static const int BOUNTY_FAME = 10;

	//------------------------------------------------------------------------------------------------
	void RZ_FactionPerks(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		for (int i = 0; i < RZ_Factions.COUNT; i++)
		{
			m_aQmPos.Insert(vector.Zero);
			m_aHasQm.Insert(false);
		}
	}

	// ================================================================ Quartermasters
	//------------------------------------------------------------------------------------------------
	//! Positions from the save file (x/z per faction, set = 1 if valid).
	void RestoreQuartermasters(array<float> xs, array<float> zs, array<int> isSet)
	{
		if (!xs || !zs || !isSet)
			return;

		for (int i = 0; i < RZ_Factions.COUNT; i++)
		{
			if (i >= xs.Count() || i >= zs.Count() || i >= isSet.Count() || isSet[i] == 0)
				continue;

			m_aQmPos[i] = RZ_Util.GroundPos(Vector(xs[i], 0, zs[i]));
			m_aHasQm[i] = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	void SaveQuartermasters(notnull array<float> xs, notnull array<float> zs, notnull array<int> isSet)
	{
		for (int i = 0; i < RZ_Factions.COUNT; i++)
		{
			vector q = m_aQmPos[i];
			xs.Insert(q[0]);
			zs.Insert(q[2]);
			if (m_aHasQm[i])
				isSet.Insert(1);
			else
				isSet.Insert(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Placed quartermasters win. Otherwise spawn each configured prefab at its saved
	//! position, or in a town that suits the faction.
	void SetupQuartermasters()
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			RZ_QuartermasterComponent placed = RZ_QuartermasterComponent.Find(f);
			if (placed)
			{
				m_aQmPos[f] = placed.GetOwner().GetOrigin();
				m_aHasQm[f] = true;
				continue;
			}

			ResourceName prefab = m_GM.GetQuartermasterPrefab(f);
			if (prefab.IsEmpty())
				continue;

			if (!m_aHasQm[f])
			{
				vector pos;
				if (!ChooseQuartermasterPos(f, pos))
					continue;

				m_aQmPos[f] = pos;
				m_aHasQm[f] = true;
			}

			IEntity qm = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(m_aQmPos[f]), Math.RandomFloat(0, 360));
			if (qm)
				m_aSpawnedQms.Insert(qm);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool ChooseQuartermasterPos(RZ_EFaction f, out vector pos)
	{
		if (f == RZ_EFaction.INSTITUTE && m_GM.HasLab())
		{
			pos = RZ_Mission.LandPointNear(m_GM.GetLabPosition(), 10, 20);
			return true;
		}

		int idx = -1;
		switch (f)
		{
			case RZ_EFaction.SURVIVORS:
				idx = RZ_Mission.PickSafestSettlement(m_GM);
				break;
			case RZ_EFaction.MILITARY:
				idx = PickTown(2, 0, 60);
				break;
			case RZ_EFaction.BANDITS:
				idx = PickTown(1, 20, 75);
				break;
		}

		if (idx < 0)
			idx = PickTown(1, 0, 100);

		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return false;

		pos = RZ_Mission.LandPointNear(s.m_vCenter, 10, Math.Max(20, s.m_fRadius * 0.4));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A random town of at least `minSize` in the infection range that has no quartermaster yet.
	protected int PickTown(int minSize, float minInf, float maxInf)
	{
		array<int> options = {};
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (s.m_iSize < minSize || s.m_fInfection < minInf || s.m_fInfection > maxInf)
				continue;

			bool taken = false;
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (m_aHasQm[f] && RZ_Util.DistXZ(m_aQmPos[f], s.m_vCenter) < s.m_fRadius)
					taken = true;
			}

			if (!taken)
				options.Insert(i);
		}

		if (options.IsEmpty())
			return -1;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	bool GetQuartermasterPos(RZ_EFaction f, out vector pos)
	{
		pos = m_aQmPos[f];
		return m_aHasQm[f];
	}

	//------------------------------------------------------------------------------------------------
	void DeleteSpawned()
	{
		foreach (IEntity e : m_aSpawnedQms)
		{
			RZ_Util.DeleteUnit(e);
		}

		m_aSpawnedQms.Clear();
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		CheckDeaths(ids, players);
		GreetAtQuartermasters(ids, players);

		m_fScanTimer += dt;
		if (m_fScanTimer >= SCAN_INTERVAL)
		{
			m_fScanTimer = 0;
			SampleScanner(ids, players);
		}

		m_fNeighbourTimer += dt;
		if (m_fNeighbourTimer >= NEIGHBOUR_INTERVAL)
		{
			m_fNeighbourTimer = 0;
			foreach (int i, int playerId : ids)
			{
				Neighbours(playerId, players[i]);
			}
		}

		UpdateHunters(dt, ids, players);

		m_fBountyTimer += dt;
		if (m_fBountyTimer >= m_GM.GetBountyMinutes() * 60)
		{
			m_fBountyTimer = 0;
			AnnounceBounties(ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned(int playerId, IEntity ent)
	{
		Neighbours(playerId, ent);
		if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
			m_GM.NotifyPlayer(playerId, "The Red Hand", "The Red Hand knows your face. Their roadblocks will let you pass for a toll.");
	}

	//------------------------------------------------------------------------------------------------
	//! Tell the player what a new tier opens up or brings down on them.
	void OnTierChanged(int playerId, RZ_EFaction faction, RZ_ETier before, RZ_ETier after)
	{
		string text = GetTierPerkText(faction, after);
		if (!text.IsEmpty())
			m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(faction) + ": " + RZ_Reputation.GetTierName(after), text);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetTierPerkText(RZ_EFaction faction, RZ_ETier tier)
	{
		if (tier == RZ_ETier.KILL_ON_SIGHT)
			return "They want you dead. There's a price on your head, and anyone near you when you fall can claim it.";

		if (RZ_Reputation.IsHunted(tier))
		{
			if (faction == RZ_EFaction.MILITARY || faction == RZ_EFaction.BANDITS)
				return "They're hunting you. Expect armed squads to come looking.";
			if (faction == RZ_EFaction.INSTITUTE)
				return "The lab will still take your samples, but you'll get nothing for them.";
			return "Their doors are barred to you.";
		}

		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (tier == RZ_ETier.KNOWN)
					return "Haven safehouses will let you in. Their quartermaster sells you basic supplies.";
				if (tier == RZ_ETier.TRUSTED)
					return "Neighbours: the Haven will point you to the nearest job.";
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				if (tier == RZ_ETier.KNOWN)
					return "Bastion checkpoints will let you through without a search.";
				if (tier == RZ_ETier.TRUSTED)
					return "Requisition: once an hour, the Bastion quartermaster lets you draw ammo or rations at half the Favor cost.";
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				if (tier == RZ_ETier.KNOWN)
					return "Toll pass: Red Hand roadblocks will let you pass for a toll.";
				if (tier == RZ_ETier.TRUSTED)
					return "The black market's better shelves are open to you.";
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (tier == RZ_ETier.KNOWN)
					return "Halcyon's stores are open to you.";
				if (tier == RZ_ETier.TRUSTED)
					return "Sample scanner: Halcyon will tell you when special infected are close.";
				if (tier == RZ_ETier.CHAMPION)
					return "Lab access: your samples are worth 25% more research.";
				break;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected void GreetAtQuartermasters(array<int> ids, array<IEntity> players)
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (!m_aHasQm[f])
				continue;

			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), m_aQmPos[f]) > QM_GREET_RADIUS)
					continue;

				int playerId = ids[i];
				string key = string.Format("%1:%2", playerId, f);
				float last;
				if (m_mGreeted.Find(key, last) && m_fClock - last < QM_GREET_COOLDOWN)
					continue;

				m_mGreeted.Set(key, m_fClock);
				string work = m_GM.GetMissions().ListContractsFor(playerId, f, p);
				if (!work.IsEmpty())
					m_GM.NotifyPlayer(playerId, "Work from " + RZ_Factions.GetName(f), work);
			}
		}
	}

	// ================================================================ Perks
	//------------------------------------------------------------------------------------------------
	//! Halcyon Trusted: special infected within 300 m.
	protected void SampleScanner(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(ids[i], RZ_EFaction.INSTITUTE), RZ_ETier.TRUSTED))
				continue;

			array<vector> found = {};
			m_GM.GetSpawner().GetSpecialsNear(p.GetOrigin(), SCAN_RADIUS, found);
			if (found.IsEmpty())
				continue;

			string text = "";
			foreach (int j, vector at : found)
			{
				if (j >= 4)
					break;

				if (j > 0)
					text += ", ";

				text += RZ_Util.DirectionText(p.GetOrigin(), at);
			}

			m_GM.NotifyPlayer(ids[i], "Sample scanner", "Special infected: " + text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Haven Trusted: the nearest job you'd be paid for.
	protected void Neighbours(int playerId, IEntity ent)
	{
		if (!ent || !RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.TRUSTED))
			return;

		RZ_Mission m = m_GM.GetMissions().FindNearestFor(playerId, ent.GetOrigin());
		if (m)
			m_GM.NotifyPlayer(playerId, "Haven neighbours", string.Format("Nearest job: %1, %2.", m.m_sTitle, m.GetWhereText(m_GM, ent.GetOrigin())));
	}

	//------------------------------------------------------------------------------------------------
	//! Requisition price: half the full Favor cost, rounded up.
	static int RequisitionPrice(int fullCost)
	{
		if (fullCost <= 0)
			return 0;

		return (fullCost + 1) / 2;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion Trusted: once an hour per player (by identity, so it survives respawns), one
	//! item from the quartermaster at half price. Nothing is free: it's paid in Favor.
	bool TryRequisition(int playerId, IEntity owner, IEntity user, ResourceName prefab, string itemName, int fullCost)
	{
		string bastion = RZ_Factions.GetName(RZ_EFaction.MILITARY);
		RZ_ETier tier = m_GM.GetTier(playerId, RZ_EFaction.MILITARY);
		if (!RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED) || RZ_Reputation.IsHunted(tier))
		{
			m_GM.NotifyPlayer(playerId, bastion, "Requisitions are for people Bastion trusts.");
			return false;
		}

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		if (m_mRequisitionAt.Find(ident, last) && m_fClock - last < REQUISITION_COOLDOWN)
		{
			int minutes = RZ_Util.RoundToInt((REQUISITION_COOLDOWN - (m_fClock - last)) / 60);
			m_GM.NotifyPlayer(playerId, bastion, string.Format("Your next requisition is approved in about %1 minutes.", minutes));
			return false;
		}

		if (prefab.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, bastion, "The stores are empty. Nothing to requisition.");
			return false;
		}

		int price = RequisitionPrice(fullCost);
		if (!m_GM.SpendFavor(playerId, RZ_EFaction.MILITARY, price))
		{
			m_GM.NotifyPlayer(playerId, bastion, string.Format("Requisitioned %1 still costs %2 Favor. You have %3.", itemName, price, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.MILITARY)));
			return false;
		}

		if (!RZ_QuartermasterHelper.SpawnNear(prefab, owner, user))
		{
			m_GM.AddFavor(playerId, RZ_EFaction.MILITARY, price); // refund
			return false;
		}

		m_mRequisitionAt.Set(ident, m_fClock);
		m_GM.NotifyPlayer(playerId, bastion, string.Format("Requisition approved: %1, signed out of Bastion stock. %2 Favor left.", itemName, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.MILITARY)));
		return true;
	}

	// ================================================================ Hunter squads
	//------------------------------------------------------------------------------------------------
	protected void UpdateHunters(float dt, array<int> ids, array<IEntity> players)
	{
		// Squads give up after a while (and are removed if nobody is near them)
		for (int i = m_aHunters.Count() - 1; i >= 0; i--)
		{
			IEntity h = m_aHunters[i];
			if (!h || RZ_Util.CountAlive(h) == 0)
			{
				m_aHunters.Remove(i);
				m_aHunterExpiry.Remove(i);
				continue;
			}

			if (m_fClock < m_aHunterExpiry[i])
				continue;

			if (RZ_Util.NearestDistance(RZ_Util.GetUnitPosition(h), players) > 300)
			{
				RZ_Util.DeleteUnit(h);
				m_aHunters.Remove(i);
				m_aHunterExpiry.Remove(i);
			}
		}

		float interval = m_GM.GetHunterSquadMinutes() * 60;
		if (interval <= 0)
			return;

		m_fHunterTimer += dt;
		if (m_fHunterTimer < interval)
			return;

		m_fHunterTimer = 0;
		array<int> hunters = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS};
		foreach (int j, IEntity p : players)
		{
			foreach (int f : hunters)
			{
				if (!RZ_Reputation.IsHunted(m_GM.GetTier(ids[j], f)))
					continue;

				ResourceName prefab = m_GM.GetHunterPrefab(f);
				if (prefab.IsEmpty())
					continue;

				SendHunters(ids[j], p, f, prefab);
				break; // one squad per player per round
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SendHunters(int playerId, IEntity target, RZ_EFaction faction, ResourceName prefab)
	{
		vector at = target.GetOrigin();
		vector from = RZ_Mission.LandPointNear(at, HUNTER_DISTANCE * 0.9, HUNTER_DISTANCE * 1.1);
		IEntity squad = RZ_Util.SpawnPrefab(prefab, from);
		if (!squad)
			return;

		RZ_Util.SendGroupTo(squad, at, m_GM.GetSpawner().GetMoveWaypointPrefab());
		m_aHunters.Insert(squad);
		m_aHunterExpiry.Insert(m_fClock + HUNTER_LIFETIME);

		string text = "A Bastion patrol has your last known position.";
		if (faction == RZ_EFaction.BANDITS)
			text = "A Red Hand crew is hunting you. They're close.";

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(faction), text + " (" + RZ_Util.DirectionText(at, from) + ")");
	}

	// ================================================================ Bounties
	//------------------------------------------------------------------------------------------------
	protected void AnnounceBounties(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (m_GM.GetTier(ids[i], f) != RZ_ETier.KILL_ON_SIGHT)
					continue;

				RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(p.GetOrigin()));
				string where = RZ_SettlementScanner.GridReference(p.GetOrigin());
				if (s)
					where = s.m_sName;

				string text = string.Format("%1 has put a bounty on %2. Last seen near %3. Be near when they die to claim %4 Favor.",
					RZ_Factions.GetName(f), RZ_Util.PlayerName(ids[i]), where, BOUNTY_FAVOR);
				foreach (int other : ids)
				{
					if (other != ids[i])
						m_GM.NotifyPlayer(other, "Bounty", text);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Compares this tick's living players with the last tick's. A player who was
	//! alive, is still connected, and isn't alive any more has died.
	protected void CheckDeaths(array<int> ids, array<IEntity> players)
	{
		array<int> connected = {};
		GetGame().GetPlayerManager().GetPlayers(connected);

		array<int> gone = {};
		foreach (int playerId, vector pos : m_mLastAlivePos)
		{
			if (!ids.Contains(playerId))
				gone.Insert(playerId);
		}

		foreach (int deadId : gone)
		{
			vector at = m_mLastAlivePos.Get(deadId);
			m_mLastAlivePos.Remove(deadId);
			if (connected.Contains(deadId))
				OnPlayerDied(deadId, at, ids, players);
		}

		foreach (int i, IEntity p : players)
		{
			m_mLastAlivePos.Set(ids[i], p.GetOrigin());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDied(int victim, vector at, array<int> ids, array<IEntity> players)
	{
		m_GM.GetMissions().OnPlayerDied(victim, at);

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (m_GM.GetTier(victim, f) != RZ_ETier.KILL_ON_SIGHT)
				continue;

			array<int> claimants = {};
			RZ_Mission.GetPlayersNear(ids, players, at, BOUNTY_CLAIM_RADIUS, claimants);
			claimants.RemoveItem(victim);
			if (claimants.IsEmpty())
				continue;

			foreach (int claimant : claimants)
			{
				m_GM.AddFavor(claimant, f, BOUNTY_FAVOR);
				m_GM.AddFame(claimant, f, BOUNTY_FAME, false);
				m_GM.NotifyPlayer(claimant, "Bounty claimed", string.Format("%1 pays you %2 Favor for %3.", RZ_Factions.GetName(f), BOUNTY_FAVOR, RZ_Util.PlayerName(victim)));
			}

			m_GM.NotifyAll("Bounty", string.Format("The %1 bounty on %2 has been claimed.", RZ_Factions.GetShortName(f), RZ_Util.PlayerName(victim)));
		}
	}
}
