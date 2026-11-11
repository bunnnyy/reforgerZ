//! The quartermaster's shop screen, server side (docs/UI.md, "The shop screen"). "Trade" at a
//! quartermaster (RZ_ShopOpenAction) opens it for one player: the quartermaster's own Buy actions
//! as rows (price for this player, the standing they need, in or out of stock), its Requisition
//! actions and services on the Services tab. A pick comes back as an index and runs the very
//! same action code (RZ_FactionShopAction.Buy, RZ_RequisitionAction.Requisition,
//! RZ_FactionServiceAction.Run), with what it says caught for the screen's status line. Nothing is
//! sold here that the actions don't sell: never a car or a car part.
//!
//! Payload (RZ_UIFeed.PAYLOAD_SHOP): S|serial|faction|title|sub|standing|favor|status, then one
//! I|index|tab|name|price|state|need|prefab|button per row; X|reason closes it; F|why tells the
//! client the screen isn't available, so the actions show for a minute.

//------------------------------------------------------------------------------------------------
class RZ_ShopScreen
{
	static const int TAB_FOOD = 0;
	static const int TAB_MEDICAL = 1;
	static const int TAB_GEAR = 2;
	static const int TAB_WEAPONS = 3;
	static const int TAB_SERVICES = 4;
	static const int TABS = 5;

	static const int ROW_SHOP = 0;
	static const int ROW_REQUISITION = 1;
	static const int ROW_SERVICE = 2;

	protected static const int MAX_ROWS = 40;

	protected static ref map<int, IEntity> s_mOwner;		// player -> the quartermaster their screen shows
	protected static ref map<int, int> s_mSerial;			// player -> that screen's serial
	protected static int s_iNextSerial = 1;

	//------------------------------------------------------------------------------------------------
	//! UI > Shop item layout is set (the client checks the same).
	static bool Available()
	{
		return !RZ_UISettings.Get().m_sShopItemLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! "Trade": opens the screen on this quartermaster for the player. When it can't (no layout,
	//! no UI component, the server at its cap of open screens) the client is told, and shows the
	//! quartermaster's actions again.
	static void Open(RZ_GameModeComponent gm, int playerId, IEntity owner, IEntity user)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!ui)
			return;

		RZ_ScreenGuard guard = gm.GetScreenGuard();
		RZ_QuartermasterComponent qm = RZ_QuartermasterComponent.Cast(owner.FindComponent(RZ_QuartermasterComponent));
		if (!guard || !qm || !Available())
		{
			ui.SendPayload(RZ_UIFeed.PAYLOAD_SHOP, "F|off");
			return;
		}

		if (!guard.Open(playerId, RZ_ScreenGuard.KIND_SHOP, owner, RZ_Factions.GetName(qm.GetFaction()), qm.GetFaction()))
		{
			ui.SendPayload(RZ_UIFeed.PAYLOAD_SHOP, "F|busy");
			return;
		}

		if (!s_mOwner)
		{
			s_mOwner = new map<int, IEntity>();
			s_mSerial = new map<int, int>();
		}

		s_mOwner.Set(playerId, owner);
		s_mSerial.Set(playerId, s_iNextSerial);
		s_iNextSerial++;
		Send(gm, playerId, "");
	}

	//------------------------------------------------------------------------------------------------
	//! The screen guard or the player closed it.
	static void OnScreenGone(int playerId)
	{
		if (!s_mOwner)
			return;

		s_mOwner.Remove(playerId);
		s_mSerial.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The quartermaster's actions the screen lists, in the screen's order: Buy actions, then
	//! Requisition, then services. `kinds` holds ROW_* for each.
	static void Rows(IEntity owner, notnull array<ScriptedUserAction> rows, notnull array<int> kinds)
	{
		rows.Clear();
		kinds.Clear();
		ActionsManagerComponent actions = ActionsManagerComponent.Cast(owner.FindComponent(ActionsManagerComponent));
		if (!actions)
			return;

		array<BaseUserAction> list = {};
		actions.GetActionsList(list);
		for (int pass = ROW_SHOP; pass <= ROW_SERVICE; pass++)
		{
			foreach (BaseUserAction a : list)
			{
				if (rows.Count() >= MAX_ROWS)
					return;

				ScriptedUserAction sa = ScriptedUserAction.Cast(a);
				if (!sa)
					continue;

				bool take = false;
				if (pass == ROW_SHOP)
					take = RZ_FactionShopAction.Cast(sa) != null;
				else if (pass == ROW_REQUISITION)
					take = RZ_RequisitionAction.Cast(sa) != null;
				else
					take = RZ_FactionServiceAction.Cast(sa) != null;

				if (!take)
					continue;

				rows.Insert(sa);
				kinds.Insert(pass);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the player's screen as it stands now, with `status` (what the last pick said).
	static void Send(RZ_GameModeComponent gm, int playerId, string status)
	{
		IEntity owner;
		int serial;
		if (!s_mOwner || !s_mOwner.Find(playerId, owner) || !s_mSerial.Find(playerId, serial))
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!ui)
			return;

		RZ_QuartermasterComponent qm;
		if (owner)
			qm = RZ_QuartermasterComponent.Cast(owner.FindComponent(RZ_QuartermasterComponent));

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		if (!qm || !profile)
		{
			Close(gm, playerId, "gone");
			return;
		}

		RZ_EFaction faction = qm.GetFaction();
		string town = "";
		RZ_Settlement here = gm.GetSettlement(gm.FindNearestSettlement(owner.GetOrigin()));
		if (here)
			town = here.m_sName;

		string sub = "Quartermaster";
		RZ_Leader leader;
		if (gm.GetLeaders())
			leader = gm.GetLeaders().FindAt(faction, owner.GetOrigin());

		if (leader)
			sub = leader.m_sName + ", quartermaster";

		if (!town.IsEmpty())
			sub += " in " + town;

		int factionId = faction;
		string payload = string.Format("S|%1|%2|%3|%4|%5|%6|%7", serial, factionId, RZ_Factions.GetShortName(faction) + " stores", RZ_UIText.Clean(sub, 80),
			"You're " + RZ_Reputation.GetTierName(gm.GetTier(playerId, faction)), profile.GetFavor(faction), RZ_UIText.Clean(status, 400));

		array<ScriptedUserAction> rows = {};
		array<int> kinds = {};
		Rows(owner, rows, kinds);
		foreach (int i, ScriptedUserAction a : rows)
		{
			payload += "\n" + RowRecord(gm, playerId, owner, i, a, kinds[i]);
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_SHOP, payload);
	}

	//------------------------------------------------------------------------------------------------
	//! I|index|tab|name|price|state|need|prefab|button. price -1: none to show (a service).
	protected static string RowRecord(RZ_GameModeComponent gm, int playerId, IEntity owner, int index, ScriptedUserAction a, int kind)
	{
		string name;
		string need = "";
		string button = "Buy";
		ResourceName prefab = "";
		int price = -1;
		int state = RZ_FactionShopAction.STATE_OPEN;
		int tab = TAB_SERVICES;
		if (kind == ROW_SHOP)
		{
			RZ_FactionShopAction shop = RZ_FactionShopAction.Cast(a);
			string refusal;
			name = shop.GetDisplayName();
			prefab = shop.GetItemPrefab();
			tab = TabFor(prefab, name);
			price = shop.Quote(gm, playerId, owner, refusal, need, state);
			if (price < 0)
				button = "";
		}
		else if (kind == ROW_REQUISITION)
		{
			RZ_RequisitionAction req = RZ_RequisitionAction.Cast(a);
			name = "Requisition: " + req.GetDisplayName();
			prefab = req.GetItemPrefab();
			price = req.GetPrice();
			button = "Sign out";
			need = "Bastion Trusted, once an hour";
			RZ_ETier tier = gm.GetTier(playerId, RZ_EFaction.MILITARY);
			if (!RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED) || RZ_Reputation.IsHunted(tier))
			{
				state = RZ_FactionShopAction.STATE_LOCKED;
				need = "Needs Trusted with Bastion";
				button = "";
			}
		}
		else
		{
			RZ_FactionServiceAction service = RZ_FactionServiceAction.Cast(a);
			name = service.GetLabel();
			button = "Ask";
		}

		return string.Format("I|%1|%2|%3|%4|%5|%6|%7|%8", index, tab, RZ_UIText.Clean(name, 90), price, state, RZ_UIText.Clean(need, 60), prefab, button);
	}

	//------------------------------------------------------------------------------------------------
	//! A pick on the screen (RZ_PlayerUIComponent.RpcAsk_ShopBuy). Checked here: the screen is open
	//! on this serial, the quartermaster is still there and the player close; then the row's own
	//! action code decides (standing, Favor, stock), and the screen is sent again with what it said.
	static void Buy(RZ_GameModeComponent gm, int playerId, int serial, int index)
	{
		IEntity owner;
		int current;
		if (!s_mOwner || !s_mOwner.Find(playerId, owner) || !s_mSerial.Find(playerId, current) || serial != current)
			return;

		RZ_ScreenGuard guard = gm.GetScreenGuard();
		if (!guard || !guard.IsOpen(playerId, RZ_ScreenGuard.KIND_SHOP))
		{
			OnScreenGone(playerId);
			return;
		}

		guard.Touch(playerId);
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!owner || !user || !RZ_Util.IsCharacterAlive(user))
		{
			Close(gm, playerId, "gone");
			return;
		}

		if (RZ_Util.DistXZ(user.GetOrigin(), owner.GetOrigin()) > RZ_DialogueSettings.Get().m_fRange)
			return;

		array<ScriptedUserAction> rows = {};
		array<int> kinds = {};
		Rows(owner, rows, kinds);
		if (index < 0 || index >= rows.Count())
			return;

		RZ_ScreenCapture.Begin(playerId);
		int kind = kinds[index];
		if (kind == ROW_SHOP)
			RZ_FactionShopAction.Cast(rows[index]).Buy(gm, playerId, owner, user);
		else if (kind == ROW_REQUISITION)
			RZ_RequisitionAction.Cast(rows[index]).Requisition(gm, playerId, owner, user);
		else
			RZ_FactionServiceAction.Cast(rows[index]).Run(gm, playerId, owner, user);

		string said = RZ_ScreenCapture.End();
		if (s_mOwner.Contains(playerId))
			Send(gm, playerId, said);
		else if (!said.IsEmpty())
			gm.NotifyPlayer(playerId, RZ_ScreenCapture.LastTitle(), said);
	}

	//------------------------------------------------------------------------------------------------
	//! Closes the player's screen from here (the quartermaster is gone).
	protected static void Close(RZ_GameModeComponent gm, int playerId, string reason)
	{
		OnScreenGone(playerId);
		if (gm.GetScreenGuard())
			gm.GetScreenGuard().Close(playerId);

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_SHOP, "X|" + reason);
	}

	//------------------------------------------------------------------------------------------------
	//! The tab an item goes on, from its prefab path and name. Simplified: a guess by words.
	static int TabFor(ResourceName prefab, string name)
	{
		string s = prefab + " " + name;
		s.ToLower();
		array<string> gear = {"flare", "smoke", "radio", "backpack", "rucksack", "torch", "flashlight", "plank", "kit"};
		array<string> medkits = {"medkit", "surgery", "first aid", "firstaid"};
		array<string> medical = {"/medicine/", "bandage", "gauze", "dressing", "morphine", "saline", "antidote", "stabil", "tablet", "pill", "antibiotic", "syringe", "suppressant", "antiviral", "booster", "medic"};
		array<string> food = {"/food/", "water", "canteen", "ration", "mre", "meat", "bread", "drink", "crackers", "canned", "tinned"};
		array<string> weapons = {"/weapons/", "magazine", "ammo", "rifle", "pistol", "grenade", "rounds", "gun"};
		if (HasAny(s, gear) && !HasAny(s, medkits))
			return TAB_GEAR;

		if (HasAny(s, medical) || HasAny(s, medkits))
			return TAB_MEDICAL;

		if (HasAny(s, food))
			return TAB_FOOD;

		if (HasAny(s, weapons))
			return TAB_WEAPONS;

		return TAB_GEAR;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasAny(string s, array<string> words)
	{
		foreach (string w : words)
		{
			if (s.Contains(w))
				return true;
		}

		return false;
	}
}
