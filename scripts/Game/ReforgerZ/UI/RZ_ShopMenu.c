//! The quartermaster's shop screen (docs/UI.md, "The shop screen"), as approved: categories as
//! tabs, the items with their Favor price and the standing they need, a picture of the selected
//! item (vanilla's item preview of its prefab, turnable) and a details panel with Buy. Opened by
//! the server when a player picks Trade (Factions/RZ_ShopScreen.c); every purchase goes back to
//! the server, which runs the quartermaster's own Buy action code.
//!
//! Keys: Q/E switch tabs, up/down pick an item, left/right turn the picture, Enter buys, Esc
//! closes. Pad: LB/RB, D-pad or left stick, the right stick turns the picture, A buys, B closes.
//! Mouse: click a tab, an item, the picture (it turns) or Buy, and the close button.

//------------------------------------------------------------------------------------------------
class RZ_ShopMenu : RZ_MenuBase
{
	protected static RZ_ShopMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static bool s_bFailed;			// the menu couldn't open here: the actions come back
	protected static float s_fFallbackUntil;	// the server couldn't open one: the actions for a minute

	protected static const float TURN_STEP = 30;
	protected static const float TURN_SPEED = 150;	// degrees a second at full stick

	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected bool m_bDone;
	protected int m_iFaction;
	protected int m_iFavor;
	protected int m_iTab = -1;
	protected int m_iSelected;
	protected int m_iLocal;					// bumps when something only this client changed (a local status)
	protected string m_sLocalStatus;
	protected ref array<string> m_aHead = {};
	protected ref array<ref array<string>> m_aItems = {};
	protected ref array<int> m_aTabRows = {};	// the items on the open tab, as indexes into m_aItems
	protected ref array<Widget> m_aRows = {};
	protected ItemPreviewWidget m_wPreview;
	protected ResourceName m_sPreviewed;

	//------------------------------------------------------------------------------------------------
	//! The shop screen is used on this machine: UI > Shop item layout set (the server checks the
	//! same), it hasn't failed to open here, and the server hasn't just refused one.
	static bool Available()
	{
		return !s_bFailed && RZ_UIW.Now() >= s_fFallbackUntil && !RZ_UISettings.Get().m_sShopItemLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! A quartermaster's Buy, Requisition and service actions hide while the screen is used.
	static bool HidesActionsOn(IEntity owner)
	{
		return owner && Available() && owner.FindComponent(RZ_QuartermasterComponent) != null;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A shop payload (RZ_PlayerUIComponent): opens or redraws the screen; X|reason closes it; F|why:
	//! the server couldn't open one, so the actions show for a minute.
	static void OnPayload(RZ_PlayerUIComponent ui, string payload)
	{
		if (payload.StartsWith("X" + RZ_UIText.SEP))
		{
			if (s_Open)
			{
				s_Open.m_bDone = true;
				s_Open.Close();
			}

			return;
		}

		if (payload.StartsWith("F" + RZ_UIText.SEP))
		{
			s_fFallbackUntil = RZ_UIW.Now() + 60;
			RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
			if (gm)
				gm.ShowNotice("Trade", "They're busy. Their wares are in the action menu for now.", RZ_ENoticeTag.AUTO);

			return;
		}

		s_sData = payload;
		s_iVersion++;
		if (s_Open || RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_ShopMenu))
			return;

		// No preset or layout here: the actions come back
		s_bFailed = true;
		ui.SendScreenClosed(RZ_ScreenGuard.KIND_SHOP);
		RZ_GameModeComponent gmFail = RZ_GameModeComponent.GetInstance();
		if (gmFail)
			gmFail.ShowNotice("Trade", "Their wares are in the action menu.", RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuLeft", EActionTrigger.DOWN, OnTurnLeft);
		input.AddActionListener("MenuRight", EActionTrigger.DOWN, OnTurnRight);
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnTabLeft);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnTabRight);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		RZ_UIW.OnClick(m_wRoot, "BuyButton", m_Click);
		RZ_UIW.OnClick(m_wRoot, "PreviewButton", m_Click);
		for (int t = 0; t < RZ_ShopScreen.TABS; t++)
		{
			RZ_UIW.OnClick(m_wRoot, "Tab" + t.ToString(), m_Click);
		}

		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "Category");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Buy") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "RZ_TurnPreview", "Turn") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		MakePreview();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnTurnLeft);
		input.RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnTurnRight);
		input.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnTabLeft);
		input.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnTabRight);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		if (s_Open == this)
			s_Open = null;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendScreenClosed(RZ_ScreenGuard.KIND_SHOP);

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! The right stick turns the picture (RZ_TurnPreview), every frame while it's held.
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		float stick = GetGame().GetInputManager().GetActionValue("RZ_TurnPreview");
		if (Math.AbsFloat(stick) > 0.2)
			Turn(stick * tDelta * TURN_SPEED);
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		return s_iVersion * 100000 + m_iLocal * 1000 + m_iTab * 100 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Max(0, m_iSelected - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Min(Math.Max(0, m_aTabRows.Count() - 1), m_iSelected + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTurnLeft(float value = 0, EActionTrigger reason = 0)
	{
		Turn(-TURN_STEP);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTurnRight(float value = 0, EActionTrigger reason = 0)
	{
		Turn(TURN_STEP);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabLeft(float value = 0, EActionTrigger reason = 0)
	{
		StepTab(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabRight(float value = 0, EActionTrigger reason = 0)
	{
		StepTab(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		TryBuy();
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: a tab opens it, an item selects it, the picture turns, Buy buys.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "BuyButton")
		{
			TryBuy();
			return;
		}

		if (name == "PreviewButton")
		{
			Turn(TURN_STEP * 1.5);
			return;
		}

		if (name == "ItemRow")
		{
			m_iSelected = RZ_UIW.GetTag(w);
			return;
		}

		for (int t = 0; t < RZ_ShopScreen.TABS; t++)
		{
			if (name == "Tab" + t.ToString() && TabCount(t) > 0)
			{
				SetTab(t);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The next tab with something on it, `dir` 1 or -1.
	protected void StepTab(int dir)
	{
		for (int i = 1; i <= RZ_ShopScreen.TABS; i++)
		{
			int t = (m_iTab + dir * i + RZ_ShopScreen.TABS * 2) % RZ_ShopScreen.TABS;
			if (TabCount(t) > 0)
			{
				SetTab(t);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetTab(int tab)
	{
		if (tab == m_iTab)
			return;

		m_iTab = tab;
		m_iSelected = 0;
		ScrollLayoutWidget sc = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "ItemScroll"));
		if (sc)
			sc.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected int TabCount(int tab)
	{
		int n = 0;
		foreach (array<string> f : m_aItems)
		{
			if (RZ_UIText.Int(f, 2) == tab)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The selected item's record, or null.
	protected array<string> Selected()
	{
		if (m_iSelected < 0 || m_iSelected >= m_aTabRows.Count())
			return null;

		return m_aItems[m_aTabRows[m_iSelected]];
	}

	//------------------------------------------------------------------------------------------------
	//! Buy the selected item if it's for sale and they can afford it (the server checks it all again).
	protected void TryBuy()
	{
		array<string> f = Selected();
		if (!f || RZ_UIText.Str(f, 8).IsEmpty() || RZ_UIText.Int(f, 5) != RZ_FactionShopAction.STATE_OPEN)
			return;

		int price = RZ_UIText.Int(f, 4);
		if (price > m_iFavor)
		{
			m_sLocalStatus = string.Format("%1 costs %2 Favor here. You have %3.", RZ_UIText.Str(f, 3), price, m_iFavor);
			m_iLocal++;
			return;
		}

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendShopBuy(m_iSerial, RZ_UIText.Int(f, 1));
	}

	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aItems.Clear();
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "S")
			{
				m_aHead.Copy(f);
				m_iSerial = RZ_UIText.Int(f, 1);
				m_iFaction = RZ_UIText.Int(f, 2);
				m_iFavor = RZ_UIText.Int(f, 6);
			}
			else if (tag == "I")
			{
				m_aItems.Insert(f);
			}
		}

		// A new screen opens on the first tab with something on it; a purchase keeps the place
		m_sLocalStatus = "";
		if (m_iTab < 0 || TabCount(m_iTab) == 0)
		{
			m_iTab = -1;
			StepTab(1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! S|serial|faction|title|sub|standing|favor|status, then I|index|tab|name|price|state|need|prefab|button.
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		Color accent = RZ_UIColor.Faction(m_iFaction);
		RZ_UIW.Tint(m_wRoot, "ShopAccent", accent);
		RZ_UIW.Tint(m_wRoot, "HeadRule", accent);
		RZ_UIW.SetText(m_wRoot, "ShopTitle", RZ_UIText.Str(m_aHead, 3));
		RZ_UIW.SetText(m_wRoot, "ShopSub", RZ_UIText.Str(m_aHead, 4));
		RZ_UIW.SetText(m_wRoot, "ShopStanding", RZ_UIText.Str(m_aHead, 5));
		RZ_UIW.SetText(m_wRoot, "ShopFavor", string.Format("%1 Favor", m_iFavor));
		RZ_UIW.Tint(m_wRoot, "ShopFavor", accent);
		string status = m_sLocalStatus;
		if (status.IsEmpty())
			status = RZ_UIText.Str(m_aHead, 7);

		RZ_UIW.SetText(m_wRoot, "ShopStatus", status);
		RZ_UIW.Show(m_wRoot, "ShopStatus", !status.IsEmpty());
		DrawTabs(accent);
		DrawItems(accent);
		DrawDetails(accent);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawTabs(Color accent)
	{
		for (int t = 0; t < RZ_ShopScreen.TABS; t++)
		{
			string n = t.ToString();
			bool on = t == m_iTab;
			RZ_UIW.Show(m_wRoot, "Tab" + n, TabCount(t) > 0);
			Color bg = RZ_UIW.Rgb(26, 29, 32);
			Color fg = RZ_UIColor.Dim();
			if (on)
			{
				bg = accent;
				fg = RZ_UIW.Rgb(21, 23, 26);
			}

			RZ_UIW.Tint(m_wRoot, "Tab" + n + "Bg", bg);
			RZ_UIW.Tint(m_wRoot, "Tab" + n + "Text", fg);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawItems(Color accent)
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Items");
		RZ_UIW.ClearChildren(list);
		m_aRows.Clear();
		m_aTabRows.Clear();
		foreach (int i, array<string> f : m_aItems)
		{
			if (RZ_UIText.Int(f, 2) == m_iTab)
				m_aTabRows.Insert(i);
		}

		m_iSelected = Math.Clamp(m_iSelected, 0, Math.Max(0, m_aTabRows.Count() - 1));
		RZ_UIW.Show(m_wRoot, "ItemsEmpty", m_aTabRows.IsEmpty());
		ResourceName rowLayout = RZ_UISettings.Get().m_sShopItemLayout;
		foreach (int pos, int idx : m_aTabRows)
		{
			array<string> item = m_aItems[idx];
			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				return;

			bool on = pos == m_iSelected;
			int state = RZ_UIText.Int(item, 5);
			int price = RZ_UIText.Int(item, 4);
			bool open = state == RZ_FactionShopAction.STATE_OPEN;
			string priceText = "";
			if (price >= 0)
				priceText = string.Format("%1 Favor", price);

			string need = "";
			if (!open)
				need = RZ_UIText.Str(item, 6);

			RZ_UIW.SetText(row, "ItemName", RZ_UIText.Str(item, 3));
			RZ_UIW.SetText(row, "ItemPrice", priceText);
			RZ_UIW.SetText(row, "ItemNeed", need);
			RZ_UIW.Show(row, "ItemNeed", !need.IsEmpty());
			RZ_UIW.Show(row, "ItemLock", state == RZ_FactionShopAction.STATE_LOCKED);
			RZ_UIW.Icon(row, "ItemLock", "status_door_locked");
			RZ_UIW.Show(row, "ItemBg", on);
			RZ_UIW.Show(row, "ItemBar", on);
			RZ_UIW.Tint(row, "ItemBar", accent);
			Color priceColour = accent;
			Color fg = RZ_UIColor.Bone();
			if (!open)
			{
				fg = RZ_UIW.Rgb(110, 106, 96);
				priceColour = fg;
			}
			else if (on)
			{
				fg = RZ_UIW.Rgb(244, 240, 230);
			}

			RZ_UIW.Tint(row, "ItemName", fg);
			RZ_UIW.Tint(row, "ItemPrice", priceColour);
			RZ_UIW.SetTag(row, pos);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRows.Insert(row);
		}

		if (m_iSelected < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[m_iSelected]);

		// Keep the selected row in view on a pad
		ScrollLayoutWidget sc = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "ItemScroll"));
		if (sc && m_aRows.Count() > 1)
			sc.SetSliderPos(0, m_iSelected / (m_aRows.Count() - 1.0));
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDetails(Color accent)
	{
		array<string> f = Selected();
		RZ_UIW.Show(m_wRoot, "Details", f != null);
		if (!f)
		{
			ShowPreview("");
			return;
		}

		int state = RZ_UIText.Int(f, 5);
		int price = RZ_UIText.Int(f, 4);
		bool open = state == RZ_FactionShopAction.STATE_OPEN;
		ResourceName prefab = RZ_UIText.Str(f, 7);
		string button = RZ_UIText.Str(f, 8);
		RZ_UIW.SetText(m_wRoot, "DetailName", RZ_UIText.Str(f, 3));
		RZ_UIW.SetText(m_wRoot, "DetailDesc", Describe(prefab));
		string priceText = "";
		if (price >= 0)
			priceText = string.Format("%1 Favor", price);

		RZ_UIW.SetText(m_wRoot, "DetailPrice", priceText);
		RZ_UIW.Tint(m_wRoot, "DetailPrice", accent);
		RZ_UIW.SetText(m_wRoot, "DetailHave", string.Format("you have %1", m_iFavor));
		RZ_UIW.Show(m_wRoot, "DetailHave", price >= 0);
		RZ_UIW.SetText(m_wRoot, "DetailNeed", RZ_UIText.Str(f, 6));
		RZ_UIW.Show(m_wRoot, "DetailNeed", !RZ_UIText.Str(f, 6).IsEmpty());
		RZ_UIW.Show(m_wRoot, "DetailLocked", !open);
		bool canBuy = open && !button.IsEmpty();
		RZ_UIW.Show(m_wRoot, "BuyButton", canBuy);
		string word = button;
		word.ToUpper();
		RZ_UIW.SetText(m_wRoot, "BuyText", word);
		bool dear = price > m_iFavor;
		Color buyBg = accent;
		if (dear)
			buyBg = RZ_UIW.Rgb(27, 31, 34);

		RZ_UIW.Tint(m_wRoot, "BuyBg", buyBg);
		RZ_UIW.Show(m_wRoot, "PreviewDim", !open);
		RZ_UIW.Show(m_wRoot, "PreviewLock", state == RZ_FactionShopAction.STATE_LOCKED);
		RZ_UIW.Icon(m_wRoot, "PreviewLock", "status_door_locked");
		ShowPreview(prefab);
	}

	// ================================================================ The picture
	//------------------------------------------------------------------------------------------------
	//! Vanilla's item preview (as the inventory draws items), made at run time inside PreviewSlot.
	protected void MakePreview()
	{
		Widget slot = RZ_UIW.Find(m_wRoot, "PreviewSlot");
		if (!slot)
			return;

		Widget w = GetGame().GetWorkspace().CreateWidget(GameWidgetType.ItemPreviewWidgetTypeID, WidgetFlags.VISIBLE, Color.FromRGBA(255, 255, 255, 255), 0, slot);
		m_wPreview = ItemPreviewWidget.Cast(w);
		if (!m_wPreview)
			return;

		FrameSlot.SetAnchorMin(m_wPreview, 0, 0);
		FrameSlot.SetAnchorMax(m_wPreview, 1, 1);
		FrameSlot.SetOffsets(m_wPreview, 0, 0, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected ItemPreviewManagerEntity PreviewManager()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return null;

		return world.GetItemPreviewManager();
	}

	//------------------------------------------------------------------------------------------------
	//! The item component of the prefab's preview entity (the preview world's copy), or null.
	//! VERIFY: ResolvePreviewEntityForPrefab gives a copy carrying the prefab's InventoryItemComponent,
	//! with its UI info, weight and preview attributes.
	protected InventoryItemComponent PreviewItem(ResourceName prefab)
	{
		ItemPreviewManagerEntity manager = PreviewManager();
		if (!manager || prefab.IsEmpty())
			return null;

		IEntity e = manager.ResolvePreviewEntityForPrefab(prefab);
		if (!e)
			return null;

		return InventoryItemComponent.Cast(e.FindComponent(InventoryItemComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowPreview(ResourceName prefab)
	{
		if (!m_wPreview)
			return;

		m_wPreview.SetVisible(!prefab.IsEmpty());
		if (prefab.IsEmpty() || prefab == m_sPreviewed)
			return;

		ItemPreviewManagerEntity manager = PreviewManager();
		if (!manager)
			return;

		m_sPreviewed = prefab;
		PreviewRenderAttributes attrs = Attributes(prefab);
		if (attrs)
			attrs.ResetDeltaRotation();

		manager.SetPreviewItemFromPrefab(m_wPreview, prefab, attrs, true);
	}

	//------------------------------------------------------------------------------------------------
	protected PreviewRenderAttributes Attributes(ResourceName prefab)
	{
		InventoryItemComponent item = PreviewItem(prefab);
		if (!item)
			return null;

		return PreviewRenderAttributes.Cast(item.FindAttribute(PreviewRenderAttributes));
	}

	//------------------------------------------------------------------------------------------------
	//! Turns the picture about its up axis, `degrees` (the stick, left/right or a click).
	//! VERIFY: RotateItemCamera on the preview copy's attributes turns the item in the widget, as the
	//! inventory's character preview does.
	protected void Turn(float degrees)
	{
		ItemPreviewManagerEntity manager = PreviewManager();
		if (!m_wPreview || !manager || m_sPreviewed.IsEmpty())
			return;

		PreviewRenderAttributes attrs = Attributes(m_sPreviewed);
		if (!attrs)
			return;

		attrs.RotateItemCamera(Vector(0, degrees, 0), "-30 -180 0", "30 180 0");
		manager.SetPreviewItemFromPrefab(m_wPreview, m_sPreviewed, attrs, true);
	}

	//------------------------------------------------------------------------------------------------
	//! The item's own description and its weight, from its prefab ("" when it has neither).
	protected string Describe(ResourceName prefab)
	{
		InventoryItemComponent item = PreviewItem(prefab);
		if (!item)
			return "";

		string text = "";
		UIInfo info = item.GetUIInfo();
		if (info)
			text = info.GetDescription();

		int tenths = RZ_Util.RoundToInt(item.GetTotalWeight() * 10);
		if (tenths > 0)
		{
			if (!text.IsEmpty())
				text += "\n";

			text += string.Format("%1.%2 kg", tenths / 10, tenths % 10);
		}

		return text;
	}
}
