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
	//------------------------------------------------------------------------------------------------
	static int GetPlayerId(IEntity user)
	{
		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns a prefab next to the quartermaster, a little in front of the buyer.
	static IEntity SpawnNear(ResourceName prefab, IEntity owner, IEntity user)
	{
		vector pos = owner.GetOrigin();
		if (user)
			pos = (pos + user.GetOrigin()) * 0.5;

		pos = RZ_Util.RandomPointInRing(pos, 0.5, 1.5) + Vector(0, 0.3, 0);
		return RZ_Util.SpawnPrefab(prefab, pos);
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

		int cost = m_iFavorCost;
		if (leaders && leader && cost > 0)
			cost = Math.Max(1, RZ_Util.RoundToInt(cost * leaders.PriceMult(leader, playerId)));

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
//! TODO (docs/WORLD.md, supply trucks): draw from an outpost's truck-supplied stock
//! instead of the quartermaster, and refuse while that outpost hasn't been resupplied.
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
