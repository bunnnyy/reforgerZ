//! Faction services and the last perks (docs/REPUTATION.md, "What each faction gives you",
//! "Working off Infamy" and "Reputation clues"). Server only, owned by the game mode.
//!
//!   Bastion Champion   Fire Mission: throw a Bastion signal smoke (RZ_FireMissionSmokeComponent)
//!                      at a horde; 30 s later the guns hit it. Long cooldown, never near people
//!   Red Hand Trusted   Fence: sell found loot for a little Red Hand Favor (never shop goods)
//!   Red Hand Champion  Bounty board: post a bounty on whoever last killed you (RZ_FactionPerks)
//!   Halcyon Trusted    the medic: patched up, a fresh bite washed out
//!   Halcyon Champion   the immunity booster: the next bite doesn't take (once per life)
//!   Anyone             Amends: pay a faction's Favor to work off a little Infamy with it,
//!                      dearer each time, capped per life, never below Watched
//!   Trusted            Reputation clues: a Red Hand armband or a Haven ribbon to wear
//! Services are RZ_FactionServiceAction on a quartermaster; each checks tier and Favor on the server.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Factions > Services.
[BaseContainerProps()]
class RZ_ServiceSettings
{
	[Attribute("1", desc: "Bastion Champion perk Fire Mission: a thrown Bastion signal smoke (RZ_FireMissionSmokeComponent) near a horde calls in the guns")]
	bool m_bFireMission;

	[Attribute("60", desc: "Fire Mission: real minutes between strikes for one player")]
	float m_fFireMissionCooldownMinutes;

	[Attribute("6", desc: "Fire Mission: infected within the strike radius needed (a horde, not a straggler)")]
	int m_iFireMissionMinInfected;

	[Attribute("50", desc: "Fire Mission: strike radius (m)")]
	float m_fFireMissionRadius;

	[Attribute("30", desc: "Fire Mission: seconds from smoke to shells")]
	float m_fFireMissionDelay;

	[Attribute("25", desc: "Fire Mission: most infected one strike kills")]
	int m_iFireMissionKillCap;

	[Attribute("250", desc: "Fire Mission: Bastion won't shell within this distance (m) of a safehouse, a faction holding or the lab")]
	float m_fFireMissionSafeDistance;

	[Attribute("1", desc: "Red Hand Trusted perk Fence: sell found loot for Red Hand Favor")]
	bool m_bFence;

	[Attribute("watch,jewel,ring,necklace,gold,silver,binocular,compass,camera,lighter", desc: "Fence: keywords (lower case, in prefab names) of the loot it buys")]
	string m_sFenceWords;

	[Attribute("3", desc: "Fence: Favor per item. Keep it well below shop prices; shop goods are never bought back")]
	int m_iFencePrice;

	[Attribute("8", desc: "Fence: most items in one sale")]
	int m_iFenceMaxItems;

	[Attribute("30", desc: "Fence: most Favor one player gets from it per real hour")]
	int m_iFenceHourlyCap;

	[Attribute("1", desc: "Red Hand Champion perk Bounty board: post a bounty on whoever last killed you")]
	bool m_bBountyBoard;

	[Attribute("60", desc: "Bounty board: Red Hand Favor to post a bounty")]
	int m_iBountyCost;

	[Attribute("40", desc: "Bounty board: Red Hand Favor paid to the killer (below the cost, so it can't be farmed)")]
	int m_iBountyPayout;

	[Attribute("3", desc: "Bounty board: postings up at once on the server")]
	int m_iBountyMaxActive;

	[Attribute("2", desc: "Bounty board: real hours a posting stays up; unclaimed, half the cost comes back")]
	float m_fBountyHours;

	[Attribute("80", desc: "Halcyon Champion: the immunity booster's price in Halcyon Favor (once per life)")]
	int m_iBoosterCost;

	[Attribute("20", desc: "Halcyon Trusted: the medic's price in Halcyon Favor")]
	int m_iMedicCost;

	[Attribute("15", desc: "Amends: Favor (the faction's own) for the first payment")]
	int m_iAmendsCost;

	[Attribute("1.5", desc: "Amends: each further payment to the same faction this life costs this much more")]
	float m_fAmendsGrowth;

	[Attribute("5", desc: "Amends: Infamy worked off per payment (before the stronghold leader's temper)")]
	int m_iAmendsInfamy;

	[Attribute("15", desc: "Amends: most Infamy one life can work off by paying, per faction")]
	int m_iAmendsLifeCap;

	[Attribute("15", desc: "Amends: paying never takes Infamy below this (the Watched line); the rest takes amends contracts")]
	int m_iAmendsFloor;

	[Attribute("30", desc: "Amends: real minutes between payments to one faction")]
	float m_fAmendsCooldownMinutes;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Reputation clue: a red armband (clothing) given at Red Hand Trusted, to wear or not (optional)", "et")]
	ResourceName m_sRedHandArmbandPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Reputation clue: a Haven ribbon (clothing or a small item) given at Haven Trusted (optional)", "et")]
	ResourceName m_sHavenRibbonPrefab;
}

//------------------------------------------------------------------------------------------------
enum RZ_EFactionService
{
	FENCE,			// Red Hand Trusted
	BOUNTY_BOARD,	// Red Hand Champion
	BOOSTER,		// Halcyon Champion
	MEDIC,			// Halcyon Trusted
	AMENDS,			// any faction: the quartermaster's own
	SETTLE			// any faction: settle up, 10 Fame for 5 Infamy (Factions/RZ_Parley.c)
}

//------------------------------------------------------------------------------------------------
//! Shells on a spot: the explosion prefab a few times, and the noise. Shared by the Fire Mission
//! contract and the Bastion Champion perk. Who dies is up to the caller.
class RZ_Artillery
{
	static void Shell(RZ_GameModeComponent gm, vector at, float radius, int shells, array<IEntity> propsOut, string cause = "")
	{
		// The signal ring: shells landed here (who called them stays on the server)
		RZ_WorldWriters.Shell(at, cause);
		ResourceName fx = gm.GetExplosionPrefab();
		for (int n = 0; n < shells; n++)
		{
			IEntity boom = RZ_Util.SpawnPrefab(fx, RZ_Util.RandomPointInRing(at, 0, radius * 0.6));
			if (boom && propsOut)
				propsOut.Insert(boom);
		}

		gm.GetHeat().AddHeat(at, 30);
	}
}

//------------------------------------------------------------------------------------------------
//! A strike on its way.
class RZ_PendingStrike
{
	vector m_vPos;
	float m_fAt;
	int m_iCaller;
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Bastion signal smoke for the Fire Mission perk: thrown near a horde by a Bastion Champion, it calls in the guns. Put it on a throwable smoke grenade sold at Bastion's Champion shelf.")]
class RZ_FireMissionSmokeComponentClass : ScriptComponentClass
{
}

//! Like a thrown noisemaker (RZ_NoisemakerComponent): the server sees it carried by a player, then
//! lying a few metres from where they stood. Then RZ_FactionServices.CallFireMission decides.
class RZ_FireMissionSmokeComponent : ScriptComponent
{
	protected int m_iThrower = -1;
	protected bool m_bWasCarried;
	protected vector m_vCarrierPos;
	protected bool m_bUsed;

	protected static ref array<RZ_FireMissionSmokeComponent> s_aAll;
	protected static const float THROW_MIN_DISTANCE = 4;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!s_aAll)
			s_aAll = new array<RZ_FireMissionSmokeComponent>();

		s_aAll.Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void Poll(RZ_FactionServices services)
	{
		IEntity owner = GetOwner();
		if (m_bUsed || !owner)
			return;

		int carrier = RZ_Util.GetCarrierPlayerId(owner);
		if (carrier > 0)
		{
			IEntity body = GetGame().GetPlayerManager().GetPlayerControlledEntity(carrier);
			m_iThrower = carrier;
			m_bWasCarried = true;
			if (body)
				m_vCarrierPos = body.GetOrigin();

			return;
		}

		if (!m_bWasCarried)
			return;

		m_bWasCarried = false;
		if (RZ_Util.DistXZ(owner.GetOrigin(), m_vCarrierPos) < THROW_MIN_DISTANCE)
			return;

		m_bUsed = true;
		services.CallFireMission(m_iThrower, owner.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_FactionServices.Tick (the server tick).
	static void TickAll(RZ_FactionServices services)
	{
		if (!s_aAll)
			return;

		for (int i = s_aAll.Count() - 1; i >= 0; i--)
		{
			RZ_FireMissionSmokeComponent c = s_aAll[i];
			if (!c)
			{
				s_aAll.Remove(i);
				continue;
			}

			c.Poll(services);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! A service at a quartermaster: put it in its ActionsManagerComponent and set the service.
//! Amends uses the quartermaster's own faction. The server checks tier and Favor on use.
class RZ_FactionServiceAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Which service: Fence and Bounty board on the Red Hand, Booster and Medic on Halcyon, Amends on any quartermaster", "", ParamEnumArray.FromEnum(RZ_EFactionService))]
	protected RZ_EFactionService m_eService;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		switch (m_eService)
		{
			case RZ_EFactionService.FENCE: outName = "Fence what you're carrying (Red Hand Trusted)"; break;
			case RZ_EFactionService.BOUNTY_BOARD: outName = "Post a bounty on whoever killed you last (Red Hand Champion)"; break;
			case RZ_EFactionService.BOOSTER: outName = "Take the immunity booster (Halcyon Champion)"; break;
			case RZ_EFactionService.MEDIC: outName = "See the Halcyon medic (Halcyon Trusted)"; break;
			case RZ_EFactionService.AMENDS: outName = "Pay to make amends (Favor)"; break;
			case RZ_EFactionService.SETTLE: outName = "Settle up (10 Fame for 5 Infamy)"; break;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Hidden on a quartermaster while this machine uses the shop screen: it's a row on its
	//! Services tab.
	override bool CanBeShownScript(IEntity user)
	{
		return !RZ_ShopMenu.HidesActionsOn(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! The label the action menu shows, for the shop screen's Services tab.
	string GetLabel()
	{
		string label;
		GetActionNameScript(label);
		return label;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		Run(gm, playerId, pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Performs the service for the player (server): the action, or its row on the shop screen.
	void Run(RZ_GameModeComponent gm, int playerId, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (gm.GetServices())
			gm.GetServices().Perform(playerId, m_eService, pOwnerEntity, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_FactionServices
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_ServiceSettings m_Settings;
	protected float m_fClock;
	protected ref map<string, float> m_mFireAt = new map<string, float>();		// identity -> clock of the last strike
	protected ref array<ref RZ_PendingStrike> m_aStrikes = {};
	protected ref array<IEntity> m_aShellProps = {};
	protected ref array<float> m_aShellExpiry = {};
	protected ref map<string, float> m_mFenceWindow = new map<string, float>();	// identity -> start of the hour
	protected ref map<string, int> m_mFenceEarned = new map<string, int>();
	protected ref map<string, float> m_mAmendsAt = new map<string, float>();		// "identity:faction" -> clock
	protected ref array<string> m_aFenceWords = {};

	protected static const float SHELL_PROP_SECONDS = 60;

	//------------------------------------------------------------------------------------------------
	void RZ_FactionServices(RZ_GameModeComponent gm, RZ_ServiceSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		RZ_ItemHelper.Words(settings.m_sFenceWords, m_aFenceWords);
	}

	//------------------------------------------------------------------------------------------------
	RZ_ServiceSettings GetSettings()
	{
		return m_Settings;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		RZ_FireMissionSmokeComponent.TickAll(this);

		for (int i = m_aStrikes.Count() - 1; i >= 0; i--)
		{
			if (m_fClock < m_aStrikes[i].m_fAt)
				continue;

			Strike(m_aStrikes[i]);
			m_aStrikes.Remove(i);
		}

		for (int j = m_aShellProps.Count() - 1; j >= 0; j--)
		{
			if (m_fClock < m_aShellExpiry[j])
				continue;

			RZ_Util.DeleteUnit(m_aShellProps[j]);
			m_aShellProps.Remove(j);
			m_aShellExpiry.Remove(j);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Perform(int playerId, RZ_EFactionService service, IEntity owner, IEntity user)
	{
		switch (service)
		{
			case RZ_EFactionService.FENCE: Fence(playerId, user); break;
			case RZ_EFactionService.BOUNTY_BOARD: m_GM.GetPerks().PostBounty(playerId); break;
			case RZ_EFactionService.BOOSTER: Booster(playerId); break;
			case RZ_EFactionService.MEDIC: Medic(playerId, user); break;
			case RZ_EFactionService.AMENDS: Amends(playerId, owner); break;
			case RZ_EFactionService.SETTLE: Settle(playerId, owner); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Make amends' second price (Reputation > Settling up): spent Fame for Infamy, from the budget.
	protected void Settle(int playerId, IEntity owner)
	{
		RZ_QuartermasterComponent qm;
		if (owner)
			qm = RZ_QuartermasterComponent.Cast(owner.FindComponent(RZ_QuartermasterComponent));

		if (!qm)
			return;

		RZ_EFaction f = qm.GetFaction();
		if (!RZ_ReputationSettings.Get().m_bSettling || !m_GM.GetCheckpoints())
		{
			m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), "\"We don't do that here.\"");
			return;
		}

		if (m_GM.GetTier(playerId, f) == RZ_ETier.KILL_ON_SIGHT)
		{
			m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), "\"Fame? From you? They want blood.\"");
			return;
		}

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), m_GM.GetCheckpoints().GetParley().Settle(playerId, f));
	}

	//------------------------------------------------------------------------------------------------
	//! Known, Trusted or Champion with the faction and not Hunted by it. Tells them why not.
	protected bool Allowed(int playerId, RZ_EFaction f, RZ_ETier needed, string what)
	{
		RZ_ETier tier = m_GM.GetTier(playerId, f);
		if (RZ_Reputation.IsAtLeast(tier, needed) && !RZ_Reputation.IsHunted(tier))
			return true;

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("%1 is for people who are %2 with them. You're %3.",
			what, RZ_Reputation.GetTierName(needed), RZ_Reputation.GetTierName(tier)));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	// ================================================================ Bastion: Fire Mission
	//------------------------------------------------------------------------------------------------
	//! A Bastion signal smoke landed. A Champion near a real horde, away from people, off cooldown,
	//! gets the guns 30 s later. Nothing is paid for it: the reward is the horde gone.
	void CallFireMission(int playerId, vector pos)
	{
		string bastion = RZ_Factions.GetName(RZ_EFaction.MILITARY);
		if (!m_Settings.m_bFireMission || playerId <= 0)
			return;

		RZ_ETier tier = m_GM.GetTier(playerId, RZ_EFaction.MILITARY);
		if (tier != RZ_ETier.CHAMPION)
		{
			m_GM.NotifyPlayer(playerId, bastion, "Red smoke, and nothing on the radio. The guns only answer people Bastion would trust with them.");
			return;
		}

		string ident = Ident(playerId);
		float last;
		float cooldown = m_Settings.m_fFireMissionCooldownMinutes * 60;
		if (m_mFireAt.Find(ident, last) && m_fClock - last < cooldown)
		{
			m_GM.NotifyPlayer(playerId, bastion, string.Format("\"Negative. The guns are rationed. Next fire mission in about %1 minutes.\"", Math.Max(1, RZ_Util.RoundToInt((cooldown - (m_fClock - last)) / 60))));
			return;
		}

		if (IsNearPeople(pos))
		{
			m_GM.NotifyPlayer(playerId, bastion, "\"Negative. That's too close to people. We don't shell our own.\"");
			return;
		}

		int needed = Math.Max(1, m_Settings.m_iFireMissionMinInfected);
		if (m_GM.GetSpawner().CountZombiesNear(pos, m_Settings.m_fFireMissionRadius, needed) < needed)
		{
			m_GM.NotifyPlayer(playerId, bastion, "\"We see your smoke. We don't see a horde. Not wasting shells on a handful.\"");
			return;
		}

		m_mFireAt.Set(ident, m_fClock);
		RZ_PendingStrike strike = new RZ_PendingStrike();
		strike.m_vPos = pos;
		strike.m_fAt = m_fClock + Math.Max(5, m_Settings.m_fFireMissionDelay);
		strike.m_iCaller = playerId;
		m_aStrikes.Insert(strike);
		RZ_ContractHelp.NotifyNear(m_GM, pos, 300, "Fire Mission", string.Format("Red smoke over the horde. The radio crackles: \"Smoke seen. %1 seconds. Get clear.\"", RZ_Util.RoundToInt(m_Settings.m_fFireMissionDelay)));
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsNearPeople(vector pos)
	{
		// The world facts own the rule (safehouses, holdings, the lab), so shelling events use it too
		return m_GM.GetFacts().IsNearLivedIn(pos, m_Settings.m_fFireMissionSafeDistance);
	}

	//------------------------------------------------------------------------------------------------
	protected void Strike(RZ_PendingStrike strike)
	{
		array<IEntity> props = {};
		RZ_Artillery.Shell(m_GM, strike.m_vPos, m_Settings.m_fFireMissionRadius, 4, props, strike.m_iCaller.ToString());
		foreach (IEntity p : props)
		{
			m_aShellProps.Insert(p);
			m_aShellExpiry.Insert(m_fClock + SHELL_PROP_SECONDS);
		}

		int killed = m_GM.GetSpawner().KillZombiesNear(strike.m_vPos, m_Settings.m_fFireMissionRadius, Math.Max(1, m_Settings.m_iFireMissionKillCap));
		RZ_ContractHelp.NotifyNear(m_GM, strike.m_vPos, 600, "Fire Mission", string.Format("The shells land in a tight group. When the dust settles, %1 of them don't get up.", killed));
		RZ_Log.Info(string.Format("Fire Mission perk: player %1, %2 infected killed.", strike.m_iCaller, killed));
	}

	// ================================================================ Red Hand: Fence
	//------------------------------------------------------------------------------------------------
	//! Found loot for a little Red Hand Favor, capped per hour. Never anything a quartermaster sold
	//! (RZ_QuartermasterHelper.WasBought), so buying and selling back can't make Favor.
	protected void Fence(int playerId, IEntity user)
	{
		string title = "The fence";
		if (!m_Settings.m_bFence || !user)
			return;

		// Season rules (the Red Hand's season): the black market is open to anyone they know
		RZ_ETier fenceTier = RZ_ETier.TRUSTED;
		if (RZ_Seasons.Mods().m_bOpenBlackMarket)
			fenceTier = RZ_ETier.KNOWN;

		if (!Allowed(playerId, RZ_EFaction.BANDITS, fenceTier, "The fence"))
			return;

		string ident = Ident(playerId);
		float window;
		if (!m_mFenceWindow.Find(ident, window) || m_fClock - window >= 3600)
		{
			m_mFenceWindow.Set(ident, m_fClock);
			m_mFenceEarned.Set(ident, 0);
			window = m_fClock;
		}

		int earned;
		m_mFenceEarned.Find(ident, earned);
		int price = Math.Max(1, m_Settings.m_iFencePrice);
		int room = m_Settings.m_iFenceHourlyCap - earned;
		int maxItems = Math.Min(m_Settings.m_iFenceMaxItems, room / price);
		if (maxItems <= 0)
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("\"I've had enough from you for one hour. Come back in %1 minutes.\"", Math.Max(1, RZ_Util.RoundToInt((3600 - (m_fClock - window)) / 60))));
			return;
		}

		int sold = TakeFenceable(user, maxItems);
		if (sold <= 0)
		{
			m_GM.NotifyPlayer(playerId, title, "\"Nothing here I can move. And don't bring me shop goods: I know my own stock.\"");
			return;
		}

		int favor = sold * price;
		m_mFenceEarned.Set(ident, earned + favor);
		m_GM.AddFavor(playerId, RZ_EFaction.BANDITS, favor);
		m_GM.NotifyPlayer(playerId, title, string.Format("The fence weighs %1 things in one hand and doesn't haggle. +%2 Red Hand Favor.", sold, favor));
	}

	//------------------------------------------------------------------------------------------------
	protected int TakeFenceable(IEntity character, int maxCount)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<IEntity> items = {};
		inv.GetItems(items);
		int taken = 0;
		foreach (IEntity item : items)
		{
			if (taken >= maxCount)
				break;

			if (!item || RZ_QuartermasterHelper.WasBought(item) || !RZ_FoodHelper.IsFood(item, m_aFenceWords))
				continue;

			inv.TryDeleteItem(item);
			taken++;
		}

		return taken;
	}

	// ================================================================ Halcyon: booster and medic
	//------------------------------------------------------------------------------------------------
	protected void Booster(int playerId)
	{
		string halcyon = RZ_Factions.GetName(RZ_EFaction.INSTITUTE);
		if (!Allowed(playerId, RZ_EFaction.INSTITUTE, RZ_ETier.CHAMPION, "The immunity booster"))
			return;

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p.m_bBoosted)
		{
			m_GM.NotifyPlayer(playerId, halcyon, "\"You've had it. A second shot does nothing but make you sick.\"");
			return;
		}

		if (p.IsInfected())
		{
			m_GM.NotifyPlayer(playerId, halcyon, "\"It's for the next bite, not this one. For this one, ask about the trial.\"");
			return;
		}

		int cost = m_Settings.m_iBoosterCost;
		if (!m_GM.SpendFavor(playerId, RZ_EFaction.INSTITUTE, cost))
		{
			m_GM.NotifyPlayer(playerId, halcyon, string.Format("\"It costs %1 Favor. Every dose is a week of work.\" You have %2.", cost, p.GetFavor(RZ_EFaction.INSTITUTE)));
			return;
		}

		p.m_bBoosted = true;
		m_GM.NotifyPlayer(playerId, halcyon, string.Format("A long needle, a cold arm, a researcher writing your name in a ledger. \"One bite. It'll fight off one bite. After that you're like everyone else.\" %1 Favor left.", p.GetFavor(RZ_EFaction.INSTITUTE)));
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.InfectPlayer: true if the booster took this bite (it's used up).
	bool TryShrugOffBite(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (!p.m_bBoosted)
			return false;

		p.m_bBoosted = false;
		m_GM.NotifyPlayer(playerId, "Bitten", "Teeth, and then a fever that lasts an hour and breaks. Halcyon's booster held. That was the only one it had in it.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The safehouse medic's treatment at Halcyon's prices: patched up, a fresh bite washed out
	//! (RZ_Infection.CleanAtSafehouse). A bite that's taken needs the trial or an antidote.
	protected void Medic(int playerId, IEntity user)
	{
		string title = "Halcyon medic";
		if (!Allowed(playerId, RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, "The medic"))
			return;

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		int cost = m_Settings.m_iMedicCost;
		if (!m_GM.SpendFavor(playerId, RZ_EFaction.INSTITUTE, cost))
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("\"%1 Favor for the supplies.\" You have %2.", cost, p.GetFavor(RZ_EFaction.INSTITUTE)));
			return;
		}

		RZ_Util.HealCharacter(user);
		bool cleaned = false;
		if (m_GM.GetInfection())
			cleaned = m_GM.GetInfection().CleanAtSafehouse(playerId);

		if (cleaned)
			return; // the clean has its own message

		if (p.IsInfected())
		{
			m_GM.NotifyPlayer(playerId, title, "Stitches, a clean dressing, a look at the bite she doesn't like. \"I can't stop that. Ask for the trial, or find an antidote. Soon.\"");
			return;
		}

		m_GM.NotifyPlayer(playerId, title, "Stitches, a clean dressing and a lecture about tetanus. \"Next time, come in before it's this bad.\"");
	}

	// ================================================================ Amends
	//------------------------------------------------------------------------------------------------
	//! Pay the quartermaster's faction to work off a little Infamy with it. Each payment costs more,
	//! at most m_iAmendsLifeCap per life, never below the Watched line, once every half hour, and
	//! all of it from the one forgiveness budget (RZ_GameModeComponent.Forgive).
	protected void Amends(int playerId, IEntity owner)
	{
		RZ_QuartermasterComponent qm;
		if (owner)
			qm = RZ_QuartermasterComponent.Cast(owner.FindComponent(RZ_QuartermasterComponent));

		if (!qm)
			return;

		RZ_EFaction f = qm.GetFaction();
		string name = RZ_Factions.GetName(f);
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		p.EnsureArrays();
		RZ_ETier tier = p.GetTier(f);
		if (tier == RZ_ETier.KILL_ON_SIGHT)
		{
			m_GM.NotifyPlayer(playerId, name, "\"Favor? From you? They want blood, not Favor.\"");
			return;
		}

		int infamy = p.GetInfamy(f);
		int minInfamy = Math.Max(0, m_Settings.m_iAmendsFloor);
		int lifeRoom = Math.Min(m_Settings.m_iAmendsLifeCap - p.m_aAmendsWorked[f], p.ForgiveLeft(f));
		// The stronghold leader's temper first (Merciful: more), then the floor and the life cap
		int amount = m_Settings.m_iAmendsInfamy;
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
			amount = -leaders.ScaleInfamy(f, -amount);

		amount = Math.Min(amount, Math.Min(lifeRoom, infamy - minInfamy));
		if (infamy <= minInfamy)
		{
			m_GM.NotifyPlayer(playerId, name, "\"Nothing to pay for. Whatever's left, you work off with your hands, not your Favor.\"");
			return;
		}

		if (p.ForgiveLeft(f) <= 0)
		{
			m_GM.NotifyPlayer(playerId, name, "\"No. Not for Favor, not for anything.\" " + m_GM.ForgiveSpentText(f));
			return;
		}

		if (amount <= 0)
		{
			m_GM.NotifyPlayer(playerId, name, "\"You've paid all you can pay. The rest you'll have to earn back.\" (Amends contracts work off Infamy.)");
			return;
		}

		string key = string.Format("%1:%2", Ident(playerId), f);
		float last;
		float gap = m_Settings.m_fAmendsCooldownMinutes * 60;
		if (m_mAmendsAt.Find(key, last) && m_fClock - last < gap)
		{
			m_GM.NotifyPlayer(playerId, name, string.Format("\"You just paid. Let it settle. Come back in %1 minutes.\"", Math.Max(1, RZ_Util.RoundToInt((gap - (m_fClock - last)) / 60))));
			return;
		}

		int cost = AmendsCost(p.m_aAmendsPaid[f]);
		if (!m_GM.SpendFavor(playerId, f, cost))
		{
			m_GM.NotifyPlayer(playerId, name, string.Format("\"Making it right costs %1 Favor this time.\" You have %2.", cost, p.GetFavor(f)));
			return;
		}

		m_mAmendsAt.Set(key, m_fClock);
		p.m_aAmendsPaid[f] = p.m_aAmendsPaid[f] + 1;
		int taken = m_GM.Forgive(playerId, f, amount, "paid amends", true);
		p.m_aAmendsWorked[f] = p.m_aAmendsWorked[f] + taken;
		m_GM.NotifyPlayer(playerId, name, string.Format("%1 Favor changes hands and someone crosses out a line in a book. Infamy with %2 is now %3. The next payment will cost %4.",
			cost, RZ_Factions.GetNameInText(f), p.GetInfamy(f), AmendsCost(p.m_aAmendsPaid[f])));
	}

	//------------------------------------------------------------------------------------------------
	protected int AmendsCost(int paidBefore)
	{
		float cost = m_Settings.m_iAmendsCost;
		for (int i = 0; i < paidBefore; i++)
		{
			cost = cost * Math.Max(1, m_Settings.m_fAmendsGrowth);
		}

		return Math.Max(1, RZ_Util.RoundToInt(cost));
	}

	// ================================================================ Reputation clues
	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned(int playerId, IEntity ent)
	{
		GiveClue(playerId, ent, RZ_EFaction.BANDITS);
		GiveClue(playerId, ent, RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	void OnTierChanged(int playerId, RZ_EFaction faction, RZ_ETier after)
	{
		if (faction != RZ_EFaction.BANDITS && faction != RZ_EFaction.SURVIVORS)
			return;

		GiveClue(playerId, GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId), faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Once per life at Trusted: the faction's mark, into the pack (or at their feet). Wearing it is
	//! up to them; worn, other players can read it.
	protected void GiveClue(int playerId, IEntity ent, RZ_EFaction f)
	{
		if (!ent)
			return;

		ResourceName prefab = m_Settings.m_sHavenRibbonPrefab;
		string text = "Someone at the safehouse ties a strip of amber cloth round your sleeve. \"So people know you're one of the good ones.\" It's in your pack. Wear it or don't.";
		if (f == RZ_EFaction.BANDITS)
		{
			prefab = m_Settings.m_sRedHandArmbandPrefab;
			text = "A red armband, tossed over without a word. Wear it and the roads will know whose you are. So will everyone else.";
		}

		RZ_ETier tier = m_GM.GetTier(playerId, f);
		if (prefab.IsEmpty() || !RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED) || RZ_Reputation.IsHunted(tier))
			return;

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		int bit = RZ_Factions.Bit(f);
		if ((p.m_iCluesGiven & bit) != 0)
			return;

		IEntity item = RZ_Util.SpawnPrefab(prefab, ent.GetOrigin() + Vector(0, 0.3, 0));
		if (!item)
			return;

		p.m_iCluesGiven = p.m_iCluesGiven | bit;
		if (!RZ_Util.InsertIntoInventory(ent, item))
			RZ_GameModeComponent.TrackLooseItem(item);

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), text);
	}
}
