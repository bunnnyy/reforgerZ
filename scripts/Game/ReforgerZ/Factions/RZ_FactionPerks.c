//! Everything factions do to and for players outside missions (docs/REPUTATION.md):
//! quartermasters, perks, hunter squads, bounties. Server only, owned by the game mode.
//!
//! Implemented perks:
//!   Haven Known          tier message (safehouses let you in: flavour for now)
//!   Haven Trusted        Neighbours: direction to the nearest job, on spawn and every 10 min
//!   Haven Champion       Safe Harbour: inside any safehouse, hunter squads lose your trail and
//!                        bounty notices give only a vague area. Remembered: when you die, a
//!                        memorial headline, the council reads your name, and your next character
//!                        is kin (one free treatment and a greeting at their first safehouse)
//!   Bastion Trusted      Requisition: once an hour, ammo or rations at half Favor (RZ_RequisitionAction),
//!                        from a Bastion outpost's truck-supplied stock while supply trucks run
//!   Bastion Known        pass Bastion's road checkpoints unsearched (RZ_Checkpoints)
//!   Bastion Champion     Fire Mission (RZ_FactionServices)
//!   Red Hand Known       toll pass: roadblocks take a toll instead of shooting (RZ_Checkpoints)
//!   Red Hand Trusted     Fence (RZ_FactionServices); roadblocks wave you through
//!   Red Hand Champion    Bounty board: a bounty on whoever last killed you (PostBounty)
//!   Halcyon Trusted      sample scanner: specials within 300 m, every minute; the medic
//!   Halcyon Champion     +25% research from your samples (RZ_CureResearch); the immunity booster
//!   Halcyon Hunted       the lab takes your samples without Fame (RZ_CureResearch)
//! Consequences:
//!   Hunted / Kill on Sight by Bastion or the Red Hand: hunter squads
//!   Kill on Sight by anyone: public bounty, paid once to the player who kills you
//!   A posted bounty (the board): paid the same way, killer only, never to the poster or a friend
//!   Hunted by the Haven: a hungry safehouse may sell your rough location (LeakLocation)
//!   Hunted by anyone: that faction's AI really shoots you (RZ_Hostility); a hunter squad
//!   is always hostile to the player it was sent after
//------------------------------------------------------------------------------------------------
//! A bounty a Red Hand Champion posted on the board. Kept by identity; not saved.
class RZ_PostedBounty
{
	string m_sPoster;
	string m_sPosterName;
	string m_sTarget;
	string m_sTargetName;
	int m_iPaid;			// what the poster paid
	int m_iPayout;			// what the killer gets
	int m_iUntilUnix;		// real time it comes down (saved)
}

//------------------------------------------------------------------------------------------------
//! What perks and services keep across a restart, by identity and real time (RZ_SaveData.perks).
class RZ_PerksSave
{
	ref array<ref RZ_PostedBounty> posted = {};
	ref array<string> killerVictims = {};		// victim identity -> killer identity and name
	ref array<string> killerIdents = {};
	ref array<string> killerNames = {};
	ref array<string> requisitionIdents = {};	// the Requisition cooldown: when each last signed out
	ref array<int> requisitionUnix = {};
	ref array<string> fenceIdents = {};			// the fence's hour: its start and what it paid
	ref array<int> fenceUnix = {};
	ref array<int> fenceEarned = {};
}

//------------------------------------------------------------------------------------------------
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
	protected ref map<string, int> m_mRequisitionAt = new map<string, int>();	// identity -> real time of the last one
	protected float m_fScanTimer;
	protected float m_fNeighbourTimer;

	// Consequences
	protected ref array<IEntity> m_aHunters = {};
	protected ref array<float> m_aHunterExpiry = {};
	protected ref array<int> m_aHunterTarget = {};		// who each squad was sent after
	protected ref array<bool> m_aHunterLost = {};		// lost the trail (Safe Harbour): walking away
	protected ref array<int> m_aHarbourTold = {};
	protected float m_fHunterTimer;
	protected float m_fBountyTimer;
	protected ref map<int, vector> m_mLastAlivePos = new map<int, vector>();
	protected ref map<int, IEntity> m_mLastAliveEnt = new map<int, IEntity>();	// the body, to read the killer from
	protected int m_iDeathVictim = -1;		// the death being handled now (KillerOf)
	protected int m_iDeathKiller = -1;

	// The bounty board (Red Hand Champion)
	protected ref array<ref RZ_PostedBounty> m_aPosted = {};
	protected ref map<string, string> m_mLastKiller = new map<string, string>();		// victim identity -> killer identity
	protected ref map<string, string> m_mLastKillerName = new map<string, string>();	// victim identity -> killer's name

	protected static const float QM_GREET_RADIUS = 15;
	protected static const float QM_GREET_COOLDOWN = 300;
	protected static const float SCAN_INTERVAL = 60;
	protected static const float SCAN_RADIUS = 300;
	protected static const float NEIGHBOUR_INTERVAL = 600;
	protected static const float REQUISITION_COOLDOWN = 3600;
	protected static const float HUNTER_DISTANCE = 400;
	protected static const int MAX_SQUADS_SERVER = 4;			// hunter squads out on the whole server at once
	protected static const float HUNTER_SIGHT = 700;			// no squad appears where a player this close could see it
	protected static const float HUNTER_LIFETIME = 900;
	protected static const float HUNTER_HOSTILE_RADIUS = 800;	// the squad is hostile to its target within this distance
	protected static const int BOUNTY_FAVOR = 50;
	protected static const int BOUNTY_FAME = 10;
	protected static const float LOST_TRAIL_SECONDS = 120;	// a squad that lost the trail gives up this soon
	protected static const int MAX_SQUADS_PER_TARGET = 2;

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

			IEntity qm = RZ_Util.SpawnPrefab(m_GM.PickLook(prefab, RZ_Looks.SeedAt(m_aQmPos[f])), RZ_Util.GroundPos(m_aQmPos[f]), Math.RandomFloat(0, 360));
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
	//! A random town of at least `minSize` in the infection range that has no quartermaster yet
	//! and isn't the lab's (Halcyon's stronghold).
	protected int PickTown(int minSize, float minInf, float maxInf)
	{
		array<int> options = {};
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (s.m_iSize < minSize || s.m_fInfection < minInf || s.m_fInfection > maxInf)
				continue;

			if (i == m_GM.GetLabSettlement())
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
			PruneGreetings();
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
		ExpirePostings();

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
		string shoots = "";
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility && RZ_Reputation.IsHunted(tier))
			shoots = hostility.DescribeHunted(faction);

		if (!shoots.IsEmpty())
			shoots = " " + shoots;

		if (tier == RZ_ETier.KILL_ON_SIGHT)
			return "They want you dead. There's a price on your head, and whoever kills you can claim it." + shoots;

		if (RZ_Reputation.IsHunted(tier))
		{
			if (faction == RZ_EFaction.MILITARY || faction == RZ_EFaction.BANDITS)
				return "They're hunting you. Expect armed squads to come looking." + shoots;
			if (faction == RZ_EFaction.INSTITUTE)
				return "The lab will still take your samples, but you'll get nothing for them." + shoots;
			return "Exile: the safehouse doors stay shut to you, their people warn others about you, and a hungry safehouse may sell your name." + shoots;
		}

		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (tier == RZ_ETier.KNOWN)
					return "Haven safehouses will let you in. Their quartermaster sells you basic supplies. House rules: don't fire near a safehouse, and tell them at the door if you're bitten.";
				if (tier == RZ_ETier.TRUSTED)
					return "Neighbours: the Haven will point you to the nearest job.";
				if (tier == RZ_ETier.CHAMPION)
					return "Safe Harbour: inside any safehouse, hunters lose your trail and bounty notices can only guess where you are. And if you fall, the Haven will remember you.";
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				if (tier == RZ_ETier.KNOWN)
					return "Bastion knows your face. Its posts will talk to you, its road checkpoints wave you through unsearched, and its quartermasters sell you the basics.";
				if (tier == RZ_ETier.TRUSTED)
					return "Requisition: once an hour, Bastion lets you draw ammo or rations at half the Favor cost, from an outpost the supply trucks have stocked.";
				if (tier == RZ_ETier.CHAMPION)
					return "Fire Mission: throw a Bastion signal smoke on a horde, well away from people, and the guns answer. Once an hour.";
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				if (tier == RZ_ETier.KNOWN)
					return "Toll pass: Red Hand roadblocks will let you pass for a toll.";
				if (tier == RZ_ETier.TRUSTED)
					return "The black market's better shelves are open to you, the fence will buy what you find, and the roadblocks wave you through.";
				if (tier == RZ_ETier.CHAMPION)
					return "Bounty board: you can put a price on whoever last killed you.";
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (tier == RZ_ETier.KNOWN)
					return "Halcyon's stores are open to you.";
				if (tier == RZ_ETier.TRUSTED)
					return "Sample scanner: Halcyon will tell you when special infected are close. Their medic will see you, for a price.";
				if (tier == RZ_ETier.CHAMPION)
					return "Lab access: your samples are worth 25% more research, and the immunity booster is yours if you can pay for it.";
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

	//------------------------------------------------------------------------------------------------
	//! Greetings whose cooldown has run out (players who left among them), so the map stays small.
	protected void PruneGreetings()
	{
		array<string> expired = {};
		foreach (string key, float last : m_mGreeted)
		{
			if (m_fClock - last >= QM_GREET_COOLDOWN)
				expired.Insert(key);
		}

		foreach (string old : expired)
		{
			m_mGreeted.Remove(old);
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
			m_GM.NotifyPlayer(playerId, "Haven neighbours", string.Format("Nearest job: %1, %2.", m.m_sTitle, m.GetWhereTextFor(m_GM, ent.GetOrigin(), playerId)));
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
	//! What Requisition may hand out: ammunition or rations, by prefab name (lower case keywords:
	//! the ammo words below, or the safehouses' food words).
	static bool IsRequisitionStock(ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return false;

		string name = prefab;
		name.ToLower();
		array<string> words = {"magazine", "mag_", "ammo", "cartridge", "rounds", "ration"};
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetSafehouses())
			words.InsertAll(gm.GetSafehouses().GetFoodKeywords());

		foreach (string word : words)
		{
			if (!word.IsEmpty() && name.Contains(word))
				return true;
		}

		return false;
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
		int last;
		int nowUnix = RZ_SeasonClock.Now();
		if (m_mRequisitionAt.Find(ident, last) && nowUnix - last < REQUISITION_COOLDOWN)
		{
			int minutes = RZ_Util.RoundToInt((REQUISITION_COOLDOWN - (nowUnix - last)) / 60);
			m_GM.NotifyPlayer(playerId, bastion, string.Format("Your next requisition is approved in about %1 minutes.", minutes));
			return false;
		}

		if (prefab.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, bastion, "The stores are empty. Nothing to requisition.");
			return false;
		}

		// Ammo and rations only, whatever the action is set to
		if (!IsRequisitionStock(prefab))
		{
			RZ_Log.Warn(string.Format("Requisition '%1' (%2) isn't ammo or rations (prefab name keywords): refused", itemName, prefab));
			m_GM.NotifyPlayer(playerId, bastion, string.Format("\"We sign out ammunition and rations. Not %1.\"", itemName));
			return false;
		}

		// Not to anyone in their enemy's colours, as at the shop
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility && hostility.GetDisguise().IsOn())
		{
			string colours = hostility.GetDisguise().ShopRefusal(playerId, RZ_EFaction.MILITARY);
			if (!colours.IsEmpty())
			{
				m_GM.NotifyPlayer(playerId, bastion, colours);
				return false;
			}
		}

		// Supply lines: signed out at an outpost, from what its trucks brought in
		RZ_SupplyState stock;
		int supplied = -1;
		RZ_SupplyLines supply = m_GM.GetSupply();
		if (supply)
			supplied = supply.RequisitionStock(owner.GetOrigin(), stock);

		if (supply && supply.IsFactionRunning(RZ_EFaction.MILITARY) && supplied < 0)
		{
			string nearest = supply.NearestSuppliedOutpost(owner.GetOrigin());
			if (nearest.IsEmpty())
				nearest = "none of them right now; wait for the trucks";

			m_GM.NotifyPlayer(playerId, bastion, "Requisitions are signed out at the outposts, from what the trucks bring in. Nearest with stock: " + nearest + ".");
			return false;
		}

		if (supplied == 0)
		{
			m_GM.NotifyPlayer(playerId, bastion, "Nothing to sign out here until the next supply truck gets through.");
			return false;
		}

		int price = RequisitionPrice(fullCost);
		if (!m_GM.SpendFavor(playerId, RZ_EFaction.MILITARY, price))
		{
			m_GM.NotifyPlayer(playerId, bastion, string.Format("Requisitioned %1 still costs %2 Favor. You have %3.", itemName, price, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.MILITARY)));
			return false;
		}

		IEntity signedOut = RZ_QuartermasterHelper.SpawnNear(prefab, owner, user);
		if (!signedOut)
		{
			m_GM.AddFavor(playerId, RZ_EFaction.MILITARY, price); // refund
			return false;
		}

		// Borrowed kit: signed out to this player, as a sale is
		if (hostility && hostility.GetDisguise().MarksOn())
			hostility.GetDisguise().GetMarks().OnSold(signedOut, playerId, RZ_EFaction.MILITARY);

		m_mRequisitionAt.Set(ident, RZ_SeasonClock.Now());
		string from = "Bastion stock";
		if (stock)
		{
			stock.m_iStock = Math.Max(0, stock.m_iStock - 1);
			from = string.Format("this outpost's stock (%1 left)", stock.m_iStock);
		}

		m_GM.NotifyPlayer(playerId, bastion, string.Format("Requisition approved: %1, signed out of %2. %3 Favor left.", itemName, from, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.MILITARY)));
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
				RemoveHunter(i);
				continue;
			}

			LoseTrailIfHarboured(i, ids, players);
			if (m_fClock < m_aHunterExpiry[i])
				continue;

			if (RZ_Util.NearestDistance(RZ_Util.GetUnitPosition(h), players) > 300)
			{
				RZ_Util.DeleteUnit(h);
				RemoveHunter(i);
			}
		}

		float interval = m_GM.GetHunterSquadMinutes() * 60;
		if (interval <= 0)
			return;

		m_fHunterTimer += dt;
		if (m_fHunterTimer < interval)
			return;

		m_fHunterTimer = 0;

		// Night (RZ_Environment): human patrols go out less
		if (m_GM.GetEnvironment() && m_GM.GetEnvironment().SkipHunterRound())
			return;

		array<int> hunters = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS};
		foreach (int j, IEntity p : players)
		{
			// Safe Harbour: nobody is sent after a Haven Champion inside a safehouse
			if (IsHarboured(ids[j], p))
				continue;

			foreach (int f : hunters)
			{
				if (!RZ_Reputation.IsHunted(m_GM.GetTier(ids[j], f)))
					continue;

				// Mid-parley with that faction: its squads skip this round
				RZ_Checkpoints posts = m_GM.GetCheckpoints();
				if (posts && posts.GetParley() && posts.GetParley().IsParleying(ids[j], f))
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
	//! A squad now, after a player who is Hunted by Bastion or the Red Hand (the errand Wanted Poster:
	//! someone sold where they are). Nothing if neither hunts them or there's no squad prefab.
	void SendHuntersNow(int playerId)
	{
		IEntity target = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!target)
			return;

		array<int> hunters = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS};
		foreach (int f : hunters)
		{
			if (!RZ_Reputation.IsHunted(m_GM.GetTier(playerId, f)))
				continue;

			ResourceName prefab = m_GM.GetHunterPrefab(f);
			if (prefab.IsEmpty())
				continue;

			SendHunters(playerId, target, f, prefab);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SendHunters(int playerId, IEntity target, RZ_EFaction faction, ResourceName prefab)
	{
		// Squads only go when nobody is near them, so a target who keeps fighting could gather
		// more and more of them: at most two after one player at a time
		// Their own limits (docs/INFECTED.md, "The one body budget"): four on the server at once
		if (CountHuntersAfter(playerId) >= MAX_SQUADS_PER_TARGET || CountHuntersAfter(-1) >= MAX_SQUADS_SERVER)
			return;

		// Out of every player's sight over the terrain: none in six tries, none this round
		vector at = target.GetOrigin();
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		vector from;
		bool hidden = false;
		for (int i = 0; i < 6 && !hidden; i++)
		{
			from = RZ_Mission.LandPointNear(at, HUNTER_DISTANCE * 0.9, HUNTER_DISTANCE * 1.1);
			hidden = !SeenFrom(from, players);
		}

		if (!hidden)
			return;

		IEntity squad = RZ_Util.SpawnPrefab(m_GM.PickLook(prefab, RZ_Looks.SeedAt(from)), from);
		if (!squad)
			return;

		RZ_Util.SendGroupTo(squad, at, m_GM.GetSpawner().GetMoveWaypointPrefab());
		// Real hostility toward the target, even if their standing alone wouldn't give it
		// (a Haven exile sold to the Red Hand)
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.MarkHostileAround(faction, squad, HUNTER_HOSTILE_RADIUS, playerId);
		m_aHunters.Insert(squad);
		m_aHunterExpiry.Insert(m_fClock + HUNTER_LIFETIME);
		m_aHunterTarget.Insert(playerId);
		m_aHunterLost.Insert(false);

		string text = "A Bastion patrol has your last known position.";
		if (faction == RZ_EFaction.BANDITS)
			text = "A Red Hand crew is hunting you. They're close.";

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(faction), text + " (" + RZ_Util.DirectionText(at, from) + ")");
	}

	//------------------------------------------------------------------------------------------------
	//! Squads out after this player; -1: on the whole server.
	protected int CountHuntersAfter(int playerId)
	{
		int n = 0;
		foreach (int i, int target : m_aHunterTarget)
		{
			if ((playerId < 0 || target == playerId) && m_aHunters[i])
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A player within HUNTER_SIGHT has a clear line over the terrain to pos.
	protected static bool SeenFrom(vector pos, array<IEntity> players)
	{
		foreach (IEntity p : players)
		{
			if (p && RZ_Util.DistXZ(p.GetOrigin(), pos) < HUNTER_SIGHT && !RZ_InfectedBodies.TerrainBlocks(p.GetOrigin(), pos))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveHunter(int i)
	{
		m_aHunters.Remove(i);
		m_aHunterExpiry.Remove(i);
		m_aHunterTarget.Remove(i);
		m_aHunterLost.Remove(i);
	}

	//------------------------------------------------------------------------------------------------
	//! Haven Champion perk "Safe Harbour": inside any safehouse.
	protected bool IsHarboured(int playerId, IEntity ent)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!ent || !houses)
			return false;

		return houses.IsSafeHarbour(playerId, ent.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	//! A squad whose target is in Safe Harbour loses the trail: it walks off the other way and
	//! gives up soon after.
	protected void LoseTrailIfHarboured(int i, array<int> ids, array<IEntity> players)
	{
		if (m_aHunterLost[i])
			return;

		int target = m_aHunterTarget[i];
		int idx = ids.Find(target);
		if (idx < 0)
			return;

		// Nobody follows anyone into a pack's ground after dark (docs/INFECTED.md, People)
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes && tribes.GetPeople().IsOn() && tribes.GetPhase() == RZ_ETribePhase.NIGHT && tribes.GetPeople().InRange(players[idx].GetOrigin()))
		{
			GiveUpTrail(i, players[idx].GetOrigin());
			m_GM.NotifyPlayer(target, "Behind you", "Whoever was following you stops at the edge of the trees. Nobody follows anyone in there after dark.");
			return;
		}

		if (!IsHarboured(target, players[idx]))
			return;

		GiveUpTrail(i, players[idx].GetOrigin());
		if (!m_aHarbourTold.Contains(target))
		{
			m_aHarbourTold.Insert(target);
			m_GM.NotifyPlayer(target, "Safe Harbour", "The shutters come down and someone moves the cart across the gate. Whoever was following you loses the trail at the edge of town.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The squad walks off, away from `from`, and gives up soon after (removed away from players).
	protected void GiveUpTrail(int i, vector from)
	{
		m_aHunterLost[i] = true;
		m_aHunterExpiry[i] = Math.Min(m_aHunterExpiry[i], m_fClock + LOST_TRAIL_SECONDS);
		vector squadPos = RZ_Util.GetUnitPosition(m_aHunters[i]);
		float gap = RZ_Util.DistXZ(squadPos, from);
		vector away = RZ_Util.RandomPointInRing(from, 400, 500);
		if (gap >= 1)
			away = squadPos + (squadPos - from) * (400 / gap);

		RZ_Util.RedirectGroup(m_aHunters[i], RZ_Util.GroundPos(away), m_GM.GetSpawner().GetMoveWaypointPrefab());
	}

	//------------------------------------------------------------------------------------------------
	//! Their target died: squads with no player near are removed; the others stop hunting, walk
	//! off and no longer count toward the next life's cap.
	protected void StandDownHunters(int victim, vector at, array<IEntity> players)
	{
		for (int i = m_aHunters.Count() - 1; i >= 0; i--)
		{
			if (m_aHunterTarget[i] != victim)
				continue;

			IEntity squad = m_aHunters[i];
			if (!squad || RZ_Util.NearestDistance(RZ_Util.GetUnitPosition(squad), players) > 300)
			{
				RZ_Util.DeleteUnit(squad);
				RemoveHunter(i);
				continue;
			}

			m_aHunterTarget[i] = -1;
			GiveUpTrail(i, at);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A bounty notice's "last seen" that only guesses: a town 1.5-3 km from where they really are.
	protected string VagueArea(vector pos)
	{
		vector guess = RZ_Util.RandomPointInRing(pos, 1500, 3000);
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(guess));
		if (s)
			return string.Format("somewhere out toward %1, they think", s.m_sName);

		return "nobody's sure where";
	}

	// ================================================================ Bounties
	//------------------------------------------------------------------------------------------------
	//! A safehouse short on food sells what it knows about someone the Haven turned out
	//! (RZ_Safehouses). The Red Hand sends a hunter squad if it has one; players the
	//! Red Hand knows hear a rough location. The target only gets a bad feeling.
	void LeakLocation(int playerId, IEntity target, string safehouseName)
	{
		if (!target)
			return;

		vector at = target.GetOrigin();
		ResourceName prefab = m_GM.GetHunterPrefab(RZ_EFaction.BANDITS);
		if (!prefab.IsEmpty())
			SendHunters(playerId, target, RZ_EFaction.BANDITS, prefab);
		else
			m_GM.NotifyPlayer(playerId, "A bad feeling", "Someone has been asking about you on the roads. Someone hungry.");

		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
		string where = RZ_SettlementScanner.GridReference(at);
		if (s)
			where = s.m_sName;

		string text = string.Format("A hungry safehouse in %1 sold a name for food: %2, seen near %3. Turned out by the Haven. Nobody will mourn them.",
			safehouseName, RZ_Util.PlayerName(playerId), where);
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int other : all)
		{
			if (other != playerId && RZ_Reputation.IsAtLeast(m_GM.GetTier(other, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
				m_GM.NotifyPlayer(other, "Word from the Red Hand", text);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AnnounceBounties(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			// One notice per wanted player, however many factions want them (one bounty is paid)
			string who = "";
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (m_GM.GetTier(ids[i], f) != RZ_ETier.KILL_ON_SIGHT)
					continue;

				if (!who.IsEmpty())
					who += " and ";

				who += RZ_Factions.GetName(f);
			}

			if (who.IsEmpty())
				continue;

			RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(p.GetOrigin()));
			string where = "near " + RZ_SettlementScanner.GridReference(p.GetOrigin());
			if (s)
				where = "near " + s.m_sName;

			// Safe Harbour: the Haven doesn't say where its Champions sleep
			if (IsHarboured(ids[i], p))
				where = VagueArea(p.GetOrigin());

			string text = string.Format("%1: a bounty on %2. Last seen %3. Whoever kills them gets %4 Favor.",
				who, RZ_Util.PlayerName(ids[i]), where, BOUNTY_FAVOR);
			foreach (int other : ids)
			{
				if (other != ids[i])
					m_GM.NotifyPlayer(other, "Bounty", text);
			}
		}

		// The board's postings, with the same rough "last seen"
		foreach (RZ_PostedBounty b : m_aPosted)
		{
			int targetId = FindPlayer(b.m_sTarget);
			int idx = ids.Find(targetId);
			if (idx < 0)
				continue;

			vector seen = players[idx].GetOrigin();
			RZ_Settlement town = m_GM.GetSettlement(m_GM.FindNearestSettlement(seen));
			string near = "near " + RZ_SettlementScanner.GridReference(seen);
			if (town)
				near = "near " + town.m_sName;

			if (IsHarboured(targetId, players[idx]))
				near = VagueArea(seen);

			string posted = string.Format("The Red Hand board: %1 Favor on %2. Last seen %3.", b.m_iPayout, b.m_sTargetName, near);
			foreach (int listener : ids)
			{
				if (listener != targetId)
					m_GM.NotifyPlayer(listener, "Bounty", posted);
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
			IEntity body;
			m_mLastAliveEnt.Find(deadId, body);
			m_mLastAlivePos.Remove(deadId);
			m_mLastAliveEnt.Remove(deadId);
			// A character who walked out at the barrier didn't die (docs/EPILOGUE.md, The Walk Out)
			RZ_Epilogue epi = RZ_Epilogue.Get();
			if (epi && epi.IsWalking(deadId))
			{
				epi.DoneWalking(deadId);
				continue;
			}

			if (connected.Contains(deadId))
				OnPlayerDied(deadId, at, body, ids, players);
		}

		foreach (int i, IEntity p : players)
		{
			m_mLastAlivePos.Set(ids[i], p.GetOrigin());
			m_mLastAliveEnt.Set(ids[i], p);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! While a death is being handled (missions' OnPlayerDied): the player who killed `victim`,
	//! -1 if it wasn't another player (infected, suicide, a fall, unknown).
	int KillerOf(int victim)
	{
		if (victim <= 0 || victim != m_iDeathVictim)
			return -1;

		return m_iDeathKiller;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDied(int victim, vector at, IEntity body, array<int> ids, array<IEntity> players)
	{
		// Only a kill by another player counts for bounties (not the infected, suicide or a fall)
		int killer = RZ_Util.GetKillerPlayerId(body);
		if (killer == victim)
			killer = -1;

		m_iDeathVictim = victim;
		m_iDeathKiller = killer;
		RememberKiller(victim, killer);
		// The life that ended, to the dying player's screen, before anything changes it (UI/RZ_DeathBrief.c)
		RZ_DeathBrief.OnDied(m_GM, victim, killer, at, body);
		// The cure finale: route items drop where their carrier fell, and kills between route players
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale)
			finale.OnPlayerKilled(victim, killer, at);

		m_GM.GetMissions().OnPlayerDied(victim, at);
		if (RZ_Epilogue.Get())
			RZ_Epilogue.Get().OnPlayerDied(victim, killer, at);

		// Killed by a tribe's body in the open: that tribe took them (docs/INFECTED.md, the taken)
		if (killer < 0 && m_GM.GetTribes())
			m_GM.GetTribes().OnPlayerDied(victim, at);
		RZ_WorldWriters.Death(victim, at);
		if (m_GM.GetErrands())
			m_GM.GetErrands().OnPlayerDied(victim);

		// A life that lasted leaves a journal: a headline or a wanderer's rumour, never an object
		if (m_GM.GetTownMemory())
			m_GM.GetTownMemory().RecordJournal(victim, at);

		// Haven Champion perk "Remembered": the Haven mourns them and greets their kin
		m_aHarbourTold.RemoveItem(victim);
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && RZ_Reputation.IsAtLeast(m_GM.GetTier(victim, RZ_EFaction.SURVIVORS), RZ_ETier.CHAMPION))
			houses.OnChampionFallen(victim, at);

		StandDownHunters(victim, at, players);
		PayBounty(victim, killer);
		m_iDeathVictim = -1;
		m_iDeathKiller = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! One bounty per death, to the killer only: from the faction that holds the most against the
	//! victim among those that put them at Kill on Sight and don't hunt the killer too.
	protected void PayBounty(int victim, int killer)
	{
		if (killer <= 0)
			return;

		array<int> connected = {};
		GetGame().GetPlayerManager().GetPlayers(connected);
		if (!connected.Contains(killer))
			return;

		// One bounty per death: Kill on Sight first, else a posting on the board
		if (!PayKillOnSight(victim, killer))
			PayPosted(victim, killer);
	}

	//------------------------------------------------------------------------------------------------
	protected bool PayKillOnSight(int victim, int killer)
	{
		// A teammate's head pays nothing: no cashing each other in
		if (RZ_Util.SameGroup(killer, victim))
			return false;

		int payer = -1;
		int worst = -1;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (m_GM.GetTier(victim, f) != RZ_ETier.KILL_ON_SIGHT)
				continue;

			// The faction doesn't pay people it hunts itself
			if (RZ_Reputation.IsHunted(m_GM.GetTier(killer, f)))
				continue;

			int infamy = m_GM.GetProfile(victim).GetInfamy(f);
			if (infamy > worst)
			{
				worst = infamy;
				payer = f;
			}
		}

		if (payer < 0)
			return false;

		m_GM.AddFavor(killer, payer, BOUNTY_FAVOR);
		m_GM.AddFame(killer, payer, BOUNTY_FAME, false);
		m_GM.NotifyPlayer(killer, "Bounty claimed", string.Format("%1 pays you %2 Favor for %3.", RZ_Factions.GetName(payer), BOUNTY_FAVOR, RZ_Util.PlayerName(victim)));
		m_GM.NotifyAll("Bounty", string.Format("The %1 bounty on %2 has been claimed.", RZ_Factions.GetShortName(payer), RZ_Names.Public(victim)));
		return true;
	}

	// ================================================================ The bounty board
	//------------------------------------------------------------------------------------------------
	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	//! The bounty board, who killed whom last and the Requisition cooldown (the fence's hour is
	//! added by RZ_FactionServices.Save).
	RZ_PerksSave Save()
	{
		RZ_PerksSave data = new RZ_PerksSave();
		foreach (RZ_PostedBounty b : m_aPosted)
		{
			if (b)
				data.posted.Insert(b);
		}

		foreach (string victim, string killer : m_mLastKiller)
		{
			string name;
			m_mLastKillerName.Find(victim, name);
			data.killerVictims.Insert(victim);
			data.killerIdents.Insert(killer);
			data.killerNames.Insert(name);
		}

		int now = RZ_SeasonClock.Now();
		foreach (string ident, int at : m_mRequisitionAt)
		{
			if (now - at >= REQUISITION_COOLDOWN)
				continue;

			data.requisitionIdents.Insert(ident);
			data.requisitionUnix.Insert(at);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	//! Older saves have none: the board, the kills and the cooldowns start empty.
	void Restore(RZ_PerksSave data)
	{
		if (!data)
			return;

		if (data.posted)
		{
			foreach (RZ_PostedBounty b : data.posted)
			{
				if (b && !b.m_sTarget.IsEmpty() && !b.m_sPoster.IsEmpty())
					m_aPosted.Insert(b);
			}
		}

		if (data.killerVictims && data.killerIdents && data.killerNames)
		{
			for (int k = 0; k < data.killerVictims.Count() && k < data.killerIdents.Count() && k < data.killerNames.Count(); k++)
			{
				m_mLastKiller.Set(data.killerVictims[k], data.killerIdents[k]);
				m_mLastKillerName.Set(data.killerVictims[k], data.killerNames[k]);
			}
		}

		if (data.requisitionIdents && data.requisitionUnix)
		{
			for (int r = 0; r < data.requisitionIdents.Count() && r < data.requisitionUnix.Count(); r++)
			{
				m_mRequisitionAt.Set(data.requisitionIdents[r], data.requisitionUnix[r]);
			}
		}

		RZ_Log.Info(string.Format("Perks: restored %1 bounty postings, %2 last killers, %3 Requisition cooldowns", m_aPosted.Count(), m_mLastKiller.Count(), m_mRequisitionAt.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! Who killed whom last, by identity: the only person a Champion can post a bounty on.
	protected void RememberKiller(int victim, int killer)
	{
		if (killer <= 0)
			return;

		string victimIdent = m_GM.GetProfile(victim).m_sIdentity;
		string killerIdent = m_GM.GetProfile(killer).m_sIdentity;
		if (victimIdent.IsEmpty() || killerIdent.IsEmpty())
			return;

		m_mLastKiller.Set(victimIdent, killerIdent);
		m_mLastKillerName.Set(victimIdent, RZ_Util.PlayerName(killer));
	}

	//------------------------------------------------------------------------------------------------
	//! The connected player with this identity, -1 if none.
	protected int FindPlayer(string ident)
	{
		if (ident.IsEmpty())
			return -1;

		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int id : all)
		{
			if (m_GM.GetProfile(id).m_sIdentity == ident)
				return id;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_PostedBounty FindPosting(string targetIdent)
	{
		foreach (RZ_PostedBounty b : m_aPosted)
		{
			if (b.m_sTarget == targetIdent)
				return b;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Red Hand Champion (RZ_FactionServiceAction BOUNTY_BOARD): a bounty on the last player who
	//! killed you. Capped (one per poster, one per target, a few on the server), and it pays the
	//! killer less than it costs, so it can't be farmed with a friend.
	void PostBounty(int playerId)
	{
		string title = "The Red Hand board";
		RZ_ServiceSettings settings = m_GM.GetServiceSettings();
		if (!settings || !settings.m_bBountyBoard)
			return;

		RZ_ETier tier = m_GM.GetTier(playerId, RZ_EFaction.BANDITS);
		if (tier != RZ_ETier.CHAMPION)
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("The board is for the Red Hand's own Champions. You're %1 with them.", RZ_Reputation.GetTierName(tier)));
			return;
		}

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		string target;
		if (!m_mLastKiller.Find(ident, target) || target.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, title, "\"A bounty on who? Nobody's put you down lately. Not that we heard.\"");
			return;
		}

		string targetName;
		m_mLastKillerName.Find(ident, targetName);
		int targetId = FindPlayer(target);
		if (targetId > 0 && RZ_Util.SameGroup(playerId, targetId))
		{
			m_GM.NotifyPlayer(playerId, title, "\"On your own crew? Sort that out yourselves.\"");
			return;
		}

		foreach (RZ_PostedBounty mine : m_aPosted)
		{
			if (mine.m_sPoster == ident)
			{
				m_GM.NotifyPlayer(playerId, title, string.Format("\"You've already got one up, on %1. One at a time.\"", mine.m_sTargetName));
				return;
			}
		}

		if (FindPosting(target))
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("\"There's already a price on %1. Wait your turn.\"", targetName));
			return;
		}

		if (m_aPosted.Count() >= settings.m_iBountyMaxActive)
		{
			m_GM.NotifyPlayer(playerId, title, "\"Board's full. Come back when somebody's collected.\"");
			return;
		}

		int cost = settings.m_iBountyCost;
		if (!m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, cost))
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("\"A bounty costs %1 Favor.\" You have %2.", cost, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS)));
			return;
		}

		RZ_PostedBounty b = new RZ_PostedBounty();
		b.m_sPoster = ident;
		b.m_sPosterName = RZ_Util.PlayerName(playerId);
		b.m_sTarget = target;
		b.m_sTargetName = targetName;
		b.m_iPaid = cost;
		b.m_iPayout = Math.Min(settings.m_iBountyPayout, cost);
		b.m_iUntilUnix = RZ_SeasonClock.Now() + RZ_Util.RoundToInt(Math.Max(0.25, settings.m_fBountyHours) * 3600);
		m_aPosted.Insert(b);

		m_GM.NotifyPlayer(playerId, title, string.Format("Your name goes up next to %1's, in red paint. %2 Favor to whoever puts them down.", targetName, b.m_iPayout));
		m_GM.NotifyAll("Bounty", string.Format("The Red Hand board: %1 Favor on %2.", b.m_iPayout, RZ_RepGates.BountyWords(m_GM, targetId, targetName)));
		if (targetId > 0)
			m_GM.NotifyPlayer(targetId, "A bad feeling", "Your name is on the Red Hand board. Someone you put down wants you to know it.");
	}

	//------------------------------------------------------------------------------------------------
	//! The killer only: never the poster, nobody in the poster's or the victim's group, and nobody
	//! the Red Hand hunts. Paid like any bounty (RZ_FactionPerks.PayBounty decides who killed whom).
	protected bool PayPosted(int victim, int killer)
	{
		string victimIdent = m_GM.GetProfile(victim).m_sIdentity;
		RZ_PostedBounty b = FindPosting(victimIdent);
		if (!b)
			return false;

		string killerIdent = m_GM.GetProfile(killer).m_sIdentity;
		int posterId = FindPlayer(b.m_sPoster);
		if (killerIdent == b.m_sPoster || RZ_Util.SameGroup(killer, posterId) || RZ_Util.SameGroup(killer, victim))
			return false;

		if (RZ_Reputation.IsHunted(m_GM.GetTier(killer, RZ_EFaction.BANDITS)))
			return false;

		m_aPosted.RemoveItem(b);
		m_GM.AddFavor(killer, RZ_EFaction.BANDITS, b.m_iPayout);
		m_GM.NotifyPlayer(killer, "Bounty claimed", string.Format("The Red Hand board pays you %1 Favor for %2.", b.m_iPayout, b.m_sTargetName));
		if (posterId > 0)
			m_GM.NotifyPlayer(posterId, "The Red Hand board", string.Format("Somebody collected on %1. Your name comes off the board.", b.m_sTargetName));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Unclaimed postings come down; the poster gets half back, online or not.
	protected void ExpirePostings()
	{
		int now = RZ_SeasonClock.Now();
		for (int i = m_aPosted.Count() - 1; i >= 0; i--)
		{
			RZ_PostedBounty b = m_aPosted[i];
			if (now < b.m_iUntilUnix)
				continue;

			m_aPosted.Remove(i);
			int back = b.m_iPaid / 2;
			int posterId = FindPlayer(b.m_sPoster);
			if (posterId <= 0)
			{
				// Offline: credited to their profile by identity
				RZ_PlayerProfile poster = m_GM.FindProfileByIdentity(b.m_sPoster);
				if (poster)
					poster.AddFavor(RZ_EFaction.BANDITS, back);

				continue;
			}

			m_GM.AddFavor(posterId, RZ_EFaction.BANDITS, back);
			m_GM.NotifyPlayer(posterId, "The Red Hand board", string.Format("Nobody collected on %1. The paint's washed off; you get %2 Favor back.", b.m_sTargetName, back));
		}
	}
}
