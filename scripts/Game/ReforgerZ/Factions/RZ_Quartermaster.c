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
	protected static const float SERVE_LOOK = 5;		// how far each way he could face is looked down
	protected static const float SERVE_ROOM = 2;		// a way with a wall closer than this is never his

	//------------------------------------------------------------------------------------------------
	//! The way a quartermaster standing at `at` serves from (docs/DESIGN.md, "Camp life", the
	//! counter): of sixteen ways, the most open one (traced at waist and head height, up to 5 m),
	//! the one toward `toward` (the customers' side) when it's about as open. Never a way with a wall
	//! within 2 m while another has room, so he never faces a wall or into a corner; out of a kiosk
	//! or a shop he faces its open side.
	static float ServeYaw(vector at, vector toward)
	{
		float want = RZ_Compass.Bearing(at, toward);
		float best = want;
		float bestScore = -float.MAX;
		for (int k = 0; k < 16; k++)
		{
			float bearing = k * 22.5;
			float open = OpenTo(at, bearing);
			float off = Math.AbsFloat(RZ_Compass.Normalize(bearing - want + 180) - 180);
			float score = open - off / 60;
			if (open < SERVE_ROOM)
				score = score - 100;

			if (score > bestScore)
			{
				bestScore = score;
				best = bearing;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! How far he can see that way before a wall, a fence or a prop, up to SERVE_LOOK.
	static float OpenTo(vector at, float bearing)
	{
		float rad = bearing * Math.DEG2RAD;
		vector dir = Vector(Math.Sin(rad), 0, Math.Cos(rad));
		float open = SERVE_LOOK;
		for (int h = 0; h < 2; h++)
		{
			TraceParam param = new TraceParam();
			param.Start = at + Vector(0, 1.0 + h * 0.5, 0);
			param.End = param.Start + dir * SERVE_LOOK;
			param.LayerMask = EPhysicsLayerPresets.Projectile;
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			float frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param);
			open = Math.Min(open, Math.Clamp(frac, 0, 1) * SERVE_LOOK);
		}

		return open;
	}

	//------------------------------------------------------------------------------------------------
	//! Turns a quartermaster (or anyone standing) to face `yaw`, where he stands.
	static void Face(IEntity person, float yaw)
	{
		if (!person)
			return;

		vector mat[4];
		person.GetWorldTransform(mat);
		vector pos = mat[3];
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), mat);
		mat[3] = pos;
		person.SetWorldTransform(mat); // VERIFY: turning a standing AI character with SetWorldTransform on the server keeps it facing that way (it stands with no job) and replicates
	}

	//------------------------------------------------------------------------------------------------
	static int GetPlayerId(IEntity user)
	{
		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns a prefab for the buyer: into their inventory if it fits, else on the ground between
	//! the quartermaster and the buyer, tracked as a loose item (quartermasters never sell
	//! vehicles: IsForbiddenStock).
	static IEntity SpawnNear(ResourceName prefab, IEntity owner, IEntity user)
	{
		vector pos = owner.GetOrigin();
		if (user)
			pos = (pos + user.GetOrigin()) * 0.5;

		pos = RZ_Util.RandomPointInRing(pos, 0.5, 1.5) + Vector(0, 0.3, 0);
		IEntity bought = RZ_Util.SpawnPrefab(prefab, pos);
		if (!bought)
			return null;

		MarkBought(bought);
		if (!RZ_Util.InsertIntoInventory(user, bought))
			RZ_GameModeComponent.TrackLooseItem(bought); // left lying there, it's cleaned up in time

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

	//------------------------------------------------------------------------------------------------
	//! Never sold or handed out by a quartermaster: a vehicle, or a part or fuel that fixes a car
	//! (Interactions > Cars). Cars are only ever found. A shop or requisition action set to one
	//! refuses, and the log says which.
	static bool IsForbiddenStock(ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return false;

		string lower = prefab;
		lower.ToLower();
		if (lower.Contains("/vehicles/") || RZ_Util.IsVehiclePrefab(prefab))
			return true;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetCarSettings())
			return false;

		RZ_CarSettings cars = gm.GetCarSettings();
		if (prefab == cars.m_sBatteryPrefab || prefab == cars.m_sTyrePrefab || prefab == cars.m_sSparkPrefab || prefab == cars.m_sFuelPrefab)
			return true;

		if (cars.m_aCarPrefabs && cars.m_aCarPrefabs.Contains(prefab))
			return true;

		return cars.m_aParkedPrefabs && cars.m_aParkedPrefabs.Contains(prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! The lowest tier, as an access rank (RZ_Reputation.GetAccessRank), at which this
	//! quartermaster's shop actions sell the prefab; -1 if none of them does.
	static int StockRank(IEntity quartermaster, ResourceName prefab)
	{
		if (!quartermaster || prefab.IsEmpty())
			return -1;

		ActionsManagerComponent actions = ActionsManagerComponent.Cast(quartermaster.FindComponent(ActionsManagerComponent));
		if (!actions)
			return -1;

		array<BaseUserAction> list = {};
		actions.GetActionsList(list);
		int best = -1;
		foreach (BaseUserAction a : list)
		{
			RZ_FactionShopAction shop = RZ_FactionShopAction.Cast(a);
			if (!shop || shop.GetItemPrefab() != prefab)
				continue;

			int rank = RZ_Reputation.GetAccessRank(shop.GetRequiredTier());
			if (best < 0 || rank < best)
				best = rank;
		}

		return best;
	}
}

//------------------------------------------------------------------------------------------------
//! "Buy <item> (<cost> Favor)". Add one per item to a quartermaster's
//! ActionsManagerComponent and set its attributes.
//!
//! Tier and Favor live only on the server. Rather than replicating them to grey
//! the action out, the action is always shown and the server checks on use: if
//! the player's tier or Favor is too low, nothing is taken and they're told why.
//! While the shop screen is used (RZ_ShopMenu) the action hides and the screen lists it instead;
//! a purchase there goes through the same Quote and Buy.
class RZ_FactionShopAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Faction that sells this", "", ParamEnumArray.FromEnum(RZ_EFaction))]
	protected RZ_EFaction m_eFaction;

	[Attribute("1", UIWidgets.ComboBox, "Tier needed with the faction (use Stranger, Known, Trusted or Champion)", "", ParamEnumArray.FromEnum(RZ_ETier))]
	protected RZ_ETier m_eRequiredTier;

	[Attribute("25", desc: "Favor it costs")]
	protected int m_iFavorCost;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Item or crate to spawn next to the quartermaster (never a vehicle or a car part: cars are only found)", "et")]
	protected ResourceName m_sItemPrefab;

	[Attribute("", desc: "Name shown in the action menu")]
	protected string m_sDisplayName;

	[Attribute("0", desc: "Sold once a life to each player (the Red Hand's clean kit sets on the Trusted shelf)")]
	protected bool m_bOncePerLife;

	static const int STATE_OPEN = 0;	// for sale to this player
	static const int STATE_LOCKED = 1;	// not at their standing
	static const int STATE_OUT = 2;		// nobody can have it here right now

	//------------------------------------------------------------------------------------------------
	ResourceName GetItemPrefab()
	{
		return m_sItemPrefab;
	}

	//------------------------------------------------------------------------------------------------
	RZ_ETier GetRequiredTier()
	{
		return m_eRequiredTier;
	}

	//------------------------------------------------------------------------------------------------
	RZ_EFaction GetFaction()
	{
		return m_eFaction;
	}

	//------------------------------------------------------------------------------------------------
	string GetDisplayName()
	{
		return m_sDisplayName;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = string.Format("Buy %1 (%2 Favor, %3)", m_sDisplayName, m_iFavorCost, RZ_Reputation.GetTierName(m_eRequiredTier));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Hidden on a quartermaster while this machine uses the shop screen: it's a row there.
	override bool CanBeShownScript(IEntity user)
	{
		return !RZ_ShopMenu.HidesActionsOn(GetOwner());
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

		Buy(gm, playerId, pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! What it costs this player at this quartermaster now, or -1 if they can't have it. Then
	//! `refusal` is what they're told when they try, `need` a short line for the shop screen
	//! ("Needs Champion with the Haven") and `state` STATE_LOCKED or STATE_OUT. Takes nothing.
	int Quote(RZ_GameModeComponent gm, int playerId, IEntity owner, out string refusal, out string need, out int state)
	{
		refusal = "";
		need = "";
		state = STATE_OPEN;
		string factionName = RZ_Factions.GetName(m_eFaction);
		RZ_ETier tier = gm.GetTier(playerId, m_eFaction);

		// The leader whose quartermaster this is: their traits and memory of you set the price,
		// and a Rogue one serves people the stronghold turned away if they've helped them
		RZ_Leaders leaders = gm.GetLeaders();
		RZ_Leader leader;
		if (leaders)
			leader = leaders.FindAt(m_eFaction, owner.GetOrigin());

		// Only for people the stronghold ranks below Known: a better customer keeps their own tier
		bool rogueFavour = false;
		if (leaders && !RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN))
			rogueFavour = leaders.ShopOverride(leader, playerId);
		if (rogueFavour)
		{
			if (RZ_Reputation.GetAccessRank(m_eRequiredTier) > 1)
			{
				refusal = string.Format("\"%1 is for people the stronghold trusts. I can do the basics for you, quietly. Not that.\"", m_sDisplayName);
				need = "Only the basics, quietly";
				state = STATE_LOCKED;
				return -1;
			}

			// Treated as Known, whatever the stronghold thinks
			tier = RZ_ETier.KNOWN;
		}

		// Colours: not to anyone in their enemy's colours, friends of theirs included (RZ_Disguise)
		RZ_Hostility hostility = gm.GetHostility();
		if (hostility && hostility.GetDisguise().IsOn())
		{
			string colours = hostility.GetDisguise().ShopRefusal(playerId, m_eFaction);
			if (!colours.IsEmpty())
			{
				refusal = colours;
				need = "Not in those colours";
				state = STATE_LOCKED;
				return -1;
			}
		}

		// Once a life (the Red Hand's clean kit sets on the Trusted shelf)
		RZ_WorldFacts facts = gm.GetFacts();
		if (m_bOncePerLife && facts && facts.Flag(OnceKey(gm, playerId)) > 0)
		{
			refusal = string.Format("\"One %1 a life. You've had yours.\"", m_sDisplayName);
			need = "Once a life: you've had it";
			state = STATE_LOCKED;
			return -1;
		}

		if (m_eFaction == RZ_EFaction.SURVIVORS && RZ_Reputation.IsHunted(tier))
		{
			refusal = "The door stays shut. Nobody here will sell you anything.";
			need = "They won't sell to you";
			state = STATE_LOCKED;
			return -1;
		}

		// A Haven safehouse only stocks what its size allows (Shelter: Known, Community and
		// Settlement: Trusted, Town: Champion). A quartermaster away from a safehouse sells everything.
		if (m_eFaction == RZ_EFaction.SURVIVORS && gm.GetSafehouses())
		{
			int cap = gm.GetSafehouses().GetShopRankCap(owner.GetOrigin());
			if (cap >= 0 && RZ_Reputation.GetAccessRank(m_eRequiredTier) > cap)
			{
				refusal = string.Format("This safehouse is too small to stock %1. A bigger one might have it.", m_sDisplayName);
				need = "A bigger safehouse stocks it";
				state = STATE_OUT;
				return -1;
			}
		}

		if (!RZ_Reputation.IsAtLeast(tier, m_eRequiredTier) || RZ_Reputation.IsHunted(tier))
		{
			refusal = string.Format("They won't sell you %1. You need to be %2 with them (you're %3).",
				m_sDisplayName, RZ_Reputation.GetTierName(m_eRequiredTier), RZ_Reputation.GetTierName(tier));
			need = string.Format("Needs %1 with %2", RZ_Reputation.GetTierName(m_eRequiredTier), RZ_Factions.GetNameInText(m_eFaction));
			if (RZ_Reputation.IsHunted(tier))
				need = "They won't sell to you";

			state = STATE_LOCKED;
			return -1;
		}

		// Cars are only ever found: no quartermaster sells a vehicle or a car part
		if (m_sItemPrefab.IsEmpty() || RZ_QuartermasterHelper.IsForbiddenStock(m_sItemPrefab))
		{
			refusal = "They're out of stock.";
			need = "Out of stock";
			state = STATE_OUT;
			return -1;
		}

		// Antidotes come only from the lab's stock (docs/DESIGN.md, "Cure research"): Halcyon sells
		// the doses the lab has made; anyone else (the Red Hand) only the doses stolen from it
		bool fromLab = m_eFaction == RZ_EFaction.INSTITUTE;
		if (IsAntidote(gm))
		{
			int doses = gm.GetCure().GetStolenAntidotes();
			if (fromLab)
				doses = gm.GetCure().GetLabAntidotes();

			if (doses <= 0)
			{
				refusal = "\"Antidotes? Fresh out. Somebody would have to fetch us some from the lab.\"";
				if (fromLab)
					refusal = string.Format("\"No doses left. The lab makes one about every %1 minutes, faster as the research goes. Come back.\"", RZ_Util.RoundToInt(gm.GetCure().AntidoteInterval() / 60));

				need = "No doses left";
				state = STATE_OUT;
				return -1;
			}
		}

		// Stabilisers at Halcyon come from the lab's small stock too, and cost double until the
		// research reaches 50% (docs/DESIGN.md, "Treatments"). Anywhere else it's an ordinary item.
		float supplyMult = 1;
		if (IsLabStabiliser(gm))
		{
			if (gm.GetCure().GetLabStabilisers() <= 0)
			{
				refusal = string.Format("\"No stabilisers. We make about one every %1 minutes, and they go fast. Come back.\"", RZ_Util.RoundToInt(gm.GetCure().StabiliserInterval() / 60));
				need = "None left";
				state = STATE_OUT;
				return -1;
			}

			supplyMult = gm.GetCure().StabiliserPriceMult();
		}

		// Supply lines (docs/DESIGN.md): a shop whose truck didn't get through is short until the
		// next one does (some items out, the top tier maybe locked, dearer); a stronghold Under
		// Pressure keeps its top tier back. Morale moves prices a little.
		RZ_SupplyLines supply = gm.GetSupply();
		RZ_SupplyState shortage;
		if (supply)
			shortage = supply.FindShopState(m_eFaction, owner.GetOrigin());

		bool topTier = RZ_Reputation.GetAccessRank(m_eRequiredTier) >= 3;
		if (shortage && shortage.m_bShortage)
		{
			if ((topTier && shortage.m_bTopLocked) || supply.IsOutOfStock(shortage, m_sDisplayName))
			{
				refusal = string.Format("No %1 until the next truck gets through.", m_sDisplayName);
				need = "Not until the next truck";
				state = STATE_OUT;
				return -1;
			}

			supplyMult = supplyMult * shortage.m_fPriceMult;
		}

		RZ_Territory territory = gm.GetTerritory();
		if (topTier && territory && territory.IsUnderPressure(m_eFaction))
		{
			RZ_Holding top = territory.GetStronghold(m_eFaction);
			if (top && RZ_Util.DistXZ(top.m_vPos, owner.GetOrigin()) < 150)
			{
				refusal = string.Format("Not while we're under pressure. The %1 stays locked up until things calm down.", m_sDisplayName);
				need = "Locked up while they're under pressure";
				state = STATE_OUT;
				return -1;
			}
		}

		if (gm.GetFactionLife())
			supplyMult = supplyMult * gm.GetFactionLife().PriceMult(m_eFaction);

		int cost = m_iFavorCost;
		if (leaders && leader && cost > 0)
			cost = Math.Max(1, RZ_Util.RoundToInt(cost * leaders.PriceMult(leader, playerId)));

		if (cost > 0 && supplyMult != 1)
			cost = Math.Max(1, RZ_Util.RoundToInt(cost * supplyMult));

		return cost;
	}

	//------------------------------------------------------------------------------------------------
	//! The world flag a once-a-life item sets, by identity and life (saved with the world facts).
	protected string OnceKey(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		return string.Format("shop.once.%1.%2.%3", p.m_sIdentity, p.m_iLives, m_sItemPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! The antidote, which only the lab's (or the stolen) stock sells.
	protected bool IsAntidote(RZ_GameModeComponent gm)
	{
		return gm.GetCure() && m_sItemPrefab == gm.GetAntidotePrefab();
	}

	//------------------------------------------------------------------------------------------------
	//! A stabiliser at Halcyon, from the lab's small stock.
	protected bool IsLabStabiliser(RZ_GameModeComponent gm)
	{
		if (m_eFaction != RZ_EFaction.INSTITUTE || !gm.GetCure() || !gm.GetInfection())
			return false;

		ResourceName stabPrefab = gm.GetInfection().GetSettings().m_sStabiliserPrefab;
		return !stabPrefab.IsEmpty() && m_sItemPrefab == stabPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Buys it for the player (server): the checks in Quote, then the Favor, the item next to the
	//! quartermaster and the lab's stock. Every outcome is told as a notice. True if bought.
	bool Buy(RZ_GameModeComponent gm, int playerId, IEntity owner, IEntity user)
	{
		string factionName = RZ_Factions.GetName(m_eFaction);
		string refusal;
		string need;
		int state;
		int cost = Quote(gm, playerId, owner, refusal, need, state);
		if (cost < 0)
		{
			if (state == STATE_OUT && !m_sItemPrefab.IsEmpty() && RZ_QuartermasterHelper.IsForbiddenStock(m_sItemPrefab))
				RZ_Log.Warn("Shop action '" + m_sDisplayName + "' sells a vehicle or a car part (" + m_sItemPrefab + "): refused. Cars are only found");

			gm.NotifyPlayer(playerId, factionName, refusal);
			return false;
		}

		if (!gm.SpendFavor(playerId, m_eFaction, cost))
		{
			gm.NotifyPlayer(playerId, factionName, string.Format("%1 costs %2 Favor here. You have %3.", m_sDisplayName, cost, gm.GetProfile(playerId).GetFavor(m_eFaction)));
			return false;
		}

		IEntity bought = RZ_QuartermasterHelper.SpawnNear(m_sItemPrefab, owner, user);
		if (!bought)
		{
			gm.AddFavor(playerId, m_eFaction, cost); // refund
			return false;
		}

		// Borrowed kit: their own kit is signed out to the buyer; anything the Red Hand sells is bought fair
		if (gm.GetHostility() && gm.GetHostility().GetDisguise().MarksOn())
			gm.GetHostility().GetDisguise().GetMarks().OnSold(bought, playerId, m_eFaction);

		if (m_bOncePerLife && gm.GetFacts())
			gm.GetFacts().SetFlag(OnceKey(gm, playerId), 1);

		bool fromLab = m_eFaction == RZ_EFaction.INSTITUTE;
		if (IsAntidote(gm))
		{
			if (fromLab)
				gm.GetCure().TakeLabAntidote();
			else
				gm.GetCure().TakeStolenAntidote();
		}

		if (IsLabStabiliser(gm))
			gm.GetCure().TakeStabiliser();

		gm.NotifyPlayer(playerId, factionName, string.Format("Here's your %1, for %2 Favor. %3 Favor left.", m_sDisplayName, cost, gm.GetProfile(playerId).GetFavor(m_eFaction)));
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! "Check standing": tells the player their tier, Fame, Infamy and Favor with
//! every faction, plus this quartermaster's open contracts they qualify for.
class RZ_StandingStatusAction : RZ_GroupedAction
{
	//------------------------------------------------------------------------------------------------
	override bool Label(out string outName)
	{
		outName = "Check standing and work";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(IEntity pOwnerEntity, IEntity pUserEntity)
	{
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
//! It's a Trusted perk, so it hands out only what the same quartermaster sells at Trusted or
//! below: an item stocked only at Champion, or not stocked there at all, is refused, and so is
//! a vehicle or a car part.
class RZ_RequisitionAction : ScriptedUserAction
{
	[Attribute("30", desc: "Full Favor price. Requisition charges half of it, rounded up")]
	protected int m_iFavorCost;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Ammo or rations to hand out (its prefab name must hold an ammo or food word). Must be an item a shop action on the same quartermaster sells at Trusted or below", "et")]
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
	//! Hidden on a quartermaster while this machine uses the shop screen: it's a row there.
	override bool CanBeShownScript(IEntity user)
	{
		return !RZ_ShopMenu.HidesActionsOn(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	ResourceName GetItemPrefab() { return m_sItemPrefab; }
	string GetDisplayName() { return m_sDisplayName; }
	//! What it charges: half the full price, rounded up.
	int GetPrice() { return RZ_FactionPerks.RequisitionPrice(m_iFavorCost); }

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		Requisition(gm, playerId, pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Signs it out for the player (server): the action, or its row on the shop screen.
	void Requisition(RZ_GameModeComponent gm, int playerId, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Each tier hands out only its own tier's stock, and Requisition is Bastion Trusted
		if (!m_sItemPrefab.IsEmpty())
		{
			int rank = RZ_QuartermasterHelper.StockRank(pOwnerEntity, m_sItemPrefab);
			bool forbidden = RZ_QuartermasterHelper.IsForbiddenStock(m_sItemPrefab);
			if (forbidden || rank < 0 || rank > RZ_Reputation.GetAccessRank(RZ_ETier.TRUSTED))
			{
				RZ_Log.Warn("Requisition '" + m_sDisplayName + "' (" + m_sItemPrefab + ") isn't Trusted stock here (a shop action on this quartermaster at Trusted or below, never a vehicle or a car part): refused");
				gm.NotifyPlayer(playerId, RZ_Factions.GetName(RZ_EFaction.MILITARY), string.Format("\"We don't sign out %1. Buy it, if you can.\"", m_sDisplayName));
				return;
			}
		}

		gm.TryRequisition(playerId, pOwnerEntity, pUserEntity, m_sItemPrefab, m_sDisplayName, m_iFavorCost);
	}
}

//------------------------------------------------------------------------------------------------
//! "Trade": opens the quartermaster's shop screen (Factions/RZ_ShopScreen.c, docs/UI.md "The
//! shop screen"). Shown only while this machine uses the screen; then the Buy, Requisition and
//! service actions hide, and come back without it.
class RZ_ShopOpenAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Trade";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return RZ_ShopMenu.Available();
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

		RZ_ShopScreen.Open(gm, playerId, pOwnerEntity, pUserEntity);
	}
}
