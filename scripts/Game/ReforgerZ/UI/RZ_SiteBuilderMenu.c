//! The site builder's screen (docs/SITES.md, "The site builder"; UI/LAYOUTS.md, RZ_SiteBuilderMenu):
//! every site in a list with faction tabs and state filters, the picked one's details and role-spot
//! checklist beside it, and a bar of buttons for what can be done to it. Browsing (tabs, filters,
//! moving the pick) stays here; a button that changes something goes to the server as an action code
//! (World/HandBuilt/RZ_SiteBuilderScreen.c), which sends the screen back.
//!
//! Pad: up/down the list, LB/RB the faction, LT/RT the filter, A go there, X build, Y use on/off, R3
//! pin, Menu more (restore, delete), B close. While you build the picked site: A put a spot at your
//! feet, LT/RT the spot type, X save, Y stop building, R3 take up the nearest spot. Keys: arrows, Q/E,
//! Z/C, Enter, R, F, V, X, Esc. Mouse: tabs, filters and rows are clicked.
class RZ_SiteBuilderMenu : RZ_MenuBase
{
	protected static RZ_SiteBuilderMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static bool s_bFailed;			// the screen couldn't open here: the object menu from now on
	protected static int s_iTab;				// 0 all, then RZ_EFaction + 1
	protected static int s_iFilter;				// 0 all, 1 not built, 2 hand-built, 3 pinned
	protected static string s_sPicked;			// the picked site's id, kept for the session
	protected static bool s_bOpen;

	protected static const int ROWS = 10;		// Row0-Row9 in the layout
	protected static const int SPOTS = 16;		// Spot0-Spot15
	protected static const int TABS = 5;
	protected static const int FILTERS = 4;

	protected int m_iParsed = -1;
	protected bool m_bDone;
	protected bool m_bMore;						// Menu pressed: the bar shows restore and delete
	protected Widget m_wLit;					// the button under the mouse, lit (OnMenuUpdate)
	protected bool m_bChoose;					// Edit on a site never built: from its layout, or empty?
	protected int m_iFirst;						// the list's first row shown
	protected ref array<string> m_aHead = {};
	protected ref array<ref array<string>> m_aSites = {};
	protected ref array<ref array<string>> m_aCheck = {};
	protected ref array<int> m_aShown = {};		// m_aSites indexes the list shows, in its order
	protected ref array<int> m_aItems = {};		// what each list line is: a site (its m_aShown index) or a faction's heading (-1 - faction)
	protected string m_sMessage;

	//------------------------------------------------------------------------------------------------
	//! The screen's payload from the server: opens or redraws it; X|reason closes it.
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

		s_sData = payload;
		s_iVersion++;
		if (s_Open || s_bFailed)
		{
			if (s_bFailed)
				ui.SendSiteAct("fallback", "");

			return;
		}

		if (RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_SiteBuilderMenu))
			return;

		// No preset or layout here: the object menu instead, from now on
		s_bFailed = true;
		ui.SendSiteAct("fallback", "");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnTabLeft);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnTabRight);
		input.AddActionListener("MenuNavLeft_NoScroll", EActionTrigger.DOWN, OnTriggerLeft);
		input.AddActionListener("MenuNavRight_NoScroll", EActionTrigger.DOWN, OnTriggerRight);
		input.AddActionListener("MouseWheelUp", EActionTrigger.DOWN, OnWheelUp);
		input.AddActionListener("MouseWheelDown", EActionTrigger.DOWN, OnWheelDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnA);
		input.AddActionListener("MenuRefresh", EActionTrigger.DOWN, OnX);
		input.AddActionListener("MenuFavourite", EActionTrigger.DOWN, OnY);
		input.AddActionListener("MenuDelete", EActionTrigger.DOWN, OnR3);
		input.AddActionListener("MenuFilter", EActionTrigger.DOWN, OnMore);
		for (int t = 0; t < TABS; t++)
		{
			RZ_UIW.OnClick(m_wRoot, "Tab" + t.ToString(), m_Click);
			RZ_UIW.SetTag(RZ_UIW.Find(m_wRoot, "Tab" + t.ToString()), t);
		}

		for (int f = 0; f < FILTERS; f++)
		{
			RZ_UIW.OnClick(m_wRoot, "Filter" + f.ToString(), m_Click);
			RZ_UIW.SetTag(RZ_UIW.Find(m_wRoot, "Filter" + f.ToString()), f);
		}

		for (int r = 0; r < ROWS; r++)
		{
			RZ_UIW.OnClick(m_wRoot, "Row" + r.ToString(), m_Click);
		}

		RZ_UIW.OnClick(m_wRoot, "SpotPrev", m_Click);
		RZ_UIW.OnClick(m_wRoot, "SpotNext", m_Click);
		for (int t = 0; t < 4; t++)
		{
			RZ_UIW.OnClick(m_wRoot, "Tile" + t.ToString(), m_Click);
			RZ_UIW.SetTag(RZ_UIW.Find(m_wRoot, "Tile" + t.ToString()), t);
		}

		array<string> asks = {"AskSave", "AskDiscard", "AskStay", "ChooseLayout", "ChooseEmpty", "ChooseCancel"};
		foreach (string askName : asks)
		{
			RZ_UIW.OnClick(m_wRoot, askName, m_Click);
		}

		s_bOpen = true;
		RZ_SiteHud.SetHidden(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnTabLeft);
		input.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnTabRight);
		input.RemoveActionListener("MenuNavLeft_NoScroll", EActionTrigger.DOWN, OnTriggerLeft);
		input.RemoveActionListener("MenuNavRight_NoScroll", EActionTrigger.DOWN, OnTriggerRight);
		input.RemoveActionListener("MouseWheelUp", EActionTrigger.DOWN, OnWheelUp);
		input.RemoveActionListener("MouseWheelDown", EActionTrigger.DOWN, OnWheelDown);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnA);
		input.RemoveActionListener("MenuRefresh", EActionTrigger.DOWN, OnX);
		input.RemoveActionListener("MenuFavourite", EActionTrigger.DOWN, OnY);
		input.RemoveActionListener("MenuDelete", EActionTrigger.DOWN, OnR3);
		input.RemoveActionListener("MenuFilter", EActionTrigger.DOWN, OnMore);
		if (s_Open == this)
			s_Open = null;

		s_bOpen = false;
		RZ_SiteHud.SetHidden(false);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendSiteAct("closed", "");

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		int more = 0;
		if (m_bMore)
			more = 1;

		return (((s_iVersion % 10000) * 8 + s_iTab) * 8 + s_iFilter) * 2 + more + m_iFirst * 1000003 + s_sPicked.Hash() % 997 * 7919;
	}

	// ================================================================ The data
	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aSites.Clear();
		m_aCheck.Clear();
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
				m_aHead.Copy(f);
			else if (tag == "S")
				m_aSites.Insert(f);
			else if (tag == "C")
				m_aCheck.Insert(f);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The sites the list shows: the tab's faction, the filter's state, nearest first.
	protected void Filter()
	{
		m_aShown.Clear();
		array<int> dist = {};
		foreach (int i, array<string> site : m_aSites)
		{
			if (s_iTab > 0 && RZ_UIText.Int(site, 4) != s_iTab - 1)
				continue;

			int state = RZ_UIText.Int(site, 5);
			if (s_iFilter == 1 && state != 0)
				continue;

			if (s_iFilter == 2 && state == 0)
				continue;

			if (s_iFilter == 3 && RZ_UIText.Int(site, 9) != 1)
				continue;

			// The All tab groups by faction (each under its heading), every tab nearest first
			int d = RZ_UIText.Int(site, 6);
			if (s_iTab == 0)
				d += RZ_UIText.Int(site, 4) * 10000000;

			int at = 0;
			while (at < dist.Count() && dist[at] <= d)
			{
				at++;
			}

			dist.InsertAt(d, at);
			m_aShown.InsertAt(i, at);
		}

		m_aItems.Clear();
		int last = -1;
		foreach (int row, int idx : m_aShown)
		{
			int fac = RZ_UIText.Int(m_aSites[idx], 4);
			if (s_iTab == 0 && fac != last)
				m_aItems.Insert(-1 - fac);

			last = fac;
			m_aItems.Insert(row);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where the picked site is in the list, -1 when it isn't shown.
	protected int PickedRow()
	{
		foreach (int row, int idx : m_aShown)
		{
			if (RZ_UIText.Str(m_aSites[idx], 1) == s_sPicked)
				return row;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected array<string> Picked()
	{
		int row = PickedRow();
		if (row < 0)
			return null;

		return m_aSites[m_aShown[row]];
	}

	//------------------------------------------------------------------------------------------------
	//! The site being built by you: the head's building id.
	protected bool BuildingPicked()
	{
		string building = RZ_UIText.Str(m_aHead, 4);
		return !building.IsEmpty() && building == s_sPicked;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Asking()
	{
		return RZ_UIText.Int(m_aHead, 6) == 2;
	}

	// ================================================================ Moving
	//------------------------------------------------------------------------------------------------
	protected void Pick(int row)
	{
		if (m_aShown.IsEmpty())
			return;

		int count = m_aShown.Count();
		row = (row % count + count) % count;
		string id = RZ_UIText.Str(m_aSites[m_aShown[row]], 1);
		if (id == s_sPicked)
			return;

		s_sPicked = id;
		m_bMore = false;
		m_sMessage = "";
		// The checklist is the server's: ask for the picked site's
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendSiteAct("select", id);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		if (!Asking())
			Pick(PickedRow() - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		if (!Asking())
			Pick(PickedRow() + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabLeft(float value = 0, EActionTrigger reason = 0)
	{
		SetTab((s_iTab + TABS - 1) % TABS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabRight(float value = 0, EActionTrigger reason = 0)
	{
		SetTab((s_iTab + 1) % TABS);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetTab(int tab)
	{
		if (Asking())
			return;

		s_iTab = tab;
		m_iFirst = 0;
		Filter();
		if (PickedRow() < 0)
			Pick(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFilter(int filter)
	{
		s_iFilter = filter;
		m_iFirst = 0;
		Filter();
		if (PickedRow() < 0)
			Pick(0);
	}

	//------------------------------------------------------------------------------------------------
	//! LT: the previous filter, or while building the previous spot type.
	protected void OnTriggerLeft(float value = 0, EActionTrigger reason = 0)
	{
		if (Asking())
			return;

		if (BuildingPicked())
			Act("spot_back");
		else
			SetFilter((s_iFilter + FILTERS - 1) % FILTERS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTriggerRight(float value = 0, EActionTrigger reason = 0)
	{
		if (Asking())
			return;

		if (BuildingPicked())
			Act("spot_next");
		else
			SetFilter((s_iFilter + 1) % FILTERS);
	}

	//------------------------------------------------------------------------------------------------
	//! The mouse wheel: the list (or, while you build the picked site, the spot type).
	protected void OnWheelUp(float value = 0, EActionTrigger reason = 0)
	{
		if (Asking())
			return;

		if (BuildingPicked())
			Act("spot_back");
		else
			Pick(PickedRow() - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWheelDown(float value = 0, EActionTrigger reason = 0)
	{
		if (Asking())
			return;

		if (BuildingPicked())
			Act("spot_next");
		else
			Pick(PickedRow() + 1);
	}

	// ================================================================ The buttons
	//------------------------------------------------------------------------------------------------
	protected void Act(string act)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendSiteAct(act, s_sPicked);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnA(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bChoose)
		{
			m_bChoose = false;
			Act("build_auto");
			return;
		}

		string act = ButtonAct("A");
		if (!act.IsEmpty())
			Act(act);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnX(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bChoose)
		{
			m_bChoose = false;
			Act("build_empty");
			return;
		}

		string act = ButtonAct("X");
		if (act == "choose")
		{
			m_bChoose = true;
			m_iSeen = -1;
			return;
		}

		if (!act.IsEmpty())
			Act(act);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnY(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bChoose)
			return;

		string act = ButtonAct("Y");
		if (!act.IsEmpty())
			Act(act);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnR3(float value = 0, EActionTrigger reason = 0)
	{
		string act = ButtonAct("R3");
		if (!act.IsEmpty())
			Act(act);
	}

	//------------------------------------------------------------------------------------------------
	//! Menu: the bar's other buttons (restore, delete), and back.
	protected void OnMore(float value = 0, EActionTrigger reason = 0)
	{
		if (Asking() || (!m_bMore && !HasMore()))
			return;

		m_bMore = !m_bMore;
	}

	//------------------------------------------------------------------------------------------------
	//! B: out of the question (stay), out of the other buttons, else the screen closes.
	override protected void Back()
	{
		if (m_bChoose)
		{
			m_bChoose = false;
			m_iSeen = -1;
			return;
		}

		if (Asking())
		{
			Act("leave_stay");
			return;
		}

		if (m_bMore)
		{
			m_bMore = false;
			return;
		}

		Close();
	}

	//------------------------------------------------------------------------------------------------
	//! What a button does now ("" nothing), and its label (ButtonLabel).
	protected string ButtonAct(string button)
	{
		string label;
		return ButtonOf(button, label);
	}

	//------------------------------------------------------------------------------------------------
	protected string ButtonOf(string button, out string label)
	{
		label = "";
		if (Asking())
		{
			if (button == "A")
			{
				label = "Save, then go on";
				return "leave_save";
			}

			if (button == "X")
			{
				label = "Discard and go on";
				return "leave_discard";
			}

			return "";
		}

		array<string> site = Picked();
		if (!site)
			return "";

		bool saved = RZ_UIText.Int(site, 5) != 0;
		bool other = RZ_UIText.Int(site, 10) == 2;
		if (BuildingPicked())
		{
			switch (button)
			{
				case "A": label = "Put spot here"; return "spot_place";
				case "X":
				{
					label = "Save";
					if (RZ_UIText.Str(m_aHead, 8) == "save")
						label = "Save: press again";

					return "save";
				}
				case "Y": label = "Stop building"; return "stop";
				case "R3": label = "Remove nearest spot"; return "spot_remove";
			}

			return "";
		}

		if (m_bMore)
		{
			if (button == "A" && RZ_UIText.Int(site, 15) > 0 && !other)
			{
				label = "Restore";
				if (RZ_UIText.Str(m_aHead, 8) == "restore")
					label = "Restore: press again";

				return "restore";
			}

			if (button == "X" && saved && !other)
			{
				label = "Delete";
				if (RZ_UIText.Str(m_aHead, 8) == "delete")
					label = "Delete: press again, for good";

				return "delete";
			}

			return "";
		}

		switch (button)
		{
			case "A": label = "Teleport there"; return "go";
			case "X":
			{
				if (other)
					return "";

				if (saved)
				{
					label = "Edit the build";
					return "build_saved";
				}

				label = "Start building";
				return "choose";
			}
			case "Y":
			{
				if (saved)
				{
					label = "Use: hand-built";
					if (RZ_UIText.Int(site, 5) == 2)
						label = "Use: automated";

					return "use";
				}

				return "";
			}
			case "R3":
			{
				if (!saved || RZ_UIText.Int(site, 16) == 1)
					return "";

				label = "Pin here every season: off";
				if (RZ_UIText.Int(site, 9) == 1)
					label = "Pin here every season: on";

				return "pin";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A big button: the same as its pad button.
	protected void PressTile(int i)
	{
		switch (i)
		{
			case 0: OnA(); break;
			case 1: OnX(); break;
			case 2: OnY(); break;
			case 3: OnR3(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The base's hover look stays off this screen's buttons: OnMenuUpdate lights them itself.
	override protected bool OnHoverRow(Widget w, string name, int tag, bool on)
	{
		return !BgOf(name).IsEmpty() || name.StartsWith("Tab");
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame: the button under the mouse is lit and says in one line what it does (the
	//! enter/leave events don't reach a button through its own picture and text, so it's polled).
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		Widget lit = null;
		if (RZ_UIW.ByMouse())
			lit = ButtonAt(WidgetManager.GetWidgetUnderCursor());

		if (lit == m_wLit)
			return;

		m_wLit = lit;
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	//! The button `w` is, or is inside ("" none).
	protected Widget ButtonAt(Widget w)
	{
		Widget p = w;
		while (p && p != m_wRoot)
		{
			string name = p.GetName();
			if (!BgOf(name).IsEmpty() || (name.StartsWith("Tab") && name.Length() == 4))
			{
				if (p.IsVisible())
					return p;

				return null;
			}

			p = p.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! After a draw: the lit button's lighter fill, and its line in the note under the site.
	protected void DrawLit()
	{
		if (!m_wLit)
			return;

		string name = m_wLit.GetName();
		int tag = RZ_UIW.GetTag(m_wLit);
		if (name.StartsWith("Row"))
		{
			if (tag < 0)
				return;

			RZ_UIW.Show(m_wRoot, "RowBg" + name.Substring(3, name.Length() - 3), true);
		}

		string bg = BgOf(name);
		if (!bg.IsEmpty())
		{
			Color fill = Lin(70, 78, 86);
			if (name == "Tile0" || (name.StartsWith("Filter") && tag == s_iFilter))
				fill = Lin(250, 208, 120);

			RZ_UIW.Tint(m_wRoot, bg, fill);
		}
		else
		{
			RZ_UIW.Tint(m_wRoot, "TabText" + name.Substring(3, 1), Lin(255, 255, 255));
		}

		string line = Describe(name, tag);
		if (!line.IsEmpty())
		{
			RZ_UIW.SetText(m_wRoot, "DetailNote", line);
			RZ_UIW.Tint(m_wRoot, "DetailNote", Lin(236, 233, 225));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One sentence on what a button does, as things stand.
	protected string Describe(string name, int tag)
	{
		if (name.StartsWith("Tile"))
		{
			array<string> pads = {"A", "X", "Y", "R3"};
			string label;
			string act = ButtonOf(pads[Math.Clamp(tag, 0, 3)], label);
			switch (act)
			{
				case "go": return "Takes you to this site, ready to look it over.";
				case "build_saved": return "Opens the saved build so you can change it in Game Master.";
				case "choose": return "Starts building here, from its usual layout or from nothing.";
				case "use": return "Switches whether camp life uses your hand-built site or lays it out by itself.";
				case "pin": return "Keeps the camp on this site every season instead of letting it move.";
				case "spot_place": return "Puts the picked spot where you stand.";
				case "save": return "Saves the build as it is now (press twice).";
				case "stop": return "Stops building; you're asked about anything unsaved.";
				case "spot_remove": return "Takes up the spot nearest to you.";
				case "restore": return "Brings back the last backup of this site (press twice).";
				case "delete": return "Deletes the saved build for good (press twice).";
				case "leave_save": return "Saves your changes, then does what you asked.";
				case "leave_discard": return "Throws away your changes, then does what you asked.";
			}

			return "";
		}

		if (name.StartsWith("Tab"))
			return "Shows only this faction's sites (LB and RB too).";

		if (name.StartsWith("Filter"))
		{
			array<string> filters = {"Shows every site.", "Shows the sites nobody has built by hand yet.", "Shows the sites with a hand-built save.", "Shows the sites pinned for every season."};
			return filters[Math.Clamp(tag, 0, 3)];
		}

		if (name.StartsWith("Row"))
			return "Picks this site and shows its details.";

		switch (name)
		{
			case "AskSave": return "Saves your changes, then goes on.";
			case "AskDiscard": return "Throws away your changes, then goes on.";
			case "AskStay": return "Goes back to building; nothing is lost.";
			case "ChooseLayout": return "Puts out the site's usual layout first, for you to change.";
			case "ChooseEmpty": return "Starts with nothing placed, so you build it all by hand.";
			case "ChooseCancel": return "Goes back without building.";
			case "CloseButton": return "Closes this screen.";
			case "SpotPrev": return "The previous kind of spot to put down.";
			case "SpotNext": return "The next kind of spot to put down.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A button's fill: "Tile2" -> "TileBg2", "AskSave" -> "AskSaveBg" ("" none).
	protected static string BgOf(string name)
	{
		array<string> numbered = {"Tile", "Row", "Filter"};
		foreach (string n : numbered)
		{
			if (name.StartsWith(n) && name.Length() > n.Length())
				return n + "Bg" + name.Substring(n.Length(), name.Length() - n.Length());
		}

		array<string> named = {"AskSave", "AskDiscard", "AskStay", "ChooseLayout", "ChooseEmpty", "ChooseCancel", "CloseButton", "SpotPrev", "SpotNext"};
		if (named.Find(name) < 0)
			return "";

		if (name == "CloseButton")
			return "CloseBg";

		return name + "Bg";
	}

	// ================================================================ Mouse
	//------------------------------------------------------------------------------------------------
	override protected void OnClickOther(Widget w, string name)
	{
		int tag = RZ_UIW.GetTag(w);
		if (name.StartsWith("Tab"))
			SetTab(tag);
		else if (name.StartsWith("Filter") && !BuildingPicked())
			SetFilter(tag);
		else if (name.StartsWith("Row") && !Asking() && tag >= 0)
			Pick(tag);
		else if (name.StartsWith("Tile"))
			PressTile(tag);
		else if (name == "AskSave" && Asking())
			Act("leave_save");
		else if (name == "AskDiscard" && Asking())
			Act("leave_discard");
		else if (name == "AskStay" && Asking())
			Act("leave_stay");
		else if (name == "ChooseLayout")
			OnA();
		else if (name == "ChooseEmpty")
			OnX();
		else if (name == "ChooseCancel")
			Back();
		else if (name == "SpotPrev" && BuildingPicked())
			Act("spot_back");
		else if (name == "SpotNext" && BuildingPicked())
			Act("spot_next");
	}

	// ================================================================ Drawing
	//------------------------------------------------------------------------------------------------
	//! A colour as it should look (0-255, like a mockup's hex): the screen takes linear values.
	static Color Lin(int r, int g, int b)
	{
		return new Color(ToLinear(r), ToLinear(g), ToLinear(b), 1);
	}

	//------------------------------------------------------------------------------------------------
	protected static float ToLinear(int v)
	{
		float c = v / 255.0;
		if (c <= 0.04045)
			return c / 12.92;

		return Math.Pow((c + 0.055) / 1.055, 2.4);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		Filter();
		if (PickedRow() < 0 && !m_aShown.IsEmpty())
		{
			// The site being built first, else the nearest
			string building = RZ_UIText.Str(m_aHead, 4);
			s_sPicked = "";
			foreach (int row, int idx : m_aShown)
			{
				if (RZ_UIText.Str(m_aSites[idx], 1) == building)
					s_sPicked = building;
			}

			if (s_sPicked.IsEmpty())
				Pick(0);
		}

		RZ_UIW.SetText(m_wRoot, "Sub", string.Format("%1 · %2 sites · %3 hand-built", RZ_UIText.Str(m_aHead, 1), RZ_UIText.Int(m_aHead, 2), RZ_UIText.Int(m_aHead, 3)));
		DrawTabs();
		DrawList();
		DrawDetail();
		DrawBar();
		DrawAsk();
		DrawLit();
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawTabs()
	{
		array<string> names = {"All", "Haven", "Bastion", "Red Hand", "Halcyon"};
		for (int t = 0; t < TABS; t++)
		{
			int n = 0;
			foreach (array<string> site : m_aSites)
			{
				if (t == 0 || RZ_UIText.Int(site, 4) == t - 1)
					n++;
			}

			string idx = t.ToString();
			RZ_UIW.SetText(m_wRoot, "TabText" + idx, string.Format("%1 (%2)", names[t], n));
			RZ_UIW.Show(m_wRoot, "TabLine" + idx, t == s_iTab);
			Color c = Lin(154, 151, 143);
			if (t == s_iTab)
				c = Lin(255, 255, 255);

			RZ_UIW.Tint(m_wRoot, "TabText" + idx, c);
		}

		array<string> filters = {"All", "Not built", "Hand-built", "Pinned"};
		for (int f = 0; f < FILTERS; f++)
		{
			string fi = f.ToString();
			RZ_UIW.SetText(m_wRoot, "FilterText" + fi, filters[f]);
			Color bg = Lin(42, 45, 49);
			Color fg = Lin(207, 204, 196);
			if (f == s_iFilter)
			{
				bg = Lin(232, 178, 58);
				fg = Lin(38, 34, 26);
			}

			RZ_UIW.Tint(m_wRoot, "FilterBg" + fi, bg);
			RZ_UIW.Tint(m_wRoot, "FilterText" + fi, fg);
		}

		string sort = "LT RT filter · nearest first";
		if (BuildingPicked())
			sort = "nearest first";

		RZ_UIW.SetText(m_wRoot, "SortText", sort);
	}

	//------------------------------------------------------------------------------------------------
	//! The state chip: its words and colours.
	protected void StateLook(array<string> site, out string words, out Color bg, out Color fg)
	{
		int state = RZ_UIText.Int(site, 5);
		int building = RZ_UIText.Int(site, 10);
		words = "AUTOMATED";
		bg = Lin(68, 68, 65);
		fg = Lin(211, 209, 199);
		if (building == 1)
		{
			words = "BUILDING NOW";
			bg = Lin(12, 68, 124);
			fg = Lin(181, 212, 244);
		}
		else if (building == 2)
		{
			words = "SOMEONE BUILDING";
			bg = Lin(12, 68, 124);
			fg = Lin(181, 212, 244);
		}
		else if (state == 1)
		{
			words = "IN USE";
			bg = Lin(39, 80, 10);
			fg = Lin(192, 221, 151);
		}
		else if (state == 2)
		{
			words = "NOT IN USE";
			bg = Lin(99, 56, 6);
			fg = Lin(250, 199, 117);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawList()
	{
		array<string> headings = {"THE HAVEN", "TASK FORCE BASTION", "THE RED HAND", "HALCYON"};
		int picked = PickedRow();
		int line = m_aItems.Find(picked);
		if (line >= 0)
		{
			// Keep the picked site's heading in view above it where it can be
			int top = line;
			if (top > 0 && m_aItems[top - 1] < 0)
				top--;

			if (top < m_iFirst)
				m_iFirst = top;
			else if (line >= m_iFirst + ROWS)
				m_iFirst = line - ROWS + 1;
		}

		m_iFirst = Math.Clamp(m_iFirst, 0, Math.Max(0, m_aItems.Count() - ROWS));
		bool unsaved = RZ_UIText.Int(m_aHead, 5) == 1;
		for (int r = 0; r < ROWS; r++)
		{
			string idx = r.ToString();
			int at = m_iFirst + r;
			Widget row = RZ_UIW.Find(m_wRoot, "Row" + idx);
			bool there = at < m_aItems.Count();
			if (row)
				row.SetVisible(there);

			if (!there)
				continue;

			int item = m_aItems[at];
			bool heading = item < 0;
			RZ_UIW.SetTag(row, item);
			RZ_UIW.Show(m_wRoot, "RowHead" + idx, heading);
			RZ_UIW.Show(m_wRoot, "RowName" + idx, !heading);
			RZ_UIW.Show(m_wRoot, "RowSpots" + idx, !heading);
			RZ_UIW.Show(m_wRoot, "RowStateBg" + idx, !heading);
			RZ_UIW.Show(m_wRoot, "RowState" + idx, !heading);
			if (heading)
			{
				int fac = Math.Clamp(-1 - item, 0, 3);
				RZ_UIW.SetText(m_wRoot, "RowHead" + idx, headings[fac]);
				RZ_UIW.Show(m_wRoot, "RowBg" + idx, false);
				RZ_UIW.Show(m_wRoot, "RowMark" + idx, false);
				RZ_UIW.Show(m_wRoot, "RowDot" + idx, false);
				continue;
			}

			array<string> site = m_aSites[m_aShown[item]];
			bool on = item == picked;
			RZ_UIW.SetText(m_wRoot, "RowName" + idx, RZ_UIText.Clean(RZ_UIText.Str(site, 2), 34));
			int building = RZ_UIText.Int(site, 10);
			RZ_UIW.Show(m_wRoot, "RowDot" + idx, building == 1 && unsaved);
			// Spots: done of all, for a site that's built or being built
			string spots = "–";
			Color spotCol = Lin(143, 140, 133);
			if (RZ_UIText.Int(site, 5) != 0 || building == 1)
			{
				int have = RZ_UIText.Int(site, 11);
				int want = RZ_UIText.Int(site, 12);
				spots = string.Format("%1/%2", have, want);
				spotCol = Lin(240, 149, 149);
				if (have >= want)
					spotCol = Lin(151, 196, 89);
			}

			RZ_UIW.SetText(m_wRoot, "RowSpots" + idx, spots);
			RZ_UIW.Tint(m_wRoot, "RowSpots" + idx, spotCol);
			string words;
			Color bg;
			Color fg;
			StateLook(site, words, bg, fg);
			RZ_UIW.SetText(m_wRoot, "RowState" + idx, words);
			RZ_UIW.Tint(m_wRoot, "RowStateBg" + idx, bg);
			RZ_UIW.Tint(m_wRoot, "RowState" + idx, fg);
			RZ_UIW.Show(m_wRoot, "RowBg" + idx, on);
			RZ_UIW.Tint(m_wRoot, "RowBg" + idx, Lin(27, 30, 33));
			RZ_UIW.Show(m_wRoot, "RowMark" + idx, on);
			if (on && row)
				RZ_UIW.FocusWidget(row);
		}

		int below = m_aItems.Count() - m_iFirst - ROWS;
		string more = "";
		if (below > 0)
			more = string.Format("▼ %1 more", below);

		RZ_UIW.SetText(m_wRoot, "ListMore", more);
		RZ_UIW.Show(m_wRoot, "ListEmpty", m_aShown.IsEmpty());
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDetail()
	{
		array<string> site = Picked();
		RZ_UIW.Show(m_wRoot, "DetailPane", site != null);
		if (!site)
			return;

		array<string> factions = {"THE HAVEN", "TASK FORCE BASTION", "THE RED HAND", "HALCYON"};
		int fac = Math.Clamp(RZ_UIText.Int(site, 4), 0, 3);
		string kind = RZ_UIText.Str(site, 3);
		kind.ToUpper();
		RZ_UIW.SetText(m_wRoot, "DetailHead", factions[fac] + " · " + kind);
		RZ_UIW.SetText(m_wRoot, "DetailName", RZ_UIText.Clean(RZ_UIText.Str(site, 2), 44));
		string words;
		Color bg;
		Color fg;
		StateLook(site, words, bg, fg);
		int state = RZ_UIText.Int(site, 5);
		if (RZ_UIText.Int(site, 10) == 0)
		{
			if (state == 1)
				words = "HAND-BUILT, IN USE";
			else if (state == 2)
				words = "HAND-BUILT, NOT IN USE";
		}

		RZ_UIW.SetText(m_wRoot, "DetailState", words);
		RZ_UIW.Tint(m_wRoot, "DetailStateBg", bg);
		RZ_UIW.Tint(m_wRoot, "DetailState", fg);
		bool pinned = RZ_UIText.Int(site, 9) == 1;
		RZ_UIW.Show(m_wRoot, "DetailPinBg", pinned);
		RZ_UIW.Show(m_wRoot, "DetailPin", pinned);
		array<string> keys = {"Building", "Where", "Objects", "Last saved"};
		array<string> vals = {};
		string house = RZ_UIText.Str(site, 18);
		if (house.IsEmpty())
			house = "not known until it's laid out";

		vals.Insert(house);
		vals.Insert(string.Format("%1, %2 of you", RZ_UIText.Str(site, 8), RZ_UIText.Str(site, 7)));
		int objects = RZ_UIText.Int(site, 13);
		string objText = "none: camp life lays it out";
		if (state != 0 || RZ_UIText.Int(site, 10) == 1)
			objText = objects.ToString();

		if (RZ_UIText.Int(site, 10) == 1 && RZ_UIText.Int(m_aHead, 5) == 1)
			objText += " · unsaved changes";

		vals.Insert(objText);
		string when = RZ_UIText.Str(site, 14);
		if (when.IsEmpty())
			when = "never";

		vals.Insert(when);
		for (int k = 0; k < 4; k++)
		{
			RZ_UIW.SetText(m_wRoot, "Key" + k.ToString(), keys[k]);
			RZ_UIW.SetText(m_wRoot, "Val" + k.ToString(), vals[k]);
		}

		RZ_UIW.SetText(m_wRoot, "SpotsHead", string.Format("ROLE SPOTS · %1 OF %2", RZ_UIText.Int(site, 11), RZ_UIText.Int(site, 12)));
		for (int s = 0; s < SPOTS; s++)
		{
			string si = s.ToString();
			bool there = s < m_aCheck.Count();
			RZ_UIW.Show(m_wRoot, "Spot" + si, there);
			RZ_UIW.Show(m_wRoot, "SpotIcon" + si, there);
			if (!there)
				continue;

			array<string> c = m_aCheck[s];
			bool ok = RZ_UIText.Int(c, 2) == 1;
			string icon = "track_next";
			Color col = Lin(240, 149, 149);
			if (ok)
			{
				icon = "track_done";
				col = Lin(151, 196, 89);
			}
			else if (RZ_UIText.Int(c, 3) != 1)
			{
				// Nice to have: grey, not red
				col = Lin(154, 151, 143);
			}

			RZ_UIW.Show(m_wRoot, "SpotIcon" + si, true);
			RZ_UIW.Icon(m_wRoot, "SpotIcon" + si, icon);
			RZ_UIW.Tint(m_wRoot, "SpotIcon" + si, col);
			RZ_UIW.SetText(m_wRoot, "Spot" + si, RZ_UIText.Str(c, 1));
			RZ_UIW.Tint(m_wRoot, "Spot" + si, col);
		}

		string note = "";
		if (RZ_UIText.Int(site, 10) == 2)
			note = "Someone else is building this site now.";
		else if (BuildingPicked())
			note = "Place objects in Game Master inside the white ring.";
		else if (RZ_UIText.Int(site, 5) == 0 && RZ_UIText.Int(site, 17) <= 0)
			note = "Go there first to build from its layout: it's put out once you're near.";

		RZ_UIW.Show(m_wRoot, "SpotPicker", BuildingPicked());
		RZ_UIW.SetText(m_wRoot, "SpotPickerType", RZ_UIText.Str(m_aHead, 9));

		RZ_UIW.SetText(m_wRoot, "DetailNote", note);
		RZ_UIW.Tint(m_wRoot, "DetailNote", Lin(232, 178, 58));
	}

	//------------------------------------------------------------------------------------------------
	//! A button on the bar (vanilla's input button: its key or pad glyph and a label). A click on it
	//! comes back through OnPromptClick; its own key press already reaches the listeners.
	protected bool Prompt(string action, string label)
	{
		Widget parent = RZ_UIW.Find(m_wRoot, "Prompts");
		if (!parent)
			return false;

		Widget w = GetGame().GetWorkspace().CreateWidgets(RZ_UIW.PROMPT_LAYOUT, parent);
		if (!w)
			return false;

		SCR_InputButtonComponent btn = SCR_InputButtonComponent.FindComponent(w);
		if (!btn)
		{
			w.RemoveFromHierarchy();
			return false;
		}

		btn.SetAction(action);
		btn.SetLabel(label);
		btn.m_OnActivated.Insert(OnPromptClick);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A bar button clicked with the mouse (a key press or pad button fires it too: those are
	//! already handled by the action listeners, so only the mouse counts here).
	protected void OnPromptClick(SCR_InputButtonComponent btn, string action)
	{
		if (!RZ_UIW.ByMouse())
			return;

		switch (action)
		{
			case "MenuSelect": OnA(); break;
			case "MenuRefresh": OnX(); break;
			case "MenuFavourite": OnY(); break;
			case "MenuDelete": OnR3(); break;
			case "MenuFilter": OnMore(); break;
			case "MenuNavRight_NoScroll": OnTriggerRight(); break;
			case "MenuBack": Back(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Whether Menu has anything under it for the picked site (restore or delete).
	protected bool HasMore()
	{
		array<string> site = Picked();
		if (!site || BuildingPicked() || RZ_UIText.Int(site, 10) == 2)
			return false;

		return RZ_UIText.Int(site, 5) != 0 || RZ_UIText.Int(site, 15) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The bar of buttons: only those that do something now.
	protected void DrawBar()
	{
		Widget prompts = RZ_UIW.Find(m_wRoot, "Prompts");
		if (prompts)
			RZ_UIW.ClearChildren(prompts);

		array<string> buttons = {"A", "X", "Y", "R3"};
		array<string> actions = {"MenuSelect", "MenuRefresh", "MenuFavourite", "MenuDelete"};
		string fallback = "";
		bool shown = true;
		foreach (int i, string b : buttons)
		{
			string label;
			string act = ButtonOf(b, label);
			if (act.IsEmpty())
				continue;

			shown = Prompt(actions[i], label) && shown;
			fallback += b + " " + label + "    ";
		}

		if (BuildingPicked())
		{
			shown = Prompt("MenuNavRight_NoScroll", "Spot type") && shown;
			fallback += "LT RT spot type    ";
		}
		else if (m_bMore || HasMore())
		{
			string more = "More";
			if (m_bMore)
				more = "Back";

			shown = Prompt("MenuFilter", more) && shown;
			fallback += "MENU " + more + "    ";
		}

		string close = "Close";
		if (Asking())
			close = "Stay";
		else if (m_bMore)
			close = "Back";

		shown = Prompt("MenuBack", close) && shown;
		fallback += "B " + close;
		RZ_UIW.SetText(m_wRoot, "PromptsText", fallback);
		RZ_UIW.Show(m_wRoot, "PromptsText", !shown);
		string message = "";
		string armed = RZ_UIText.Str(m_aHead, 8);
		if (armed == "save")
			message = "Press Save again to replace the last save.";
		else if (armed == "restore")
			message = "Press Restore again to go back to the save before this one.";
		else if (armed == "delete")
			message = "Press Delete again to delete the build for good: the automated layout comes back.";

		Color tone = Lin(232, 178, 58);
		if (message.IsEmpty())
		{
			string said = RZ_UIText.Str(m_aHead, 10);
			if (!said.IsEmpty())
				m_sMessage = said;

			message = m_sMessage;
			tone = Lin(151, 196, 89);
		}

		RZ_UIW.SetText(m_wRoot, "Message", RZ_UIText.Clean(message, 140));
		RZ_UIW.Tint(m_wRoot, "Message", tone);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawAsk()
	{
		bool ask = Asking();
		RZ_UIW.Show(m_wRoot, "Ask", ask);
		if (ask)
			RZ_UIW.SetText(m_wRoot, "AskText", RZ_UIText.Str(m_aHead, 7));

		if (ask || !Picked())
			m_bChoose = false;

		RZ_UIW.Show(m_wRoot, "Choose", m_bChoose);
		if (m_bChoose)
			RZ_UIW.SetText(m_wRoot, "ChooseText", "From its layout: camp life's pieces stay, yours to move, delete and add to (go there first, they're put out once you're near). Empty: bare ground, you place everything.");

		DrawTiles();
	}

	//------------------------------------------------------------------------------------------------
	//! The big buttons in the site's pane: what A, X, Y and R3 do now, each with its button's name.
	protected void DrawTiles()
	{
		array<string> buttons = {"A", "X", "Y", "R3"};
		foreach (int i, string b : buttons)
		{
			string idx = i.ToString();
			string label;
			string act = ButtonOf(b, label);
			bool there = !act.IsEmpty() && !m_bChoose;
			RZ_UIW.Show(m_wRoot, "Tile" + idx, there);
			if (!there)
				continue;

			RZ_UIW.SetText(m_wRoot, "TileKey" + idx, b);
			RZ_UIW.SetText(m_wRoot, "TileText" + idx, label);
			Color fill = Lin(36, 40, 44);
			Color ink = Lin(236, 233, 225);
			if (i == 0)
			{
				// The first is the main one: amber
				fill = Lin(232, 178, 58);
				ink = Lin(33, 29, 20);
			}

			RZ_UIW.Tint(m_wRoot, "TileBg" + idx, fill);
			RZ_UIW.Tint(m_wRoot, "TileText" + idx, ink);
			RZ_UIW.Tint(m_wRoot, "TileKey" + idx, ink);
		}
	}
}
