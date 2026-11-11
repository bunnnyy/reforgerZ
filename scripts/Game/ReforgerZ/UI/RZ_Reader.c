//! The reader (docs/UI.md, "The reader"): notes, letters, orders and flyers open on a sheet of
//! paper in the middle of the screen, a page at a time. You can't move while it's open; danger
//! closes it (UI/RZ_ScreenGuard.c). The text only ever goes to its player.

//------------------------------------------------------------------------------------------------
//! Server: show a readable thing to one player.
class RZ_Reader
{
	protected static const int MAX_TEXT = 5600;

	//------------------------------------------------------------------------------------------------
	//! title: what it is ("Note on the dashboard"); how: one line on how it looks ("Folded twice,
	//! in pencil"), or ""; text: what it says (paragraphs split by line breaks). With **UI > Reader**
	//! off, no UI component on the player's controller, or the server at its cap of open screens,
	//! it's a notice card instead, as before.
	static void Show(RZ_GameModeComponent gm, int playerId, string title, string how, string text)
	{
		if (!gm)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = gm.GetScreenGuard();
		if (!ui || !guard || !RZ_UISettings.Get().m_bReader || !guard.Open(playerId, RZ_ScreenGuard.KIND_READER, null, "", -1))
		{
			gm.NotifyPlayer(playerId, title, text);
			return;
		}

		string body = text;
		if (body.Length() > MAX_TEXT)
			body = body.Substring(0, MAX_TEXT - 3) + "...";

		string head = "P" + RZ_UIText.SEP + RZ_UIText.Clean(title, 120) + RZ_UIText.SEP + RZ_UIText.Clean(how, 160);
		ui.SendPayload(RZ_UIFeed.PAYLOAD_READER, head + "\n" + body);
	}

	//------------------------------------------------------------------------------------------------
	//! A read label that means paper: "Read the note", "Read the flyer" (not "Look at", "Listen",
	//! "Search", "Talk to").
	static bool IsPaperLabel(string label)
	{
		string l = label;
		l.ToLower();
		return l.IndexOf("read") == 0;
	}
}

//------------------------------------------------------------------------------------------------
//! The reader's screen (client). Opened by a PAYLOAD_READER; an empty one from the server closes
//! it. LB/RB or Q/E (or the arrows) turn the page, B or Esc closes.
class RZ_ReaderMenu : RZ_MenuBase
{
	protected static RZ_ReaderMenu s_Open;

	protected ref array<string> m_aPages = {};
	protected int m_iPage;
	protected int m_iParsed = -1;
	protected bool m_bServerClosed;

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The server closed it (danger): no need to tell the server back.
	static void CloseFromServer()
	{
		if (!s_Open)
			return;

		s_Open.m_bServerClosed = true;
		s_Open.Close();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		GetGame().GetInputManager().AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnPrev);
		GetGame().GetInputManager().AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnNext);
		GetGame().GetInputManager().AddActionListener("MenuLeft", EActionTrigger.DOWN, OnPrev);
		GetGame().GetInputManager().AddActionListener("MenuRight", EActionTrigger.DOWN, OnNext);
		RZ_UIW.OnClick(m_wRoot, "PrevButton", m_Click);
		RZ_UIW.OnClick(m_wRoot, "NextButton", m_Click);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabLeft", "");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "Pages") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		RZ_UIW.Focus(m_wRoot, "CloseButton");
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnPrev);
		GetGame().GetInputManager().RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnNext);
		GetGame().GetInputManager().RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnPrev);
		GetGame().GetInputManager().RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnNext);
		if (s_Open == this)
			s_Open = null;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bServerClosed)
			ui.SendScreenClosed(RZ_ScreenGuard.KIND_READER);

		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "PrevButton")
			OnPrev();
		else if (name == "NextButton")
			OnNext();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrev(float value = 0, EActionTrigger reason = 0)
	{
		m_iPage = Math.Max(0, m_iPage - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNext(float value = 0, EActionTrigger reason = 0)
	{
		m_iPage = Math.Min(Math.Max(0, m_aPages.Count() - 1), m_iPage + 1);
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return m_iPage;

		return ui.GetReaderVersion() * 100 + m_iPage;
	}

	//------------------------------------------------------------------------------------------------
	//! P|title|how, then the text (docs/UI.md, "The reader").
	override protected void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		string payload = ui.GetReader();
		int nl = payload.IndexOf("\n");
		string head = payload;
		string text = "";
		if (nl >= 0)
		{
			head = payload.Substring(0, nl);
			text = payload.Substring(nl + 1, payload.Length() - nl - 1);
		}

		array<string> f = {};
		RZ_UIText.Split(head, RZ_UIText.SEP, f);
		if (m_iParsed != ui.GetReaderVersion())
		{
			m_iParsed = ui.GetReaderVersion();
			RZ_UIText.Paginate(text, RZ_UISettings.Get().m_iReaderPageChars, m_aPages);
			m_iPage = 0;
		}

		int count = Math.Max(1, m_aPages.Count());
		m_iPage = Math.Clamp(m_iPage, 0, count - 1);
		string how = RZ_UIText.Str(f, 2);
		RZ_UIW.SetText(m_wRoot, "PaperTitle", RZ_UIText.Str(f, 1));
		RZ_UIW.SetText(m_wRoot, "PaperHow", how);
		RZ_UIW.Show(m_wRoot, "PaperHow", !how.IsEmpty());
		string page = "";
		if (!m_aPages.IsEmpty())
			page = m_aPages[m_iPage];

		RZ_UIW.SetText(m_wRoot, "PaperText", page);
		RZ_UIW.SetText(m_wRoot, "PaperPages", string.Format("%1 / %2", m_iPage + 1, count));
		RZ_UIW.Show(m_wRoot, "PrevButton", m_iPage > 0);
		RZ_UIW.Show(m_wRoot, "NextButton", m_iPage < count - 1);
	}
}
