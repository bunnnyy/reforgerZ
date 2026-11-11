//! The notebook (docs/UI.md, "The notebook"): one menu with six tabs, Standing, Journal, Radio,
//! Map key, Hall of fame and The season. The menu (RZ_NotebookMenu, layout RZ_NotebookMenu.layout) holds the
//! shared frame, the tabs with their unread counts and an empty `Page` frame; each tab is a page
//! (RZ_NotebookPage) whose layout (UI > Notebook pages) is created into `Page` when the tab is
//! shown, so switching tabs swaps the page instead of closing one menu and opening another.
//!
//! Opened with N (RZ_OpenNotebook), the pause menu's Notebook button (RZ_PauseNotebook.c) or the
//! notebook item (RZ_NotebookItem.c). It opens on the tab last used this session (the first time:
//! Journal). LB/RB or Q/E switch tabs, up/down move in the list, A or Enter selects, X or F cycles
//! the Radio filter, B or Esc closes. It only reads; nothing here changes the game.

//------------------------------------------------------------------------------------------------
//! The notebook's tabs, in order.
enum RZ_ENotebookTab
{
	STANDING,
	JOURNAL,
	RADIO,
	MAP_KEY,
	HALL,
	SEASON
}

//------------------------------------------------------------------------------------------------
//! Opening the notebook (client).
class RZ_Notebook
{
	static const int TABS = 6;

	protected static int s_iLastTab = RZ_ENotebookTab.JOURNAL;
	protected static int s_iAskedTab = -1;
	protected static bool s_bOpen;
	protected static IEntity s_ItemHolder;	// opened from the notebook item: put it away on close

	//------------------------------------------------------------------------------------------------
	//! tab: RZ_ENotebookTab, or -1 for the tab last used. False if the menu couldn't open (no preset or layout).
	static bool Open(int tab = -1)
	{
		if (s_bOpen)
			return true;

		s_iAskedTab = tab;
		return RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_NotebookMenu) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! From the notebook item, taken into the hand by this machine's character.
	static void OpenFromItem(IEntity holder)
	{
		if (s_bOpen)
			return;

		if (Open(-1))
			s_ItemHolder = holder;
		else
			PutAway(holder);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOpen()
	{
		return s_bOpen;
	}

	//------------------------------------------------------------------------------------------------
	//! The menu: which tab to start on.
	static int TakeTab()
	{
		int tab = s_iLastTab;
		if (s_iAskedTab >= 0 && s_iAskedTab < TABS)
			tab = s_iAskedTab;

		s_iAskedTab = -1;
		return tab;
	}

	//------------------------------------------------------------------------------------------------
	static void OnOpened()
	{
		s_bOpen = true;
	}

	//------------------------------------------------------------------------------------------------
	static void OnClosed(int lastTab)
	{
		s_bOpen = false;
		s_iLastTab = lastTab;
		IEntity holder = s_ItemHolder;
		s_ItemHolder = null;
		if (holder)
			PutAway(holder);
	}

	//------------------------------------------------------------------------------------------------
	//! The notebook item goes back from the hand. VERIFY: RemoveHeldGadget on the owning client
	//! puts a held gadget away (vanilla's gadget input does the same); if not, it stays in the hand.
	protected static void PutAway(IEntity holder)
	{
		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(holder);
		if (gadgets)
			gadgets.RemoveHeldGadget();
	}

	//------------------------------------------------------------------------------------------------
	static string TabName(int tab)
	{
		switch (tab)
		{
			case RZ_ENotebookTab.STANDING: return "STANDING";
			case RZ_ENotebookTab.JOURNAL: return "JOURNAL";
			case RZ_ENotebookTab.RADIO: return "RADIO";
			case RZ_ENotebookTab.MAP_KEY: return "MAP KEY";
			case RZ_ENotebookTab.HALL: return "HALL OF FAME";
			case RZ_ENotebookTab.SEASON: return "THE SEASON";
		}

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! The notebook menu: the frame, the tabs, and the page shown in `Page`.
class RZ_NotebookMenu : RZ_MenuBase
{
	protected int m_iTab = -1;
	protected ref RZ_NotebookPage m_Page;
	protected Widget m_wPage;

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		RZ_Notebook.OnOpened();
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnTabLeft);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnTabRight);
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.AddActionListener("RZ_RadioFilter", EActionTrigger.DOWN, OnFilter);
		for (int t = 0; t < RZ_Notebook.TABS; t++)
		{
			RZ_UIW.OnClick(m_wRoot, "Tab" + t.ToString(), m_Click);
		}

		// Footer prompts by device (vanilla input buttons); the layout's static line otherwise
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabLeft", "");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "Tabs") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "RZ_RadioFilter", "Radio filter") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);

		RZ_HudDisplay.ClearNotebookCount();
		ShowTab(RZ_Notebook.TakeTab());
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnTabLeft);
		input.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnTabRight);
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.RemoveActionListener("RZ_RadioFilter", EActionTrigger.DOWN, OnFilter);
		if (m_Page)
			m_Page.Close();

		RZ_Notebook.OnClosed(Math.Max(0, m_iTab));
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! Swap the page: the old page's widgets go, the new page's layout is created into `Page`.
	protected void ShowTab(int tab)
	{
		int idx = tab;
		if (idx < 0)
			idx = RZ_Notebook.TABS - 1;
		else if (idx >= RZ_Notebook.TABS)
			idx = 0;

		if (m_Page)
			m_Page.Close();

		RZ_UIW.Remove(m_wPage);
		m_wPage = null;
		m_iTab = idx;
		m_Page = MakePage(idx);
		ResourceName layout = m_Page.Layout(RZ_UISettings.Get());
		m_wPage = RZ_UIW.Create(layout, RZ_UIW.Find(m_wRoot, "Page"));
		RZ_UIW.Show(m_wRoot, "PageEmpty", m_wPage == null);
		RZ_UIW.SetText(m_wRoot, "Heading", RZ_Notebook.TabName(idx));
		RZ_UIW.SetText(m_wRoot, "Header", "");
		DrawTabs();
		if (m_wPage)
			m_Page.Open(m_wRoot, m_Click);

		// Pad focus: the page's first row if it has one, else the open tab (VERIFY: focus after a
		// page swap; it's set again here every time)
		if (!m_wPage || !m_Page.FocusFirst())
			RZ_UIW.Focus(m_wRoot, "Tab" + idx.ToString());

		m_iSeen = -1;
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_NotebookPage MakePage(int tab)
	{
		switch (tab)
		{
			case RZ_ENotebookTab.STANDING: return new RZ_StandingPage();
			case RZ_ENotebookTab.RADIO: return new RZ_RadioPage();
			case RZ_ENotebookTab.MAP_KEY: return new RZ_MapKeyPage();
			case RZ_ENotebookTab.HALL: return new RZ_HallPage();
			case RZ_ENotebookTab.SEASON: return new RZ_SeasonPage();
		}

		return new RZ_JournalPage();
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		int v = 0;
		if (m_Page && m_wPage)
			v = m_Page.DataVersion();

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			v = v * 7 + ui.GetUnreadJournal() * 131 + ui.GetUnreadRadio() * 977;

		return v;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Redraw()
	{
		if (m_Page && m_wPage)
			m_Page.Redraw();

		DrawTabs();
	}

	//------------------------------------------------------------------------------------------------
	//! The open tab bone with dark text; the others dark with a border; a count on a tab with
	//! something new since you last looked (Journal: mission notices, Radio: logged notices).
	protected void DrawTabs()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && m_iTab == RZ_ENotebookTab.JOURNAL)
			ui.MarkJournalSeen();
		else if (ui && m_iTab == RZ_ENotebookTab.RADIO)
			ui.MarkRadioSeen();

		for (int t = 0; t < RZ_Notebook.TABS; t++)
		{
			string n = t.ToString();
			bool on = t == m_iTab;
			Color bg = RZ_UIW.Rgb(26, 29, 32);
			Color fg = RZ_UIColor.Dim();
			if (on)
			{
				bg = RZ_UIColor.Bone();
				fg = RZ_UIW.Rgb(20, 20, 20);
			}

			RZ_UIW.Tint(m_wRoot, "Tab" + n + "Bg", bg);
			RZ_UIW.Tint(m_wRoot, "Tab" + n + "Text", fg);
			RZ_UIW.Show(m_wRoot, "Tab" + n + "Line", !on);

			int unread = 0;
			if (ui && t == RZ_ENotebookTab.JOURNAL)
				unread = ui.GetUnreadJournal();
			else if (ui && t == RZ_ENotebookTab.RADIO)
				unread = ui.GetUnreadRadio();

			RZ_UIW.Show(m_wRoot, "TabBadge" + n, unread > 0 && !on);
			RZ_UIW.SetText(m_wRoot, "TabBadge" + n, unread.ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnWidgetClick(Widget w)
	{
		string name = w.GetName();
		for (int t = 0; t < RZ_Notebook.TABS; t++)
		{
			if (name == "Tab" + t.ToString())
			{
				if (t != m_iTab)
					ShowTab(t);

				return;
			}
		}

		super.OnWidgetClick(w);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClickOther(Widget w, string name)
	{
		if (m_Page && m_wPage)
			m_Page.OnClick(w, name);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabLeft(float value = 0, EActionTrigger reason = 0)
	{
		ShowTab(m_iTab - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabRight(float value = 0, EActionTrigger reason = 0)
	{
		ShowTab(m_iTab + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		if (m_Page && m_wPage)
			m_Page.Move(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		if (m_Page && m_wPage)
			m_Page.Move(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		if (m_Page && m_wPage)
			m_Page.Select();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFilter(float value = 0, EActionTrigger reason = 0)
	{
		if (m_Page && m_wPage)
			m_Page.Filter();
	}
}

//------------------------------------------------------------------------------------------------
//! One tab of the notebook. It draws with the menu's root, so names in the frame (Header) and in
//! the page's own layout are found the same way; their drawing code is the old menus', unchanged.
class RZ_NotebookPage
{
	protected Widget m_wRoot;
	protected RZ_MenuClick m_Click;

	//------------------------------------------------------------------------------------------------
	void Open(Widget root, RZ_MenuClick click)
	{
		m_wRoot = root;
		m_Click = click;
		OnOpened();
	}

	//------------------------------------------------------------------------------------------------
	void Close()
	{
	}

	//------------------------------------------------------------------------------------------------
	ResourceName Layout(RZ_UISettings settings)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOpened()
	{
	}

	//------------------------------------------------------------------------------------------------
	int DataVersion()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	void Redraw()
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Gives its first row the focus; false if it has none.
	bool FocusFirst()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void OnClick(Widget w, string name)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Up (-1) or down (1): the D-pad, the left stick, the arrows. By default it scrolls the page.
	void Move(int dir)
	{
	}

	//------------------------------------------------------------------------------------------------
	void Select()
	{
	}

	//------------------------------------------------------------------------------------------------
	void Filter()
	{
	}
}

//------------------------------------------------------------------------------------------------
//! Fame, Infamy, Favor and tier per faction; the leaders you've met; what each faction needs;
//! truces and feuds (docs/REPUTATION.md).
class RZ_StandingPage : RZ_NotebookPage
{
	//------------------------------------------------------------------------------------------------
	override ResourceName Layout(RZ_UISettings settings)
	{
		return settings.m_sStandingPage;
	}

	//------------------------------------------------------------------------------------------------
	override int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return 0;

		return ui.GetStandingVersion();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_STANDING);
	}

	//------------------------------------------------------------------------------------------------
	//! The leaders' lists scroll together.
	override void Move(int dir)
	{
		for (int i = 0; i < 4; i++)
		{
			RZ_UIW.Scroll(m_wRoot, "LeadersScroll" + i.ToString(), dir);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		bool have = false;
		if (ui)
			have = !ui.GetStanding().IsEmpty();

		RZ_UIW.Show(m_wRoot, "Waiting", !have);
		if (!have)
			return;

		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(ui.GetStanding(), recs);
		array<string> leaders = {"", "", "", ""};
		string relations = "";
		// The reputation grid: which factions put you in each of the 16 cells (RepCell_<fame>_<infamy>)
		array<string> cells = {};
		for (int c = 0; c < 16; c++)
		{
			cells.Insert("");
		}

		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
			{
				string who = string.Format("%1 - %2 - LIFE %3", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
				who.ToUpper();
				RZ_UIW.SetText(m_wRoot, "Header", who);
			}
			else if (tag == "F")
			{
				DrawFaction(f);
				int fac = RZ_UIText.Int(f, 1);
				int cell = RZ_UIText.Int(f, 8);
				if (f.Count() > 8 && cell >= 0 && fac >= 0 && fac < 4)
				{
					int idx = RZ_Reputation.Band(RZ_UIText.Int(f, 2)) * 4 + RZ_Reputation.Band(RZ_UIText.Int(f, 3));
					cells[idx] = cells[idx] + RZ_RepNames.Initial(fac);
				}
			}
			else if (tag == "L")
			{
				int lf = RZ_UIText.Int(f, 1);
				if (lf >= 0 && lf < 4)
					leaders[lf] = leaders[lf] + string.Format("%1\n%2\n%3\n\n", RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3), RZ_UIText.Str(f, 4));
			}
			else if (tag == "R")
			{
				string kind = "FEUD";
				string effect = "rival Infamy x1.5";
				if (RZ_UIText.Int(f, 1) == RZ_FactionLife.REL_TRUCE)
				{
					kind = "TRUCE";
					effect = "rival Infamy halved";
				}

				relations += string.Format("%1   %2 and %3   %4 min left, %5\n", kind, RZ_Factions.GetName(RZ_UIText.Int(f, 2)),
					RZ_Factions.GetNameInText(RZ_UIText.Int(f, 3)), RZ_UIText.Str(f, 4), effect);
			}
		}

		for (int i = 0; i < 4; i++)
		{
			string text = leaders[i];
			if (text.IsEmpty())
				text = "Nobody's given you a name yet.";

			RZ_UIW.SetText(m_wRoot, "Leaders" + i.ToString(), text);
		}

		if (relations.IsEmpty())
			relations = "No truces, no feuds. For now.";

		RZ_UIW.SetText(m_wRoot, "Relations", relations);
		DrawGrid(cells);
	}

	//------------------------------------------------------------------------------------------------
	//! The 16 cells, code-filled: each cell's name, and the initials of the factions that put you
	//! there (H the Haven, B Bastion, R the Red Hand, C Halcyon), brighter where you are.
	protected void DrawGrid(array<string> cells)
	{
		for (int fame = 0; fame < 4; fame++)
		{
			for (int infamy = 0; infamy < 4; infamy++)
			{
				string here = cells[fame * 4 + infamy];
				string text = RZ_RepNames.GetName(RZ_RepNames.CellAt(fame, infamy));
				text.ToUpper();
				Color tc = RZ_UIColor.Dim();
				if (!here.IsEmpty())
				{
					text = text + "  " + here;
					tc = RZ_UIColor.Bone();
				}

				string name = string.Format("RepCell_%1_%2", fame, infamy);
				RZ_UIW.SetText(m_wRoot, name, text);
				RZ_UIW.Tint(m_wRoot, name, tc);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! F|faction|fame|infamy|favor|tier|need|need line|rep|notoriety|word|note|left to forgive|owed
	protected void DrawFaction(array<string> f)
	{
		int fac = RZ_UIText.Int(f, 1);
		if (fac < 0 || fac > 3)
			return;

		string n = fac.ToString();
		int fame = RZ_UIText.Int(f, 2);
		int infamy = RZ_UIText.Int(f, 3);
		RZ_ETier tier = RZ_UIText.Int(f, 5);
		Color accent = RZ_UIColor.Faction(fac);
		Color tc = RZ_UIColor.Tier(tier);
		RZ_UIW.SetText(m_wRoot, "FactionName" + n, RZ_Factions.GetName(fac));
		RZ_UIW.Tint(m_wRoot, "Accent" + n, accent);
		RZ_UIW.Tint(m_wRoot, "Emblem" + n, accent);
		RZ_UIW.Icon(m_wRoot, "Emblem" + n, RZ_UIText.FactionIcon(fac));
		RZ_UIW.Icon(m_wRoot, "TierIcon" + n, RZ_UIText.TierIcon(tier));
		RZ_UIW.Tint(m_wRoot, "TierIcon" + n, tc);
		// The reputation's name (Reputation > Names), else the tier word; the note says what it counts
		// as (docs/REPUTATION.md, "Reputations")
		string tierName = RZ_Reputation.GetTierName(tier);
		int rep = RZ_UIText.Int(f, 8);
		if (f.Count() > 8 && rep >= 0)
			tierName = RZ_RepNames.GetName(rep);

		tierName.ToUpper();
		RZ_UIW.SetText(m_wRoot, "TierName" + n, tierName);
		RZ_UIW.Tint(m_wRoot, "TierName" + n, tc);
		string note = RZ_UIText.Str(f, 11);
		if (note.IsEmpty())
			note = RZ_UIText.TierNote(tier);

		RZ_UIW.SetText(m_wRoot, "TierNote" + n, note);
		RZ_UIW.SetText(m_wRoot, "FameValue" + n, fame.ToString());
		RZ_UIW.Progress(m_wRoot, "FameBar" + n, fame / 100.0);
		RZ_UIW.Tint(m_wRoot, "FameBar" + n, accent);
		// The forgiveness budget left, when there's Infamy to forgive: "36 - 12 TO FORGIVE"
		string infamyText = infamy.ToString();
		if (f.Count() > 12 && infamy > 0 && RZ_UIText.Int(f, 12) >= 0)
			infamyText = string.Format("%1 - %2 TO FORGIVE", infamy, RZ_UIText.Int(f, 12));

		RZ_UIW.SetText(m_wRoot, "InfamyValue" + n, infamyText);
		RZ_UIW.Progress(m_wRoot, "InfamyBar" + n, infamy / 100.0);
		RZ_UIW.SetText(m_wRoot, "FavorValue" + n, RZ_UIText.Str(f, 4));
		RZ_UIW.Tint(m_wRoot, "FavorValue" + n, accent);
		string needLine = RZ_UIText.Str(f, 7);
		RZ_UIW.Show(m_wRoot, "NeedBlock" + n, !needLine.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "NeedTitle" + n, RZ_UIText.NeedTitle(RZ_UIText.Int(f, 6)));
		RZ_UIW.SetText(m_wRoot, "Need" + n, needLine);
		RZ_UIW.Show(m_wRoot, "Warn" + n, RZ_Reputation.IsHunted(tier));
	}
}

//------------------------------------------------------------------------------------------------
//! Active contracts and events with type, giver, rough direction, requirements and rewards, the
//! puzzle clues you've read (docs/MISSIONS.md), and the lab's line (research and its backlog).
class RZ_JournalPage : RZ_NotebookPage
{
	protected int m_iSelected;
	protected ref array<ref array<string>> m_aMissions = {};
	protected ref array<ref array<string>> m_aClues = {};
	protected ref array<Widget> m_aRows = {};

	//------------------------------------------------------------------------------------------------
	override ResourceName Layout(RZ_UISettings settings)
	{
		return settings.m_sJournalPage;
	}

	//------------------------------------------------------------------------------------------------
	override int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return m_iSelected;

		return ui.GetJournalVersion() * 10000 + ui.GetDetailVersion() * 100 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_JOURNAL);
	}

	//------------------------------------------------------------------------------------------------
	override bool FocusFirst()
	{
		if (m_aRows.IsEmpty())
			return false;

		RZ_UIW.FocusWidget(m_aRows[Math.Min(m_iSelected, m_aRows.Count() - 1)]);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Move(int dir)
	{
		m_iSelected = Math.Clamp(m_iSelected + dir, 0, Math.Max(0, m_aMissions.Count() - 1));
	}

	//------------------------------------------------------------------------------------------------
	//! A or Enter: the row with the focus becomes the selected one.
	override void Select()
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!ws)
			return;

		Widget focused = ws.GetFocusedWidget();
		int idx = RZ_UIW.GetTag(focused);
		if (focused && focused.GetName() == "MissionRow" && idx >= 0 && idx < m_aMissions.Count())
			m_iSelected = idx;
	}

	//------------------------------------------------------------------------------------------------
	override void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		m_aMissions.Clear();
		m_aClues.Clear();
		string lab = "";
		if (ui)
		{
			array<ref array<string>> recs = {};
			RZ_MenuBase.Records(ui.GetJournal(), recs);
			foreach (array<string> f : recs)
			{
				if (RZ_UIText.Str(f, 0) == "M")
					m_aMissions.Insert(f);
				else if (RZ_UIText.Str(f, 0) == "C")
					m_aClues.Insert(f);
				else if (RZ_UIText.Str(f, 0) == "S")
					lab = RZ_UIText.Str(f, 1);
			}
		}

		// The lab's line: research, what waits in its backlog and how long that takes
		RZ_UIW.Show(m_wRoot, "LabStatus", !lab.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "LabStatus", lab);
		RZ_UIW.Show(m_wRoot, "Empty", m_aMissions.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "Header", string.Format("%1 OPEN", m_aMissions.Count()));
		DrawList();
		if (!m_aMissions.IsEmpty())
		{
			m_iSelected = Math.Min(m_iSelected, m_aMissions.Count() - 1);
			DrawDetail(m_aMissions[m_iSelected]);
			FocusFirst();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawList()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "List");
		RZ_UIW.ClearChildren(list);
		m_aRows.Clear();
		ResourceName rowLayout = RZ_UISettings.Get().m_sMissionRowLayout;
		foreach (int i, array<string> m : m_aMissions)
		{
			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				return;

			int giver = RZ_UIText.Int(m, 3);
			Color accent = RZ_UIColor.Hazard();
			string sub = RZ_UIText.Str(m, 6);
			if (giver >= 0)
			{
				accent = RZ_UIColor.Faction(giver);
				sub = RZ_Factions.GetName(giver) + " - " + sub;
			}

			RZ_UIW.Icon(row, "RowIcon", RZ_UIText.MissionIcon(RZ_UIText.Int(m, 2)));
			RZ_UIW.Tint(row, "RowIcon", accent);
			RZ_UIW.Tint(row, "RowAccent", accent);
			RZ_UIW.Show(row, "RowSelected", i == m_iSelected);
			RZ_UIW.SetText(row, "RowTitle", RZ_UIText.Str(m, 4));
			RZ_UIW.SetText(row, "RowSub", sub);
			RZ_UIW.SetText(row, "RowDir", RZ_UIText.Str(m, 7));
			// A questline has no clock (minutes -1); a pack shows what you know of it
			string rowTime = RZ_UIText.Str(m, 8) + " min left";
			if (RZ_UIText.Int(m, 2) == RZ_EMissionKindUI.PACK || RZ_UIText.Int(m, 2) == RZ_EMissionKindUI.DOORS)
				rowTime = RZ_UIText.Str(m, 5);
			else if (RZ_UIText.Int(m, 8) < 0)
				rowTime = "Questline";

			RZ_UIW.SetText(row, "RowTime", rowTime);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRows.Insert(row);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a row (its root, MissionRow, is a Button) selects it.
	override void OnClick(Widget w, string name)
	{
		int idx = RZ_UIW.GetTag(w);
		if (name == "MissionRow" && idx >= 0 && idx < m_aMissions.Count())
			m_iSelected = idx;
	}

	//------------------------------------------------------------------------------------------------
	//! M|id|kind|giver|title|given by|where|direction|minutes|requirements|Fame|Favor|brief|taking part
	protected void DrawDetail(array<string> m)
	{
		int giver = RZ_UIText.Int(m, 3);
		Color accent = RZ_UIColor.Hazard();
		string icon = RZ_UIText.MissionIcon(RZ_UIText.Int(m, 2));
		if (giver >= 0)
		{
			accent = RZ_UIColor.Faction(giver);
			icon = RZ_UIText.FactionIcon(giver);
		}

		RZ_UIW.Tint(m_wRoot, "DetailAccent", accent);
		RZ_UIW.Tint(m_wRoot, "DetailIcon", accent);
		RZ_UIW.Icon(m_wRoot, "DetailIcon", icon);
		RZ_UIW.SetText(m_wRoot, "DetailTitle", RZ_UIText.Str(m, 4));
		string by = RZ_UIText.Str(m, 5);
		if (by.IsEmpty() && giver >= 0)
			by = RZ_Factions.GetName(giver);

		RZ_UIW.SetText(m_wRoot, "DetailBy", by);
		RZ_UIW.SetText(m_wRoot, "DetailBrief", RZ_UIText.Str(m, 12));
		RZ_UIW.SetText(m_wRoot, "DetailWhere", string.Format("%1\n%2", RZ_UIText.Str(m, 7), RZ_UIText.Str(m, 6)));
		string req = RZ_UIText.Str(m, 9);
		if (req.IsEmpty())
			req = "Open to anyone.";

		RZ_UIW.SetText(m_wRoot, "DetailReq", req);
		string rewards = "";
		if (RZ_UIText.Int(m, 2) == RZ_EMissionKindUI.PACK)
			rewards = "Heard of: a rough area. Read: its ground. Mapped: where it lies up. Read and Mapped are rings on the map.";
		else if (RZ_UIText.Int(m, 2) == RZ_EMissionKindUI.DOORS)
			rewards = "What people wrote on doors, as you read it. It goes with this life.";
		else if (RZ_UIText.Int(m, 8) < 0)
			rewards = "It depends on how it ends.";
		else if (giver >= 0)
			rewards = string.Format("About %1 Fame and %2 Favor with %3", RZ_UIText.Str(m, 10), RZ_UIText.Str(m, 11), RZ_Factions.GetNameInText(giver));

		RZ_UIW.SetText(m_wRoot, "DetailRewards", rewards);

		string id = RZ_UIText.Str(m, 1);
		string clues = "";

		// The row's detail, asked for when it's selected (what they told you, what you've found,
		// who it went to)
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
		{
			ui.AskDetail(RZ_UIText.Int(m, 1));
			array<ref array<string>> detail = {};
			RZ_MenuBase.Records(ui.GetDetail(), detail);
			foreach (array<string> d : detail)
			{
				if (RZ_UIText.Str(d, 0) != "D" || RZ_UIText.Str(d, 1) != id)
					continue;

				if (!RZ_UIText.Str(d, 3).IsEmpty())
					clues += "What you've found\n" + RZ_UIText.Str(d, 3) + "\n\n";

				if (!RZ_UIText.Str(d, 4).IsEmpty())
					clues += "Who it went to\n" + RZ_UIText.Str(d, 4) + "\n\n";
			}
		}

		foreach (array<string> c : m_aClues)
		{
			if (RZ_UIText.Str(c, 1) == id)
				clues += string.Format("%1\n%2\n\n", RZ_UIText.Str(c, 2), RZ_UIText.Str(c, 3));
		}

		RZ_UIW.Show(m_wRoot, "CluesBlock", !clues.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "DetailClues", clues);
	}
}

//------------------------------------------------------------------------------------------------
//! What you heard and were told this life, newest first: the broadcast, open channels, cracked
//! secure channels, and the mission and world notices (RZ_PlayerUIComponent.OnNotice). X or F
//! (or a click on the chip) cycles the filter: All, Open channel, Secure, Broadcast, Missions, World.
class RZ_RadioPage : RZ_NotebookPage
{
	protected static int s_iFilter;	// 0 all, 1 open, 2 secure, 3 broadcast, 4 missions, 5 world; kept for the session
	protected static const int FILTERS = 6;

	//------------------------------------------------------------------------------------------------
	override ResourceName Layout(RZ_UISettings settings)
	{
		return settings.m_sRadioPage;
	}

	//------------------------------------------------------------------------------------------------
	override int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return s_iFilter;

		return ui.GetRadioVersion() * 10 + s_iFilter;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		RZ_UIW.OnClick(m_wRoot, "FilterButton", m_Click);
	}

	//------------------------------------------------------------------------------------------------
	override bool FocusFirst()
	{
		RZ_UIW.Focus(m_wRoot, "FilterButton");
		return RZ_UIW.Find(m_wRoot, "FilterButton") != null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnClick(Widget w, string name)
	{
		if (name == "FilterButton")
			Filter();
	}

	//------------------------------------------------------------------------------------------------
	override void Select()
	{
		Filter();
	}

	//------------------------------------------------------------------------------------------------
	override void Filter()
	{
		s_iFilter = (s_iFilter + 1) % FILTERS;
	}

	//------------------------------------------------------------------------------------------------
	override void Move(int dir)
	{
		RZ_UIW.Scroll(m_wRoot, "LogScroll", dir);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Passes(RZ_RadioEntry e)
	{
		switch (s_iFilter)
		{
			case 1: return e.m_iKind == RZ_ENoticeKind.OPEN_CHANNEL;
			case 2: return e.m_iKind == RZ_ENoticeKind.SECURE_CHANNEL;
			case 3: return e.m_iKind == RZ_ENoticeKind.BROADCAST;
			case 4: return e.m_iKind == RZ_ENoticeKind.MISSION;
			case 5: return e.m_iKind == RZ_ENoticeKind.EVENT || e.m_iKind == RZ_ENoticeKind.WORLD;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Redraw()
	{
		array<string> names = {"ALL", "OPEN CHANNEL", "SECURE", "BROADCAST", "MISSIONS", "WORLD"};
		RZ_UIW.SetText(m_wRoot, "FilterText", names[s_iFilter]);
		RZ_UIW.SetText(m_wRoot, "Header", "WHAT YOU HEARD AND WERE TOLD THIS LIFE, NEWEST FIRST");
		Widget list = RZ_UIW.Find(m_wRoot, "Log");
		RZ_UIW.ClearChildren(list);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		ResourceName rowLayout = RZ_UISettings.Get().m_sRadioRowLayout;
		int shown = 0;
		foreach (RZ_RadioEntry e : ui.GetRadioLog())
		{
			if (!Passes(e))
				continue;

			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				break;

			RZ_NoticeStyle st = new RZ_NoticeStyle();
			st.m_iKind = e.m_iKind;
			st.m_iFaction = e.m_iFaction;
			Color accent = st.Accent();
			RZ_UIW.Tint(row, "RowAccent", accent);
			RZ_UIW.Tint(row, "RowIcon", accent);
			RZ_UIW.Tint(row, "RowChannel", accent);
			RZ_UIW.Icon(row, "RowIcon", st.Icon());
			RZ_UIW.SetText(row, "RowTime", e.m_sTime);
			RZ_UIW.SetText(row, "RowChannel", st.KindText());
			RZ_UIW.SetText(row, "RowSpeaker", e.m_sTitle);
			RZ_UIW.SetText(row, "RowText", e.m_sText);
			shown++;
		}

		RZ_UIW.Show(m_wRoot, "Empty", shown == 0);
	}
}

//------------------------------------------------------------------------------------------------
//! The map key: a static legend of the marker icons (the page layout holds it all).
class RZ_MapKeyPage : RZ_NotebookPage
{
	//------------------------------------------------------------------------------------------------
	override ResourceName Layout(RZ_UISettings settings)
	{
		return settings.m_sMapKeyPage;
	}

	//------------------------------------------------------------------------------------------------
	override void Redraw()
	{
		RZ_UIW.SetText(m_wRoot, "Header", "WHAT THE MAP SHOWS YOU");
	}
}

//------------------------------------------------------------------------------------------------
//! The season tab (docs/UI.md, "The notebook"): the four majors as they stand, then, in the
//! epilogue, the season as the Zone tells it, your lines and what's still open. Read only.
class RZ_SeasonPage : RZ_NotebookPage
{
	//------------------------------------------------------------------------------------------------
	override ResourceName Layout(RZ_UISettings settings)
	{
		return settings.m_sSeasonPage;
	}

	//------------------------------------------------------------------------------------------------
	override int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return 0;

		return ui.GetSeasonVersion();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_SEASON);
	}

	//------------------------------------------------------------------------------------------------
	override void Move(int dir)
	{
		RZ_UIW.Scroll(m_wRoot, "ZoneScroll", dir);
	}

	//------------------------------------------------------------------------------------------------
	//! M|line (the majors), Z|line (as the Zone tells it), Y|line (and you, unfinished)
	override void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		bool have = false;
		if (ui)
			have = !ui.GetSeason().IsEmpty();

		RZ_UIW.Show(m_wRoot, "Waiting", !have);
		if (!have)
			return;

		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(ui.GetSeason(), recs);
		string majors = "";
		string zone = "";
		string you = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			string line = RZ_UIText.Str(f, 1) + "\n";
			if (tag == "M")
				majors += line;
			else if (tag == "Z")
				zone += line;
			else if (tag == "Y")
				you += line;
		}

		if (majors.IsEmpty())
			majors = "No story this season yet.";

		if (zone.IsEmpty())
			zone = "The season isn't over. Nobody's writing it down yet.";

		RZ_UIW.SetText(m_wRoot, "MajorsList", majors);
		RZ_UIW.SetText(m_wRoot, "ZoneList", zone);
		RZ_UIW.SetText(m_wRoot, "YouList", you);
	}
}

//------------------------------------------------------------------------------------------------
//! The hall of fame (Persistence/RZ_HallOfFame.c): this season's names, all time, and past seasons.
class RZ_HallPage : RZ_NotebookPage
{
	//------------------------------------------------------------------------------------------------
	override ResourceName Layout(RZ_UISettings settings)
	{
		return settings.m_sHallPage;
	}

	//------------------------------------------------------------------------------------------------
	override int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return 0;

		return ui.GetHallVersion();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_HALL);
	}

	//------------------------------------------------------------------------------------------------
	override void Move(int dir)
	{
		RZ_UIW.Scroll(m_wRoot, "PastScroll", dir);
	}

	//------------------------------------------------------------------------------------------------
	//! H|season, S and A|category|category name|name|value, P|season|winner|lines
	override void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		bool have = false;
		if (ui)
			have = !ui.GetHall().IsEmpty();

		RZ_UIW.Show(m_wRoot, "Waiting", !have);
		if (!have)
			return;

		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(ui.GetHall(), recs);
		string seasonText = "";
		string allText = "";
		string pastText = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
			{
				RZ_UIW.SetText(m_wRoot, "Header", "SEASON " + RZ_UIText.Str(f, 1));
			}
			else if (tag == "S" || tag == "A")
			{
				string line = string.Format("%1\n%2", RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
				string value = RZ_UIText.Str(f, 4);
				if (!value.IsEmpty())
					line += "  (" + value + ")";

				if (tag == "S")
					seasonText += line + "\n\n";
				else
					allText += line + "\n\n";
			}
			else if (tag == "P")
			{
				pastText += string.Format("Season %1: the cure went to %2.\n%3\n\n", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
			}
		}

		if (seasonText.IsEmpty())
			seasonText = "Nobody's name is on it yet.";

		RZ_UIW.SetText(m_wRoot, "SeasonList", seasonText);
		RZ_UIW.SetText(m_wRoot, "AllTimeList", allText);
		RZ_UIW.SetText(m_wRoot, "PastList", pastText);
	}
}
