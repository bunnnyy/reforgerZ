//! The talk screen (docs/UI.md, "The talk screen"), New Vegas style: the person stays in view and
//! a compact panel sits in the lower middle of the screen, below their head and chest, with a top
//! border in their faction colour; their name and "The Haven - council head - knows your name";
//! what they say; your replies as a numbered list of at most 5, the selected one lighter with an
//! accent bar, a dim tag on the right, what a reply needs when it can't be picked, grey when there's
//! nothing new behind it (it still works); the last is always the way out. One line of prompts
//! closes the panel.
//!
//! Keys: up/down or 1-5, Enter; mouse: hover lights a reply, a click says it; Esc leaves. Pad: D-pad
//! or left stick, A says it, B leaves. A list longer than the screen comes a page at a time (4 and
//! the way out): the page shows as "1/2" with "MORE >" while there are more, and LB/RB (Q/E, or a
//! click on the arrows) turn it. The right stick (or the mouse wheel) scrolls what they say when it's
//! long. The server keeps a server admin's replies under [Admin]. You can't move while it's open; the
//! world keeps running and danger closes it (the server's screen guard).

//------------------------------------------------------------------------------------------------
//! Mouse on a reply row: the cursor coming onto it selects it, a click says it.
class RZ_TalkRowEvents : ScriptedWidgetEventHandler
{
	protected RZ_TalkMenu m_Menu;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkRowEvents(RZ_TalkMenu menu)
	{
		m_Menu = menu;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_Menu || !w)
			return false;

		m_Menu.SayRow(RZ_UIW.GetTag(w));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: OnMouseEnter fires on the row's Button (its children are images and text) when the
	//! cursor comes onto it, in a ChimeraMenuBase.
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_Menu && w)
			m_Menu.HoverRow(RZ_UIW.GetTag(w));

		return false;
	}
}

//------------------------------------------------------------------------------------------------
//! Mouse on the page arrows: a click turns the page.
class RZ_TalkPageEvents : ScriptedWidgetEventHandler
{
	protected RZ_TalkMenu m_Menu;

	//------------------------------------------------------------------------------------------------
	void RZ_TalkPageEvents(RZ_TalkMenu menu)
	{
		m_Menu = menu;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_Menu || !w)
			return false;

		if (w.GetName() == "PagePrev")
			m_Menu.TurnPage(-1);
		else
			m_Menu.TurnPage(1);

		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_TalkMenu : RZ_MenuBase
{
	protected static const int ROWS = 5;	// replies on the screen at once (RZ_Dialogue.ROWS)
	protected static const float SCROLL_SPEED = 1.5;	// what they say: share of the text a second at full stick

	protected static RZ_TalkMenu s_Open;
	protected static string s_sNode;
	protected static int s_iVersion;
	protected static bool s_bFailed;		// the menu couldn't open here: the actions come back

	protected int m_iSelected;
	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected bool m_bDone;					// left, or closed by the server: nothing more to send
	protected ref Color m_Accent;
	protected ref RZ_TalkRowEvents m_RowEvents;
	protected ref RZ_TalkPageEvents m_PageEvents;
	protected int m_iPage;					// this list's page, from 0 (the server's)
	protected int m_iPages = 1;
	protected ref array<int> m_aIds = {};
	protected ref array<bool> m_aEnabled = {};
	protected ref array<bool> m_aHeard = {};
	protected ref array<ref array<string>> m_aReplies = {};
	protected ref array<string> m_aHead = {};
	protected ref array<Widget> m_aRows = {};

	//------------------------------------------------------------------------------------------------
	//! The talk screen is used on this machine: Dialogue > Talk screen on, UI > Talk reply layout
	//! set (the server checks the same two), and it hasn't failed to open here.
	static bool Available()
	{
		return !s_bFailed && RZ_DialogueSettings.Get().m_bTalkScreen && !RZ_UISettings.Get().m_sTalkReplyLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! A person's choice and use actions hide while the talk screen is used: they're its replies.
	static bool HidesSlotsOn(IEntity owner)
	{
		return Available() && RZ_Dialogue.IsPerson(owner);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A talk payload (RZ_PlayerUIComponent): a node opens or redraws the screen; X|reason closes it.
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

		s_sNode = payload;
		s_iVersion++;
		if (s_Open || RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_TalkMenu))
			return;

		// No preset or layout here: say it as a card, leave, and give the actions back
		s_bFailed = true;
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		if (recs.IsEmpty())
			return;

		array<string> head = recs[0];
		ui.SendTalkPick(RZ_UIText.Int(head, 1), RZ_Dialogue.LEAVE);
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
			gm.ShowNotice(RZ_UIText.Str(head, 3), Unbreak(RZ_UIText.Str(head, 5)), RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	static string Unbreak(string said)
	{
		string s = said;
		s.Replace("^", "\n");
		return s;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		m_RowEvents = new RZ_TalkRowEvents(this);
		m_PageEvents = new RZ_TalkPageEvents(this);
		RZ_UIW.OnClick(m_wRoot, "PagePrev", m_PageEvents);
		RZ_UIW.OnClick(m_wRoot, "PageNext", m_PageEvents);
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnPageBack);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnPageOn);
		input.AddActionListener("RZ_Reply1", EActionTrigger.DOWN, OnReply1);
		input.AddActionListener("RZ_Reply2", EActionTrigger.DOWN, OnReply2);
		input.AddActionListener("RZ_Reply3", EActionTrigger.DOWN, OnReply3);
		input.AddActionListener("RZ_Reply4", EActionTrigger.DOWN, OnReply4);
		input.AddActionListener("RZ_Reply5", EActionTrigger.DOWN, OnReply5);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Say it");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Leave") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "Page") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnPageBack);
		input.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnPageOn);
		input.RemoveActionListener("RZ_Reply1", EActionTrigger.DOWN, OnReply1);
		input.RemoveActionListener("RZ_Reply2", EActionTrigger.DOWN, OnReply2);
		input.RemoveActionListener("RZ_Reply3", EActionTrigger.DOWN, OnReply3);
		input.RemoveActionListener("RZ_Reply4", EActionTrigger.DOWN, OnReply4);
		input.RemoveActionListener("RZ_Reply5", EActionTrigger.DOWN, OnReply5);
		if (s_Open == this)
			s_Open = null;

		// Esc, B, the close button, or another menu on top: the conversation ends
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendTalkPick(m_iSerial, RZ_Dialogue.LEAVE);

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! A new node from the server. A new selection only restyles the rows (Select).
	override protected int DataVersion()
	{
		return s_iVersion;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		Select(Math.Max(0, m_iSelected - 1));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		Select(Math.Min(Math.Max(0, m_aIds.Count() - 1), m_iSelected + 1));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		Say(m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPageBack(float value = 0, EActionTrigger reason = 0)
	{
		TurnPage(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPageOn(float value = 0, EActionTrigger reason = 0)
	{
		TurnPage(1);
	}

	//------------------------------------------------------------------------------------------------
	//! LB/RB, Q/E or a click on the arrows: the page on (dir 1) or back (-1), round to the other end.
	//! The server sends the new page; with one page nothing happens.
	void TurnPage(int dir)
	{
		if (m_bDone || m_iPages <= 1)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		if (dir < 0)
			ui.SendTalkPick(m_iSerial, RZ_Dialogue.PAGE_BACK);
		else
			ui.SendTalkPick(m_iSerial, RZ_Dialogue.MORE);
	}

	//------------------------------------------------------------------------------------------------
	//! The right stick scrolls what they say when it's long (RZ_TalkScroll), every frame it's held.
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		float stick = GetGame().GetInputManager().GetActionValue("RZ_TalkScroll");
		if (Math.AbsFloat(stick) < 0.2)
			return;

		ScrollLayoutWidget said = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "SaidScroll"));
		if (!said)
			return;

		float x, y;
		said.GetSliderPos(x, y);
		said.SetSliderPos(x, Math.Clamp(y - stick * tDelta * SCROLL_SPEED, 0, 1)); // VERIFY: the stick reads + when pushed up, so up scrolls toward the top
	}

	//------------------------------------------------------------------------------------------------
	protected void OnReply1(float value = 0, EActionTrigger reason = 0) { Say(0); }
	protected void OnReply2(float value = 0, EActionTrigger reason = 0) { Say(1); }
	protected void OnReply3(float value = 0, EActionTrigger reason = 0) { Say(2); }
	protected void OnReply4(float value = 0, EActionTrigger reason = 0) { Say(3); }
	protected void OnReply5(float value = 0, EActionTrigger reason = 0) { Say(4); }

	//------------------------------------------------------------------------------------------------
	//! Esc or B: the way out (the last reply: I'll be going., or Back. in the [Admin] list).
	override protected void Back()
	{
		Say(m_aIds.Count() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a reply row (RZ_TalkRowEvents) says it.
	void SayRow(int index)
	{
		Say(index);
	}

	//------------------------------------------------------------------------------------------------
	//! The cursor on a reply row lights it, as the D-pad would.
	void HoverRow(int index)
	{
		if (index >= 0 && index < m_aIds.Count() && index != m_iSelected)
			Select(index);
	}

	//------------------------------------------------------------------------------------------------
	protected void Select(int index)
	{
		if (index == m_iSelected)
			return;

		m_iSelected = index;
		Style();
	}

	//------------------------------------------------------------------------------------------------
	//! Pick reply `index` (it must be enabled); Leave closes the screen.
	protected void Say(int index)
	{
		if (index < 0 || index >= m_aIds.Count() || !m_aEnabled[index])
		{
			if (index >= 0 && index < m_aIds.Count())
				Select(index);

			return;
		}

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendTalkPick(m_iSerial, m_aIds[index]);

		if (m_aIds[index] == RZ_Dialogue.LEAVE)
		{
			m_bDone = true;
			Close();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! T|serial|faction|name|sub|said|page|pages, then R|id|label|tag|need|enabled|heard.
	override protected void Redraw()
	{
		if (m_iParsed == s_iVersion)
		{
			Style();
			return;
		}

		m_iParsed = s_iVersion;
		Parse();
		int faction = RZ_UIText.Int(m_aHead, 2);
		m_Accent = RZ_UIColor.Bone();
		if (faction >= 0 && !RZ_UIText.Str(m_aHead, 2).IsEmpty())
			m_Accent = RZ_UIColor.Faction(faction);

		RZ_UIW.Tint(m_wRoot, "TalkAccent", m_Accent);
		RZ_UIW.SetText(m_wRoot, "TalkName", RZ_UIText.Str(m_aHead, 3));
		RZ_UIW.Tint(m_wRoot, "TalkName", m_Accent);
		RZ_UIW.SetText(m_wRoot, "TalkSub", RZ_UIText.Str(m_aHead, 4));
		RZ_UIW.SetText(m_wRoot, "TalkSaid", Unbreak(RZ_UIText.Str(m_aHead, 5)));
		ShowPage();
		BuildRows();
		Style();
	}

	//------------------------------------------------------------------------------------------------
	//! "1/2" and the arrows when the list has more than one page: "<" while there's a page before,
	//! "MORE >" while there's one after (LB/RB still turn round from either end).
	protected void ShowPage()
	{
		m_iPage = Math.Max(0, RZ_UIText.Int(m_aHead, 6));
		m_iPages = Math.Max(1, RZ_UIText.Int(m_aHead, 7));
		bool paged = m_iPages > 1;
		RZ_UIW.Show(m_wRoot, "TalkPage", paged);
		if (!paged)
			return;

		int shown = m_iPage + 1;
		RZ_UIW.SetText(m_wRoot, "PageText", string.Format("%1/%2", shown, m_iPages));
		RZ_UIW.Show(m_wRoot, "PagePrev", m_iPage > 0);
		RZ_UIW.Show(m_wRoot, "PageNext", m_iPage < m_iPages - 1);
		RZ_UIW.Tint(m_wRoot, "PageNextText", m_Accent);
	}

	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sNode, recs);
		m_aHead.Clear();
		m_aIds.Clear();
		m_aEnabled.Clear();
		m_aHeard.Clear();
		m_aReplies.Clear();
		array<ref array<string>> replies = {};
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "T")
			{
				m_aHead.Copy(f);
				m_iSerial = RZ_UIText.Int(f, 1);
			}
			else if (tag == "R")
			{
				replies.Insert(f);
			}
		}

		// The server sends at most 5; more would be cut to the first 4 and the way out
		while (replies.Count() > ROWS)
		{
			replies.RemoveOrdered(ROWS - 1);
		}

		foreach (array<string> rec : replies)
		{
			m_aReplies.Insert(rec);
			m_aIds.Insert(RZ_UIText.Int(rec, 1));
			m_aEnabled.Insert(RZ_UIText.Int(rec, 5) == 1);
			m_aHeard.Insert(RZ_UIText.Int(rec, 6) == 1);
		}

		// A new node starts on its first reply that can be picked and has something new, else the
		// first that can be picked; what they say starts at the top
		m_iSelected = -1;
		foreach (int i, bool on : m_aEnabled)
		{
			if (on && !m_aHeard[i])
			{
				m_iSelected = i;
				break;
			}
		}

		if (m_iSelected < 0)
			m_iSelected = Math.Max(0, m_aEnabled.Find(true));

		ScrollLayoutWidget said = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "SaidScroll"));
		if (said)
			said.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! The rows, made once per node; what changes with the selection is in Style.
	protected void BuildRows()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Replies");
		RZ_UIW.ClearChildren(list);
		m_aRows.Clear();
		ResourceName rowLayout = RZ_UISettings.Get().m_sTalkReplyLayout;
		foreach (int i, array<string> f : m_aReplies)
		{
			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				return;

			bool enabled = m_aEnabled[i];
			string need = RZ_UIText.Str(f, 4);
			int num = i + 1;
			RZ_UIW.SetText(row, "ReplyNum", num.ToString());
			RZ_UIW.SetText(row, "ReplyText", RZ_UIText.Str(f, 2));
			RZ_UIW.SetText(row, "ReplyTag", RZ_UIText.Str(f, 3));
			RZ_UIW.Show(row, "ReplyTag", enabled && !RZ_UIText.Str(f, 3).IsEmpty());
			RZ_UIW.SetText(row, "ReplyNeed", "(" + need + ")");
			RZ_UIW.Show(row, "ReplyNeed", !enabled && !need.IsEmpty());
			RZ_UIW.Tint(row, "ReplyBar", m_Accent);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_RowEvents);
			m_aRows.Insert(row);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The selected row (the D-pad's, the keys' or the cursor's) lighter with the accent bar; a reply
	//! with nothing new behind it grey, one that can't be picked faint.
	protected void Style()
	{
		foreach (int i, Widget row : m_aRows)
		{
			bool on = i == m_iSelected;
			RZ_UIW.Show(row, "ReplyBg", on);
			RZ_UIW.Show(row, "ReplyBar", on);
			Color fg = RZ_UIColor.Bone();
			Color num = m_Accent;
			if (!m_aEnabled[i])
			{
				fg = RZ_UIW.Rgb(110, 106, 96);
				num = fg;
			}
			else if (m_aHeard[i])
			{
				fg = RZ_UIW.Rgb(128, 124, 114);
				if (on)
					fg = RZ_UIW.Rgb(170, 165, 152);

				num = fg;
			}
			else if (on)
			{
				fg = RZ_UIW.Rgb(244, 240, 230);
			}

			RZ_UIW.Tint(row, "ReplyText", fg);
			RZ_UIW.Tint(row, "ReplyNum", num);
		}

		// Pad focus on the selected reply, so the D-pad and A act on what's lit
		if (m_iSelected >= 0 && m_iSelected < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[m_iSelected]);
	}
}
