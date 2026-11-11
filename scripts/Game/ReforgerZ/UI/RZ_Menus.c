//! The ReforgerZ menus (docs/UI.md, "Screens"): the shared base, and the safehouse notice board.
//! The notebook (Standing, Journal, Radio, Map key, Hall of fame as tabs) is in RZ_Notebook.c, the
//! talk screen in RZ_TalkMenu.c and the reader in RZ_Reader.c. Layouts and widget names are in
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
	RZ_TalkMenu
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
		GetGame().GetInputManager().AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		GetGame().GetInputManager().AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		RZ_UIW.OnClick(m_wRoot, "PostButton", m_Click);
		RZ_UIW.Focus(m_wRoot, "CloseButton");
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_BOARD);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		s_bOpen = false;
		GetGame().GetInputManager().RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		GetGame().GetInputManager().RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
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
	override protected void OnClickOther(Widget w, string name)
	{
		if (name != "PostButton")
			return;

		string text = RZ_UIW.GetEditText(m_wRoot, "NoteInput");
		if (text.IsEmpty())
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCommand(RZ_PlayerUIComponent.CMD_NOTE, text);

		RZ_UIW.SetEditText(m_wRoot, "NoteInput", "");
	}

	//------------------------------------------------------------------------------------------------
	//! B|safehouse|notes|longest note|can write, then N|author|age|text newest first
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
			}
			else if (tag == "N")
			{
				notes += string.Format("%1, %2\n%3\n\n", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
			}
		}

		RZ_UIW.Show(m_wRoot, "Empty", notes.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "Notes", notes);
		RZ_UIW.Show(m_wRoot, "WriteBlock", canWrite);
		RZ_UIW.SetText(m_wRoot, "NoteLimit", string.Format("%1 characters at most. Everyone who comes by can read it.", limit));
	}
}
