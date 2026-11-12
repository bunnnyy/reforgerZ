//! The notebook (docs/UI.md, "The notebook"): one menu with five tabs, Standing, Journal, Radio,
//! Hall of fame and The season (the map key is on the map itself, RZ_MapIcons.c). The menu (RZ_NotebookMenu, layout RZ_NotebookMenu.layout) holds the
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
	HALL,
	SEASON
}

//------------------------------------------------------------------------------------------------
//! Opening the notebook (client).
class RZ_Notebook
{
	static const int TABS = 5;

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
//! The Standing page (docs/UI.md, "The notebook"; layout tree in UI/LAYOUTS.md): one row per faction,
//! each the name they call you, Favor, the leaders you've met, what they need and a kit tell, all
//! four always on screen. One row is open and shows how they see you (and the next name up),
//! the leaders, the need and their kit; up/down or a hover moves which, A, Enter or a click pins it.
//! Truces and feuds are one line under the rows (docs/REPUTATION.md).
class RZ_StandingPage : RZ_NotebookPage
{
	protected static const float ROW_W = 1728;
	protected static const float OPEN_H = 444;
	protected static const float CLOSED_H = 84;
	protected static const float GAP = 8;

	protected static int s_iOpen;		// the open row (faction 0-3), kept for the session
	protected static bool s_bPinned;	// a click, A or Enter pinned it: a hover doesn't move it
	protected ref RZ_StandingRowHandler m_RowHandler;

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

		// The rows (Buttons Row0-Row3) hear clicks and the mouse coming over them
		m_RowHandler = new RZ_StandingRowHandler(this);
		for (int i = 0; i < 4; i++)
		{
			Widget row = RZ_UIW.Find(m_wRoot, "Row" + i.ToString());
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_RowHandler);
		}

		Arrange();
	}

	//------------------------------------------------------------------------------------------------
	override void Close()
	{
		if (m_RowHandler)
			m_RowHandler.Drop();
	}

	//------------------------------------------------------------------------------------------------
	override bool FocusFirst()
	{
		Widget row = RZ_UIW.Find(m_wRoot, "Row" + s_iOpen.ToString());
		if (!row)
			return false;

		RZ_UIW.FocusWidget(row);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Up/down (the D-pad, the left stick, the arrows): the next row opens; a pin goes with it.
	override void Move(int dir)
	{
		s_iOpen = Math.ClampInt(s_iOpen + dir, 0, 3);
		Arrange();
		FocusFirst();
	}

	//------------------------------------------------------------------------------------------------
	//! A or Enter: pins the open row, or lets it go.
	override void Select()
	{
		ClickRow(s_iOpen);
	}

	//------------------------------------------------------------------------------------------------
	//! The mouse came over a row: it opens, unless another is pinned.
	void HoverRow(int idx)
	{
		if (s_bPinned || idx < 0 || idx > 3 || idx == s_iOpen)
			return;

		s_iOpen = idx;
		Arrange();
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a row: opens and pins it; a click on the pinned row lets it go.
	void ClickRow(int idx)
	{
		if (idx < 0 || idx > 3)
			return;

		if (s_bPinned && idx == s_iOpen)
		{
			s_bPinned = false;
		}
		else
		{
			s_iOpen = idx;
			s_bPinned = true;
		}

		Arrange();
		FocusFirst();
	}

	//------------------------------------------------------------------------------------------------
	//! Stacks the rows: the open one fills the free space and shows its detail, the others are one
	//! line each. A pinned row's border is bone.
	protected void Arrange()
	{
		float y = 0;
		for (int i = 0; i < 4; i++)
		{
			string n = i.ToString();
			bool open = i == s_iOpen;
			float h = CLOSED_H;
			if (open)
				h = OPEN_H;

			Widget row = RZ_UIW.Find(m_wRoot, "Row" + n);
			RZ_UIW.SetPos(row, 0, y);
			RZ_UIW.SetSize(row, ROW_W, h);
			RZ_UIW.Show(m_wRoot, "Detail" + n, open);
			RZ_UIW.Show(m_wRoot, "RowOpen" + n, open);
			Color line = RZ_UIW.Rgb(42, 46, 49);
			if (open && s_bPinned)
				line = RZ_UIColor.Bone();

			RZ_UIW.Tint(m_wRoot, "RowLine" + n, line);
			y += h + GAP;
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
		RZ_UIW.Show(m_wRoot, "Rows", have);
		RZ_UIW.Show(m_wRoot, "ColumnHeads", have);
		if (!have)
			return;

		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(ui.GetStanding(), recs);
		array<ref array<string>> factions = {};
		array<string> names = {"", "", "", ""};
		array<int> met = {0, 0, 0, 0};
		array<string> kit = {"", "", "", ""};
		array<int> kitState = {-1, -1, -1, -1};
		array<string> words = {"", "", "", ""};
		array<string> verdicts = {"", "", "", ""};
		string relations = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
			{
				string who = string.Format("%1 - %2 - LIFE %3", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
				who.ToUpper();
				// The whole picture after it, as said (RZ_RepNamesPage.Header)
				string picture = RZ_UIText.Str(f, 4);
				if (!picture.IsEmpty())
					who = who + "   " + picture;

				RZ_UIW.SetText(m_wRoot, "Header", who);
			}
			else if (tag == "F")
			{
				factions.Insert(f);
			}
			else if (tag == "K")
			{
				int kf = RZ_UIText.Int(f, 1);
				if (kf >= 0 && kf < 4)
				{
					kit[kf] = RZ_UIText.Str(f, 2);
					kitState[kf] = KitState(f);
				}
			}
			else if (tag == "W")
			{
				int wf = RZ_UIText.Int(f, 1);
				if (wf >= 0 && wf < 4)
				{
					words[wf] = RZ_UIText.Str(f, 2);
					verdicts[wf] = RZ_UIText.Str(f, 3);
				}
			}
			else if (tag == "L")
			{
				int lf = RZ_UIText.Int(f, 1);
				if (lf >= 0 && lf < 4)
				{
					string title = RZ_UIText.Str(f, 3);
					string leader = RZ_UIText.Str(f, 2);
					if (!title.IsEmpty())
						leader = leader + ", " + title;

					names[lf] = names[lf] + leader + "\n";
					met[lf] = met[lf] + 1;
				}
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

				if (!relations.IsEmpty())
					relations += "      ";

				relations += string.Format("%1  %2 and %3, %4 min left (%5)", kind, RZ_Factions.GetName(RZ_UIText.Int(f, 2)),
					RZ_Factions.GetNameInText(RZ_UIText.Int(f, 3)), RZ_UIText.Str(f, 4), effect);
			}
		}

		foreach (array<string> ff : factions)
		{
			int fac = RZ_UIText.Int(ff, 1);
			if (fac < 0 || fac > 3)
				continue;

			DrawFaction(ff, met[fac], kitState[fac]);
			DrawDetail(ff, words[fac], verdicts[fac], names[fac], kit[fac]);
		}

		if (relations.IsEmpty())
			relations = "No truces, no feuds. For now.";

		RZ_UIW.SetText(m_wRoot, "Relations", relations);
		Arrange();
	}

	//------------------------------------------------------------------------------------------------
	//! K|faction|kit line|they know your face (0/1)|catches this life|blown (0/1): 2 blown, 1 known
	//! or caught, 0 neither (an older line without the numbers too).
	protected static int KitState(array<string> f)
	{
		if (f.Count() > 5 && RZ_UIText.Int(f, 5) == 1)
			return 2;

		if (f.Count() > 4 && (RZ_UIText.Int(f, 3) == 1 || RZ_UIText.Int(f, 4) > 0))
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The row's one line: emblem and name, the name they call you, Favor, leaders met, the need and
	//! the kit tell.
	//! F|faction|fame|infamy|favor|tier|need|need line|rep|notoriety|word|note|left to forgive|owed
	protected void DrawFaction(array<string> f, int leaders, int kitState)
	{
		int fac = RZ_UIText.Int(f, 1);
		string n = fac.ToString();
		RZ_ETier tier = RZ_UIText.Int(f, 5);
		Color accent = RZ_UIColor.Faction(fac);
		Color tc = RZ_UIColor.Tier(tier);
		string facName = RZ_Factions.GetName(fac);
		facName.ToUpper();
		RZ_UIW.SetText(m_wRoot, "FactionName" + n, facName);
		RZ_UIW.Tint(m_wRoot, "Accent" + n, accent);
		RZ_UIW.Tint(m_wRoot, "Emblem" + n, accent);
		RZ_UIW.Icon(m_wRoot, "Emblem" + n, RZ_UIText.FactionIcon(fac));
		RZ_UIW.Icon(m_wRoot, "TierIcon" + n, RZ_UIText.TierIcon(tier));
		RZ_UIW.Tint(m_wRoot, "TierIcon" + n, tc);
		// The reputation's name (Reputation > Names), else the tier word (docs/REPUTATION.md, "Reputations")
		string tierName = RZ_Reputation.GetTierName(tier);
		int rep = RZ_UIText.Int(f, 8);
		if (f.Count() > 8 && rep >= 0)
			tierName = RZ_RepNames.GetName(rep);

		tierName.ToUpper();
		RZ_UIW.SetText(m_wRoot, "TierName" + n, tierName);
		RZ_UIW.Tint(m_wRoot, "TierName" + n, tc);
		RZ_UIW.Tint(m_wRoot, "FavorIcon" + n, accent);
		RZ_UIW.SetText(m_wRoot, "FavorValue" + n, RZ_UIText.Str(f, 4));

		string count = "NONE";
		Color leaderTint = RZ_UIColor.Dim();
		if (leaders > 0)
		{
			count = leaders.ToString();
			leaderTint = accent;
		}

		RZ_UIW.SetText(m_wRoot, "LeaderCount" + n, count);
		RZ_UIW.Tint(m_wRoot, "LeaderCount" + n, leaderTint);
		RZ_UIW.Tint(m_wRoot, "LeaderIcon" + n, leaderTint);

		int need = RZ_UIText.Int(f, 6);
		bool needs = need != RZ_ENeed.NONE && !RZ_UIText.NeedTitle(need).IsEmpty();
		RZ_UIW.Show(m_wRoot, "NeedIcon" + n, needs);
		string needTitle = "NONE";
		Color needTint = RZ_UIColor.Dim();
		if (needs)
		{
			needTitle = RZ_UIText.NeedTitle(need);
			needTint = RZ_UIColor.Bone();
			RZ_UIW.Icon(m_wRoot, "NeedIcon" + n, NeedIcon(need));
			RZ_UIW.Tint(m_wRoot, "NeedIcon" + n, accent);
		}

		RZ_UIW.SetText(m_wRoot, "NeedTitle" + n, needTitle);
		RZ_UIW.Tint(m_wRoot, "NeedTitle" + n, needTint);

		// The kit tell: none without a kit line (the Haven, borrowed kit off); dim while they don't
		// know you, hazard once they know your face or caught you, red while you're blown
		RZ_UIW.Show(m_wRoot, "KitIcon" + n, kitState >= 0);
		Color kitTint = RZ_UIColor.Dim();
		if (kitState == 1)
			kitTint = RZ_UIColor.Hazard();
		else if (kitState == 2)
			kitTint = RZ_UIColor.Bad();

		RZ_UIW.Tint(m_wRoot, "KitIcon" + n, kitTint);
	}

	//------------------------------------------------------------------------------------------------
	//! The open row's four blocks: how they see you (and the next name), the leaders you've met,
	//! what they need, their kit.
	protected void DrawDetail(array<string> f, string word, string verdict, string leaders, string kit)
	{
		int fac = RZ_UIText.Int(f, 1);
		string n = fac.ToString();
		int fame = RZ_UIText.Int(f, 2);
		int infamy = RZ_UIText.Int(f, 3);
		RZ_ETier tier = RZ_UIText.Int(f, 5);
		Color accent = RZ_UIColor.Faction(fac);

		// How they see you: their word as a sentence, what the name counts as, what's left to
		// forgive (only with Infamy to forgive), and the hunters
		string see = word;
		string note = RZ_UIText.Str(f, 11);
		if (note.IsEmpty())
			note = RZ_UIText.TierNote(tier);

		if (!note.IsEmpty())
		{
			if (!see.IsEmpty())
				see += "\n\n";

			see += note;
		}

		int left = -1;
		if (f.Count() > 12)
			left = RZ_UIText.Int(f, 12);

		if (infamy > 0 && left >= 0)
			see += string.Format("\nLeft to forgive this life: %1 Infamy", left);

		if (RZ_Reputation.IsHunted(tier))
			see += "\nHunter squads are out for you.";

		RZ_UIW.SetText(m_wRoot, "SeeText" + n, see);
		RZ_UIW.Tint(m_wRoot, "SeeIcon" + n, accent);
		string next = NextText(fame, infamy, left);
		RZ_UIW.Show(m_wRoot, "NextBlock" + n, !next.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "NextText" + n, next);

		// The leaders you've met, and one of them in their own words (the nearest)
		string leaderText = leaders;
		if (leaderText.IsEmpty())
			leaderText = "Nobody's given you a name yet.";
		else if (!verdict.IsEmpty())
			leaderText += "\n" + verdict;

		RZ_UIW.SetText(m_wRoot, "LeadersText" + n, leaderText);
		RZ_UIW.Tint(m_wRoot, "LeadersIconBig" + n, accent);

		int need = RZ_UIText.Int(f, 6);
		string needLine = RZ_UIText.Str(f, 7);
		string needName = "";
		if (need != RZ_ENeed.NONE)
		{
			needName = RZ_UIText.NeedTitle(need);
			RZ_UIW.Icon(m_wRoot, "NeedIconBig" + n, NeedIcon(need));
		}

		if (needLine.IsEmpty())
			needLine = "Nothing they're short of today.";

		RZ_UIW.SetText(m_wRoot, "NeedName" + n, needName);
		RZ_UIW.Tint(m_wRoot, "NeedName" + n, accent);
		RZ_UIW.SetText(m_wRoot, "NeedText" + n, needLine);
		RZ_UIW.Tint(m_wRoot, "NeedIconBig" + n, accent);

		string kitText = kit;
		if (kitText.IsEmpty() && fac == RZ_EFaction.SURVIVORS)
			kitText = "The Haven wears no colours to borrow.";
		else if (kitText.IsEmpty())
			kitText = "Borrowed kit is off on this server.";

		RZ_UIW.SetText(m_wRoot, "KitText" + n, kitText);
		RZ_UIW.Tint(m_wRoot, "KitIconBig" + n, accent);
	}

	//------------------------------------------------------------------------------------------------
	//! The next name up and what earns it, from the bands (docs/REPUTATION.md, "Reputations"): more
	//! Fame to the next Fame band, and Infamy worked off to the band below (not once they've forgiven
	//! all they will this life). "" at the top (Champion).
	protected static string NextText(int fame, int infamy, int left)
	{
		int fameBand = RZ_Reputation.Band(fame);
		int infamyBand = RZ_Reputation.Band(infamy);
		string now = RZ_RepNames.Shown(fame, infamy);
		string text = "";
		if (fameBand < 3)
		{
			int fameUp = BandFloor(fameBand + 1);
			string upName = RZ_RepNames.Shown(fameUp, infamy);
			if (upName != now)
			{
				upName.ToUpper();
				text = string.Format("%1: %2 more Fame", upName, fameUp - fame);
			}
		}

		if (infamyBand > 0 && left != 0)
		{
			int infamyDown = BandFloor(infamyBand) - 1;
			string downName = RZ_RepNames.Shown(fame, infamyDown);
			if (downName != now)
			{
				downName.ToUpper();
				if (!text.IsEmpty())
					text += "\n";

				text += string.Format("%1: work off %2 Infamy", downName, infamy - infamyDown);
			}
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The lowest value of a band (RZ_Reputation.Band): 0, 15, 40, 75.
	protected static int BandFloor(int band)
	{
		switch (band)
		{
			case 1: return 15;
			case 2: return 40;
			case 3: return 75;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The icon for a faction's need (UI/Icons, "Standing and needs").
	protected static string NeedIcon(int need)
	{
		switch (need)
		{
			case RZ_ENeed.FOOD: return "need_food";
			case RZ_ENeed.SUPPLIES: return "status_shortage";
			case RZ_ENeed.SECURITY: return "need_ammo";
			case RZ_ENeed.SAMPLES: return "item_sample";
			case RZ_ENeed.PEOPLE: return "need_people";
			case RZ_ENeed.REVENGE: return "need_revenge";
		}

		return "status_shortage";
	}
}

//------------------------------------------------------------------------------------------------
//! The Standing page's rows hear the mouse: a hover opens a row (unless one is pinned), a click
//! pins it or lets it go. Pad and keys go through RZ_StandingPage.Move and Select.
class RZ_StandingRowHandler : ScriptedWidgetEventHandler
{
	protected RZ_StandingPage m_Page;

	//------------------------------------------------------------------------------------------------
	void RZ_StandingRowHandler(RZ_StandingPage page)
	{
		m_Page = page;
	}

	//------------------------------------------------------------------------------------------------
	//! The page is closing: hear nothing more.
	void Drop()
	{
		m_Page = null;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_Page || !w)
			return false;

		m_Page.ClickRow(RZ_UIW.GetTag(w));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: OnMouseEnter reaches a Button's handler when the cursor comes over its children.
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_Page && w)
			m_Page.HoverRow(RZ_UIW.GetTag(w));

		return false;
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
			if (RZ_UIText.Int(m, 2) == RZ_EMissionKindUI.PACK || RZ_UIText.Int(m, 2) == RZ_EMissionKindUI.DOORS || RZ_UIText.Int(m, 2) == RZ_EMissionKindUI.FINALE)
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
		else if (RZ_UIText.Int(m, 2) == RZ_EMissionKindUI.FINALE)
			rewards = "The lab's readout by faction, never by player. Lock counts are never shown.";
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
