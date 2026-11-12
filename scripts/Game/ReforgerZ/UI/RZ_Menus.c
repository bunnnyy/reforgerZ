//! The ReforgerZ menus (docs/UI.md, "Screens"): the shared base, and the safehouse notice board.
//! The notebook (Standing, Journal, Radio, Hall of fame and The season as tabs) is in RZ_Notebook.c, the
//! talk screen in RZ_TalkMenu.c, the reader in RZ_Reader.c, the car repair screen in RZ_CarMenu.c,
//! the shop screen in RZ_ShopMenu.c, the crafting screen in RZ_CraftMenu.c, the object menu in RZ_ObjectMenu.c, the power
//! panel in RZ_PowerMenu.c and the spawn screen in RZ_SpawnMenu.c. Layouts and widget names are in
//! UI/LAYOUTS.md; the presets are registered in a chimeraMenus.conf override (WORKBENCH_SETUP.md).
//!
//! Each menu asks RZ_PlayerUIComponent for its data when it opens and redraws when a new copy
//! arrives. They only read; nothing here changes the game.
//!
//! Mouse: the buttons in the layouts (tabs, CloseButton, rows, the filter chip, PostButton) report
//! their clicks through RZ_MenuClick. Gamepad: every menu closes with B (MenuBack) and gives a
//! widget the focus when it opens.

modded enum ChimeraMenuPreset // each entry needs a matching chimeraMenus.conf entry
{
	RZ_NotebookMenu,
	RZ_NoticeBoardMenu,
	RZ_ReaderMenu,
	RZ_TalkMenu,
	RZ_CarMenu,
	RZ_ShopMenu,
	RZ_SpawnMenu,
	RZ_CraftMenu,
	RZ_ObjectMenu,
	RZ_PowerMenu
}

//------------------------------------------------------------------------------------------------
//! Passes a clicked widget to its menu.
class RZ_MenuClick : ScriptedWidgetEventHandler
{
	protected RZ_MenuBase m_Menu;

	//------------------------------------------------------------------------------------------------
	void RZ_MenuClick(RZ_MenuBase menu)
	{
		m_Menu = menu;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_Menu || !w)
			return false;

		m_Menu.OnWidgetClick(w);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! The shared base: Esc / B and the close button close it; it redraws when DataVersion changes.
class RZ_MenuBase : ChimeraMenuBase
{
	protected static const float POLL = 0.25;

	protected Widget m_wRoot;
	protected int m_iSeen = -1;
	protected float m_fPoll;
	protected ref RZ_MenuClick m_Click;

	//------------------------------------------------------------------------------------------------
	//! Changes when there is something new to draw.
	protected int DataVersion()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOpened()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void Redraw()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		m_Click = new RZ_MenuClick(this);
		RZ_UIW.OnClick(m_wRoot, "CloseButton", m_Click);
		OnOpened();
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse (RZ_MenuClick): the close button here, the rest in OnClickOther.
	void OnWidgetClick(Widget w)
	{
		string name = w.GetName();
		if (name == "CloseButton")
		{
			Close();
			return;
		}

		OnClickOther(w, name);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickOther(Widget w, string name)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		m_fPoll -= tDelta;
		if (m_fPoll > 0)
			return;

		m_fPoll = POLL;
		int v = DataVersion();
		if (v == m_iSeen)
			return;

		m_iSeen = v;
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack(float value = 0, EActionTrigger reason = 0)
	{
		Back();
	}

	//------------------------------------------------------------------------------------------------
	//! Esc or B. The talk screen leaves the conversation instead.
	protected void Back()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	//! A payload's records, each split into fields.
	static void Records(string payload, notnull array<ref array<string>> records)
	{
		records.Clear();
		array<string> lines = {};
		RZ_UIText.Split(payload, "\n", lines);
		foreach (string ln : lines)
		{
			if (ln.IsEmpty())
				continue;

			array<string> f = {};
			RZ_UIText.Split(ln, RZ_UIText.SEP, f);
			records.Insert(f);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! The notice board at the safehouse you're at (World/RZ_NoticeBoard.c): the notes, newest first,
//! and a box to write one (NoteInput, PostButton). Opened by the server when the board is read
//! (UI > Board menus); without it the board is a notice and notes are written with /note. Not a
//! notebook tab: Esc, B or the close button close it.
class RZ_NoticeBoardMenu : RZ_MenuBase
{
	protected static bool s_bOpen;
	protected int m_iPreset = -1;			// the ready-made note in the box, -1 none yet
	protected bool m_bFocusSet;				// the first redraw gave the focus (PIN IT UP when you can write)
	protected bool m_bCanClear;				// a server admin: X / Delete clears this board
	protected bool m_bClearPrompt;

	//------------------------------------------------------------------------------------------------
	//! Ready-made notes, so a pad (or anyone) can pin one up without typing (docs/UI.md, "Controllers").
	//! Left and right put one in the box; the box can still be edited with a keyboard.
	static void GetPresets(notnull array<string> result)
	{
		result.Insert("Gone out for food. Back before dark.");
		result.Insert("Looking for my family. Ask for me at the door.");
		result.Insert("The east streets are thick with them. Go round.");
		result.Insert("Need medicine. Will trade for it.");
		result.Insert("Slept here safe. Thank you, whoever keeps this place.");
		result.Insert("Car by the road out of town. Runs, just. Take it if you need it.");
		result.Insert("If you see a girl in a red coat, tell her her dad is here.");
	}

	//------------------------------------------------------------------------------------------------
	static bool IsBoardOpen()
	{
		return s_bOpen;
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return 0;

		return ui.GetBoardVersion();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_bOpen = true;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuLeft", EActionTrigger.DOWN, OnPresetPrev);
		input.AddActionListener("MenuRight", EActionTrigger.DOWN, OnPresetNext);
		input.AddActionListener("RZ_BoardClear", EActionTrigger.DOWN, OnClear);
		RZ_UIW.OnClick(m_wRoot, "PostButton", m_Click);
		RZ_UIW.Focus(m_wRoot, "CloseButton");
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Pin it up");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuRight", "Ready-made note") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuDown", "Scroll") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_BOARD);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		s_bOpen = false;
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnPresetPrev);
		input.RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnPresetNext);
		input.RemoveActionListener("RZ_BoardClear", EActionTrigger.DOWN, OnClear);
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! Pad: up and down scroll the notes.
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.Scroll(m_wRoot, "NotesScroll", -1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.Scroll(m_wRoot, "NotesScroll", 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Left / right: the previous or next ready-made note into the box. Not while the box is being
	//! typed in (the keyboard's arrows move the cursor there).
	protected void OnPresetPrev(float value = 0, EActionTrigger reason = 0)
	{
		StepPreset(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPresetNext(float value = 0, EActionTrigger reason = 0)
	{
		StepPreset(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void StepPreset(int dir)
	{
		Widget box = RZ_UIW.Find(m_wRoot, "NoteInput");
		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!box || !box.IsVisible() || (ws && ws.GetFocusedWidget() == box))
			return;

		array<string> presets = {};
		GetPresets(presets);
		int count = presets.Count();
		if (m_iPreset < 0 && dir < 0)
			m_iPreset = 0;

		m_iPreset = (m_iPreset + dir + count) % count;
		RZ_UIW.SetEditText(m_wRoot, "NoteInput", presets[m_iPreset]);
	}

	//------------------------------------------------------------------------------------------------
	//! X / Delete (server admins): clear this board, what "/noteclear" does, without typing.
	protected void OnClear(float value = 0, EActionTrigger reason = 0)
	{
		if (!m_bCanClear)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCommand(RZ_PlayerUIComponent.CMD_CLEAR_BOARD, "");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClickOther(Widget w, string name)
	{
		if (name != "PostButton")
			return;

		// VERIFY: on a console the note box (an EditBoxWidget) opens the platform's on-screen keyboard;
		// the ready-made notes (left and right) need no keyboard either way
		string text = RZ_UIW.GetEditText(m_wRoot, "NoteInput");
		if (text.IsEmpty())
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCommand(RZ_PlayerUIComponent.CMD_NOTE, text);

		RZ_UIW.SetEditText(m_wRoot, "NoteInput", "");
	}

	//------------------------------------------------------------------------------------------------
	//! B|safehouse|notes|longest note|can write|can clear, then N|author|age|text newest first
	override protected void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		array<ref array<string>> recs = {};
		Records(ui.GetBoard(), recs);
		string notes = "";
		bool canWrite = false;
		int limit = 160;
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "B")
			{
				string head = "NOTICE BOARD: " + RZ_UIText.Str(f, 1);
				head.ToUpper();
				RZ_UIW.SetText(m_wRoot, "Header", head);
				limit = RZ_UIText.Int(f, 3);
				canWrite = RZ_UIText.Int(f, 4) == 1;
				m_bCanClear = RZ_UIText.Int(f, 5) == 1;
			}
			else if (tag == "N")
			{
				notes += string.Format("%1, %2\n%3\n\n", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
			}
		}

		RZ_UIW.Show(m_wRoot, "Empty", notes.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "Notes", notes);
		RZ_UIW.Show(m_wRoot, "WriteBlock", canWrite);
		RZ_UIW.SetText(m_wRoot, "NoteLimit", string.Format("%1 characters at most. Everyone who comes by can read it. Left and right: a ready-made note.", limit));

		// The pad's way in: A pins up what's in the box (a ready-made note needs no typing)
		if (!m_bFocusSet)
		{
			m_bFocusSet = true;
			if (canWrite)
				RZ_UIW.Focus(m_wRoot, "PostButton");
		}

		if (m_bCanClear && !m_bClearPrompt)
		{
			m_bClearPrompt = true;
			RZ_UIW.AddPrompt(m_wRoot, "Prompts", "RZ_BoardClear", "Clear board (admin)");
		}
	}
}
