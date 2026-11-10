//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Marks a faction contact (quartermaster). Put shop, requisition, standing and hand-over actions in its ActionsManagerComponent.")]
class RZ_QuartermasterComponentClass : ScriptComponentClass
{
}

//! A faction's quartermaster: where you spend Favor, check your standing and
//! hear about contracts. Place one per faction in the world, or set the
//! quartermaster prefabs on the game mode and they spawn in suitable towns.
//! Players who walk within 15 m hear about the contracts they qualify for.
class RZ_QuartermasterComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, "Faction this quartermaster belongs to", "", ParamEnumArray.FromEnum(RZ_EFaction))]
	protected RZ_EFaction m_eFaction;

	protected static ref array<RZ_QuartermasterComponent> s_aAll;

	//------------------------------------------------------------------------------------------------
	RZ_EFaction GetFaction()
	{
		return m_eFaction;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_QuartermasterComponent Find(RZ_EFaction faction)
	{
		if (!s_aAll)
			return null;

		foreach (RZ_QuartermasterComponent qm : s_aAll)
		{
			if (qm && qm.GetFaction() == faction)
				return qm;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	static array<RZ_QuartermasterComponent> GetAll()
	{
		if (!s_aAll)
			s_aAll = new array<RZ_QuartermasterComponent>();

		return s_aAll;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		GetAll().Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);

		super.OnDelete(owner);
	}
}

//------------------------------------------------------------------------------------------------
//! Shared server-side helpers for quartermaster actions.
class RZ_QuartermasterHelper
{
	protected static ref array<IEntity> s_aBought;		// weak: what quartermasters sold lately (the fence won't buy it)
	protected static const int BOUGHT_MEMORY = 300;

	//------------------------------------------------------------------------------------------------
	static int GetPlayerId(IEntity user)
	{
		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns a prefab next to the quartermaster, a little in front of the buyer. Tracked as a loose
	//! item (vehicles aren't).
	static IEntity SpawnNear(ResourceName prefab, IEntity owner, IEntity user)
	{
		vector pos = owner.GetOrigin();
		if (user)
			pos = (pos + user.GetOrigin()) * 0.5;

		pos = RZ_Util.RandomPointInRing(pos, 0.5, 1.5) + Vector(0, 0.3, 0);
		IEntity bought = RZ_Util.SpawnPrefab(prefab, pos);
		RZ_GameModeComponent.TrackLooseItem(bought); // left lying there, it's cleaned up in time
		MarkBought(bought);
		return bought;
	}

	//------------------------------------------------------------------------------------------------
	//! Remembers a sold item, so the Red Hand fence (RZ_FactionServices) never buys it back.
	static void MarkBought(IEntity e)
	{
		if (!e)
			return;

		if (!s_aBought)
			s_aBought = new array<IEntity>();

		// Deleted items leave null entries: drop those first, then the oldest
		for (int i = s_aBought.Count() - 1; i >= 0; i--)
		{
			if (!s_aBought[i])
				s_aBought.RemoveOrdered(i);
		}

		while (s_aBought.Count() >= BOUGHT_MEMORY)
		{
			s_aBought.RemoveOrdered(0);
		}

		s_aBought.Insert(e);
	}

	//------------------------------------------------------------------------------------------------
	static bool WasBought(IEntity e)
	{
		return e && s_aBought && s_aBought.Contains(e);
	}
}

//------------------------------------------------------------------------------------------------
//! "Buy <item> (<cost> Favor)". Add one per item to a quartermaster's
//! ActionsManagerComponent and set its attributes.
//!
//! Tier and Favor live only on the server. Rather than replicating them to grey
//! the action out, the action is always shown and the server checks on use: if
//! the player's tier or Favor is too low, nothing is taken and they're told why.
class RZ_FactionShopAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Faction that sells this", "", ParamEnumArray.FromEnum(RZ_EFaction))]
	protected RZ_EFaction m_eFaction;

	[Attribute("1", UIWidgets.ComboBox, "Tier needed with the faction (use Stranger, Known, Trusted or Champion)", "", ParamEnumArray.FromEnum(RZ_ETier))]
	protected RZ_ETier m_eRequiredTier;

	[Attribute("25", desc: "Favor it costs")]
	protected int m_iFavorCost;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Item, vehicle or crate to spawn next to the quartermaster", "et")]
	protected ResourceName m_sItemPrefab;

	[Attribute("", desc: "Name shown in the action menu")]
	protected string m_sDisplayName;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = string.Format("Buy %1 (%2 Favor, %3)", m_sDisplayName, m_iFavorCost, RZ_Reputation.GetTierName(m_eRequiredTier));
		return true;
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

		string factionName = RZ_Factions.GetName(m_eFaction);
		RZ_ETier tier = gm.GetTier(playerId, m_eFaction);

		// The leader whose quartermaster this is: their traits and memory of you set the price,
		// and a Rogue one serves people the stronghold turned away if they've helped them
		RZ_Leaders leaders = gm.GetLeaders();
		RZ_Leader leader;
		if (leaders)
			leader = leaders.FindAt(m_eFaction, pOwnerEntity.GetOrigin());

		bool rogueFavour = false;
		if (leaders)
			rogueFavour = leaders.ShopOverride(leader, playerId);
		if (rogueFavour)
		{
			if (RZ_Reputation.GetAccessRank(m_eRequiredTier) > 1)
			{
				gm.NotifyPlayer(playerId, factionName, string.Format("\"%1 is for people the stronghold trusts. I can do the basics for you, quietly. Not that.\"", m_sDisplayName));
				return;
			}

			// Treated as Known, whatever the stronghold thinks
			tier = RZ_ETier.KNOWN;
		}

		if (m_eFaction == RZ_EFaction.SURVIVORS && RZ_Reputation.IsHunted(tier))
		{
			gm.NotifyPlayer(playerId, factionName, "The door stays shut. Nobody here will sell you anything.");
			return;
		}

		// A Haven safehouse only stocks what its size allows (Shelter: Known, Community and
		// Settlement: Trusted, Town: Champion). A quartermaster away from a safehouse sells everything.
		if (m_eFaction == RZ_EFaction.SURVIVORS && gm.GetSafehouses())
		{
			int cap = gm.GetSafehouses().GetShopRankCap(pOwnerEntity.GetOrigin());
			if (cap >= 0 && RZ_Reputation.GetAccessRank(m_eRequiredTier) > cap)
			{
				gm.NotifyPlayer(playerId, factionName, string.Format("This safehouse is too small to stock %1. A bigger one might have it.", m_sDisplayName));
				return;
			}
		}

		if (!RZ_Reputation.IsAtLeast(tier, m_eRequiredTier) || RZ_Reputation.IsHunted(tier))
		{
			gm.NotifyPlayer(playerId, factionName, string.Format("They won't sell you %1. You need to be %2 with them (you're %3).",
				m_sDisplayName, RZ_Reputation.GetTierName(m_eRequiredTier), RZ_Reputation.GetTierName(tier)));
			return;
		}

		if (m_sItemPrefab.IsEmpty())
		{
			gm.NotifyPlayer(playerId, factionName, "They're out of stock.");
			return;
		}

		// Antidotes come only from the lab's stock (docs/DESIGN.md, "Cure research"): Halcyon sells
		// the doses the lab has made; anyone else (the Red Hand) only the doses stolen from it
		bool antidote = m_sItemPrefab == gm.GetAntidotePrefab();
		bool fromLab = m_eFaction == RZ_EFaction.INSTITUTE;
		if (antidote && gm.GetCure())
		{
			int doses = gm.GetCure().GetStolenAntidotes();
			if (fromLab)
				doses = gm.GetCure().GetLabAntidotes();

			if (doses <= 0)
			{
				if (fromLab)
					gm.NotifyPlayer(playerId, factionName, string.Format("\"No doses left. The lab makes one about every %1 minutes, faster as the research goes. Come back.\"", RZ_Util.RoundToInt(gm.GetCure().AntidoteInterval() / 60)));
				else
					gm.NotifyPlayer(playerId, factionName, "\"Antidotes? Fresh out. Somebody would have to fetch us some from the lab.\"");
				return;
			}
		}

		// Stabilisers at Halcyon come from the lab's small stock too, and cost double until the
		// research reaches 50% (docs/DESIGN.md, "Treatments"). Anywhere else it's an ordinary item.
		bool stabiliser = false;
		float stockMult = 1;
		if (fromLab && gm.GetCure() && gm.GetInfection())
		{
			ResourceName stabPrefab = gm.GetInfection().GetSettings().m_sStabiliserPrefab;
			stabiliser = !stabPrefab.IsEmpty() && m_sItemPrefab == stabPrefab;
		}

		if (stabiliser)
		{
			if (gm.GetCure().GetLabStabilisers() <= 0)
			{
				gm.NotifyPlayer(playerId, factionName, string.Format("\"No stabilisers. We make about one every %1 minutes, and they go fast. Come back.\"", RZ_Util.RoundToInt(gm.GetCure().StabiliserInterval() / 60)));
				return;
			}

			stockMult = gm.GetCure().StabiliserPriceMult();
		}

		// Supply lines (docs/DESIGN.md): a shop whose truck didn't get through is short until the
		// next one does (some items out, the top tier maybe locked, dearer); a stronghold Under
		// Pressure keeps its top tier back. Morale moves prices a little.
		float supplyMult = stockMult;
		RZ_SupplyLines supply = gm.GetSupply();
		RZ_SupplyState shortage;
		if (supply)
			shortage = supply.FindShopState(m_eFaction, pOwnerEntity.GetOrigin());

		bool topTier = RZ_Reputation.GetAccessRank(m_eRequiredTier) >= 3;
		if (shortage && shortage.m_bShortage)
		{
			if ((topTier && shortage.m_bTopLocked) || supply.IsOutOfStock(shortage, m_sDisplayName))
			{
				gm.NotifyPlayer(playerId, factionName, string.Format("No %1 until the next truck gets through.", m_sDisplayName));
				return;
			}

			supplyMult = supplyMult * shortage.m_fPriceMult;
		}

		RZ_Territory territory = gm.GetTerritory();
		if (topTier && territory && territory.IsUnderPressure(m_eFaction))
		{
			RZ_Holding top = territory.GetStronghold(m_eFaction);
			if (top && RZ_Util.DistXZ(top.m_vPos, pOwnerEntity.GetOrigin()) < 150)
			{
				gm.NotifyPlayer(playerId, factionName, string.Format("Not while we're under pressure. The %1 stays locked up until things calm down.", m_sDisplayName));
				return;
			}
		}

		if (gm.GetFactionLife())
			supplyMult = supplyMult * gm.GetFactionLife().PriceMult(m_eFaction);

		int cost = m_iFavorCost;
		if (leaders && leader && cost > 0)
			cost = Math.Max(1, RZ_Util.RoundToInt(cost * leaders.PriceMult(leader, playerId)));

		if (cost > 0 && supplyMult != 1)
			cost = Math.Max(1, RZ_Util.RoundToInt(cost * supplyMult));

		if (!gm.SpendFavor(playerId, m_eFaction, cost))
		{
			gm.NotifyPlayer(playerId, factionName, string.Format("%1 costs %2 Favor here. You have %3.", m_sDisplayName, cost, gm.GetProfile(playerId).GetFavor(m_eFaction)));
			return;
		}

		if (!RZ_QuartermasterHelper.SpawnNear(m_sItemPrefab, pOwnerEntity, pUserEntity))
		{
			gm.AddFavor(playerId, m_eFaction, cost); // refund
			return;
		}

		if (antidote && gm.GetCure())
		{
			if (fromLab)
				gm.GetCure().TakeLabAntidote();
			else
				gm.GetCure().TakeStolenAntidote();
		}

		if (stabiliser)
			gm.GetCure().TakeStabiliser();

		gm.NotifyPlayer(playerId, factionName, string.Format("Here's your %1, for %2 Favor. %3 Favor left.", m_sDisplayName, cost, gm.GetProfile(playerId).GetFavor(m_eFaction)));
	}
}

//------------------------------------------------------------------------------------------------
//! "Check standing": tells the player their tier, Fame, Infamy and Favor with
//! every faction, plus this quartermaster's open contracts they qualify for.
class RZ_StandingStatusAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Check standing and work";
		return true;
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

		gm.NotifyPlayer(playerId, "Your standing", gm.DescribeStanding(playerId));

		RZ_QuartermasterComponent qm = RZ_QuartermasterComponent.Cast(pOwnerEntity.FindComponent(RZ_QuartermasterComponent));
		if (!qm)
			return;

		string work = gm.GetMissions().ListContractsFor(playerId, qm.GetFaction(), pUserEntity);
		if (work.IsEmpty())
			work = "Nothing for you right now.";

		gm.NotifyPlayer(playerId, "Work from " + RZ_Factions.GetName(qm.GetFaction()), work);
	}
}

//------------------------------------------------------------------------------------------------
//! Bastion Trusted perk "Requisition": once an hour, draw ammo or rations from
//! Bastion's stock at half the Favor cost. Put one per item on the Bastion
//! quartermaster and set its attributes, like RZ_FactionShopAction. All of them
//! share one hourly allowance per player.
//! With supply trucks running for Bastion, it's signed out at a Bastion outpost's
//! quartermaster, from the stock its trucks brought (one unit each), and refused while that
//! outpost is short or hasn't been resupplied (RZ_FactionPerks.TryRequisition).
class RZ_RequisitionAction : ScriptedUserAction
{
	[Attribute("30", desc: "Full Favor price. Requisition charges half of it, rounded up")]
	protected int m_iFavorCost;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Ammo or rations to hand out, spawned next to the quartermaster", "et")]
	protected ResourceName m_sItemPrefab;

	[Attribute("rations", desc: "Name shown in the action menu")]
	protected string m_sDisplayName;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = string.Format("Requisition %1 (%2 Favor, Bastion Trusted, once an hour)", m_sDisplayName, RZ_FactionPerks.RequisitionPrice(m_iFavorCost));
		return true;
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
		if (gm && playerId > 0)
			gm.TryRequisition(playerId, pOwnerEntity, pUserEntity, m_sItemPrefab, m_sDisplayName, m_iFavorCost);
	}
}
