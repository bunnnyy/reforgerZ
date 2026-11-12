//! The talk screen (docs/UI.md, "The talk screen"), New Vegas style, in two steps. The person stays
//! in view; their name and sub-line sit at the top right in their faction colour.
//!
//! 1. Their line on its own, in a subtitle band in the lower middle, with a thin bar that fills over
//!    the reading time (READ_PER_CHAR a character, READ_MIN to READ_MAX). A, Space or a click skips
//!    straight to the replies; nothing needs pressing. The right stick or the wheel scrolls a long
//!    line (the stick holds the bar while it's pushed).
//! 2. Then your replies on their own: the band becomes an outlined box in the faction colour, the
//!    selected reply with a fill and an outline, check tags in the faction colour, a reply you can't
//!    take faint with what it needs, one with nothing new behind it grey. Arrows at the left show
//!    there are more pages, "1/2" sits bottom right; LB/RB, Q/E or a click on an arrow turn the
//!    page. The last reply is always the way out.
//! A reply you say fades and their answer takes the band: back to step 1.
//!
//! Keys: up/down or 1-5, Enter; mouse: hover lights a reply, a click says it; Esc leaves. Pad: D-pad
//! or left stick, A says it, B leaves (from either step). The server sends the line and the replies
//! together (PAYLOAD_TALK); the two steps are this client's. The server keeps a server admin's
//! replies under [Admin]. You can't move while it's open; the world keeps running and danger closes
//! it (the server's screen guard).

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
	protected static const float READ_PER_CHAR = 0.05;	// their line's reading time: seconds a character
	protected static const float READ_MIN = 2.5;
	protected static const float READ_MAX = 9;
	protected static const float CLICK_GUARD = 0.3;	// a click this soon after the replies show is the skip's own
	protected static const float FADE_TIME = 0.35;	// a reply you said fades over this
	protected static const float FADE_TO = 0.3;
	protected static const float ANSWER_WAIT = 2;	// no answer by then (the server didn't take it): the replies come back
	protected static const float FILL_ALPHA = 0.16;	// the selected reply's fill, in the faction colour

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

	// The two steps
	protected bool m_bLine;					// step 1: their line on its own; else step 2, the replies
	protected float m_fRead;				// this line's reading time
	protected float m_fReadLeft;
	protected string m_sLastSaid;
	protected bool m_bSaid;					// a reply was said: the next node is their answer
	protected float m_fSaidAge = -1;		// seconds since it was said (the fade); -1 when nothing waits
	protected int m_iSaidRow = -1;
	protected float m_fRepliesAt;			// when the replies showed (RZ_UIW.Now)

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
	//! Their line's reading time: READ_PER_CHAR a character, READ_MIN to READ_MAX seconds.
	static float ReadTime(string said)
	{
		return Math.Clamp(said.Length() * READ_PER_CHAR, READ_MIN, READ_MAX);
	}

	//------------------------------------------------------------------------------------------------
	//! A reply's tag as it shows, in brackets: "[Persuade - Haven - 1 of 2]", "[a job]".
	static string TagText(string tag)
	{
		if (tag.IsEmpty() || tag.StartsWith("["))
			return tag;

		return "[" + tag + "]";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		m_RowEvents = new RZ_TalkRowEvents(this);
		m_PageEvents = new RZ_TalkPageEvents(this);
		RZ_UIW.OnClick(m_wRoot, "PagePrev", m_PageEvents);
		RZ_UIW.OnClick(m_wRoot, "PageNext", m_PageEvents);
		RZ_UIW.Rotate(m_wRoot, "PageNextIcon", 180);	// the same arrow as PagePrevIcon, pointing down
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnPageBack);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnPageOn);
		input.AddActionListener("RZ_TalkSkip", EActionTrigger.DOWN, OnSkip);
		input.AddActionListener("RZ_Reply1", EActionTrigger.DOWN, OnReply1);
		input.AddActionListener("RZ_Reply2", EActionTrigger.DOWN, OnReply2);
		input.AddActionListener("RZ_Reply3", EActionTrigger.DOWN, OnReply3);
		input.AddActionListener("RZ_Reply4", EActionTrigger.DOWN, OnReply4);
		input.AddActionListener("RZ_Reply5", EActionTrigger.DOWN, OnReply5);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuUp", "Choose");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Say it") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "Page") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Leave") && prompts;
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
		input.RemoveActionListener("RZ_TalkSkip", EActionTrigger.DOWN, OnSkip);
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
	//! The replies take input only in step 2, and not while a reply you said waits for its answer.
	protected bool Choosing()
	{
		return !m_bLine && m_fSaidAge < 0 && !m_bDone;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		if (Choosing())
			Select(Math.Max(0, m_iSelected - 1));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		if (Choosing())
			Select(Math.Min(Math.Max(0, m_aIds.Count() - 1), m_iSelected + 1));
	}

	//------------------------------------------------------------------------------------------------
	//! A or Enter: skips their line in step 1, says the selected reply in step 2.
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bLine)
		{
			ShowReplies();
			return;
		}

		if (RZ_UIW.Now() - m_fRepliesAt >= CLICK_GUARD) // the same A press may have skipped the line (RZ_TalkSkip)
			Say(m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	//! Space or a click (RZ_TalkSkip): skips their line; nothing in step 2.
	protected void OnSkip(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bLine)
			ShowReplies();
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
	//! The server sends the new page; with one page, or in step 1, nothing happens.
	void TurnPage(int dir)
	{
		if (!Choosing() || m_iPages <= 1)
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
	//! Every frame: step 1's reading time and its bar, the right stick on a long line, and the fade
	//! of a reply you said.
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		if (m_bLine)
			UpdateLine(tDelta);
		else if (m_fSaidAge >= 0)
			UpdateSaid(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLine(float tDelta)
	{
		float stick = GetGame().GetInputManager().GetActionValue("RZ_TalkScroll");
		if (Math.AbsFloat(stick) >= 0.2)
		{
			// Reading a long line: the stick scrolls it and holds the bar
			ScrollLayoutWidget said = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "SaidScroll"));
			if (said)
			{
				float x, y;
				said.GetSliderPos(x, y);
				said.SetSliderPos(x, Math.Clamp(y - stick * tDelta * SCROLL_SPEED, 0, 1)); // VERIFY: the stick reads + when pushed up, so up scrolls toward the top
			}

			return;
		}

		m_fReadLeft -= tDelta;
		if (m_fReadLeft <= 0)
		{
			ShowReplies();
			return;
		}

		RZ_UIW.Progress(m_wRoot, "LineBar", 1 - m_fReadLeft / m_fRead);
	}

	//------------------------------------------------------------------------------------------------
	//! The reply you said fades while the answer comes; if none comes, the replies come back.
	protected void UpdateSaid(float tDelta)
	{
		m_fSaidAge += tDelta;
		if (m_fSaidAge > ANSWER_WAIT)
		{
			m_fSaidAge = -1;
			m_bSaid = false;
			ShowReplies();
			return;
		}

		if (m_iSaidRow >= 0 && m_iSaidRow < m_aRows.Count())
			RZ_UIW.Fade(m_aRows[m_iSaidRow], 1 - (1 - FADE_TO) * Math.Min(1, m_fSaidAge / FADE_TIME));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnReply1(float value = 0, EActionTrigger reason = 0) { SayKey(0); }
	protected void OnReply2(float value = 0, EActionTrigger reason = 0) { SayKey(1); }
	protected void OnReply3(float value = 0, EActionTrigger reason = 0) { SayKey(2); }
	protected void OnReply4(float value = 0, EActionTrigger reason = 0) { SayKey(3); }
	protected void OnReply5(float value = 0, EActionTrigger reason = 0) { SayKey(4); }

	//------------------------------------------------------------------------------------------------
	//! 1-5 (LT/RT + D-pad): a reply at once, in step 2 only.
	protected void SayKey(int index)
	{
		if (!m_bLine)
			Say(index);
	}

	//------------------------------------------------------------------------------------------------
	//! Esc or B, from either step: the way out (the last reply: I'll be going., or Back. in the
	//! [Admin] list).
	override protected void Back()
	{
		if (m_bDone)
			return;

		int last = m_aIds.Count() - 1;
		if (last < 0)
		{
			Close();
			return;
		}

		m_fSaidAge = -1;
		Say(last);
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a reply row (RZ_TalkRowEvents) says it in step 2, but not the click that skipped
	//! the line.
	void SayRow(int index)
	{
		if (!m_bLine && RZ_UIW.Now() - m_fRepliesAt >= CLICK_GUARD)
			Say(index);
	}

	//------------------------------------------------------------------------------------------------
	//! The cursor on a reply row lights it, as the D-pad would.
	void HoverRow(int index)
	{
		if (Choosing() && index >= 0 && index < m_aIds.Count() && index != m_iSelected)
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
	//! Pick reply `index` (it must be enabled); Leave closes the screen. A reply that's said (not the
	//! screen's own [Admin] and Back.) fades, the others go, and their answer comes as step 1.
	protected void Say(int index)
	{
		if (m_fSaidAge >= 0 || m_bDone)
			return;

		if (index < 0 || index >= m_aIds.Count() || !m_aEnabled[index])
		{
			if (index >= 0 && index < m_aIds.Count())
				Select(index);

			return;
		}

		int id = m_aIds[index];
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendTalkPick(m_iSerial, id);

		if (id == RZ_Dialogue.LEAVE)
		{
			m_bDone = true;
			Close();
			return;
		}

		if (id == RZ_Dialogue.ADMIN || id == RZ_Dialogue.ADMIN_BACK)
			return;

		m_bSaid = true;
		m_fSaidAge = 0;
		m_iSaidRow = index;
		foreach (int i, Widget row : m_aRows)
		{
			if (i != index)
				RZ_UIW.Fade(row, 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! T|serial|faction|name|sub|said|page|pages, then R|id|label|tag|need|enabled|heard. Their answer
	//! to a reply you said, or a new line, starts at step 1; a page turn or the [Admin] list stays on
	//! the replies.
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

		RZ_UIW.SetText(m_wRoot, "TalkName", RZ_UIText.Str(m_aHead, 3));
		RZ_UIW.Tint(m_wRoot, "TalkName", m_Accent);
		RZ_UIW.SetText(m_wRoot, "TalkSub", RZ_UIText.Str(m_aHead, 4));
		RZ_UIW.Tint(m_wRoot, "LineAccent", m_Accent);
		RZ_UIW.Tint(m_wRoot, "LineBar", m_Accent);
		RZ_UIW.Tint(m_wRoot, "ReplyPanelLine", m_Accent);
		RZ_UIW.Tint(m_wRoot, "PagePrevIcon", m_Accent);
		RZ_UIW.Tint(m_wRoot, "PageNextIcon", m_Accent);
		RZ_UIW.Tint(m_wRoot, "PageText", m_Accent);
		string said = RZ_UIText.Str(m_aHead, 5);
		RZ_UIW.SetText(m_wRoot, "TalkSaid", Unbreak(said));
		ShowPage();
		BuildRows();
		bool answer = m_bSaid || said != m_sLastSaid;
		m_bSaid = false;
		m_fSaidAge = -1;
		m_sLastSaid = said;
		if (answer && !said.IsEmpty())
			ShowLine(said);
		else if (m_bLine)
			Style();
		else
			ShowReplies();
	}

	//------------------------------------------------------------------------------------------------
	//! Step 1: their line on its own, its bar empty, from the top.
	protected void ShowLine(string said)
	{
		m_bLine = true;
		m_fRead = ReadTime(said);
		m_fReadLeft = m_fRead;
		RZ_UIW.Show(m_wRoot, "ReplyPanel", false);
		RZ_UIW.Show(m_wRoot, "LinePanel", true);
		RZ_UIW.Progress(m_wRoot, "LineBar", 0);
		ScrollLayoutWidget box = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "SaidScroll"));
		if (box)
			box.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Step 2: the replies on their own.
	protected void ShowReplies()
	{
		m_bLine = false;
		m_fRepliesAt = RZ_UIW.Now();
		RZ_UIW.Show(m_wRoot, "LinePanel", false);
		RZ_UIW.Show(m_wRoot, "ReplyPanel", true);
		foreach (Widget row : m_aRows)
		{
			RZ_UIW.Fade(row, 1);
		}

		Style();
	}

	//------------------------------------------------------------------------------------------------
	//! "1/2" bottom right and the arrows at the left when the list has more than one page: up while
	//! there's a page before, down while there's one after (LB/RB still turn round from either end).
	protected void ShowPage()
	{
		m_iPage = Math.Max(0, RZ_UIText.Int(m_aHead, 6));
		m_iPages = Math.Max(1, RZ_UIText.Int(m_aHead, 7));
		bool paged = m_iPages > 1;
		RZ_UIW.Show(m_wRoot, "PageText", paged);
		RZ_UIW.Show(m_wRoot, "PagePrev", paged && m_iPage > 0);
		RZ_UIW.Show(m_wRoot, "PageNext", paged && m_iPage < m_iPages - 1);
		if (!paged)
			return;

		int shown = m_iPage + 1;
		RZ_UIW.SetText(m_wRoot, "PageText", string.Format("%1/%2", shown, m_iPages));
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
		// first that can be picked
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
			string tag = RZ_UIText.Str(f, 3);
			RZ_UIW.SetText(row, "ReplyText", RZ_UIText.Str(f, 2));
			RZ_UIW.SetText(row, "ReplyTag", TagText(tag));
			RZ_UIW.Show(row, "ReplyTag", !tag.IsEmpty());
			RZ_UIW.SetText(row, "ReplyNeed", "(" + need + ")");
			RZ_UIW.Show(row, "ReplyNeed", !enabled && !need.IsEmpty());
			RZ_UIW.Tint(row, "ReplyBg", m_Accent);
			RZ_UIW.Tint(row, "ReplyEdge", m_Accent);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_RowEvents);
			m_aRows.Insert(row);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The selected row (the D-pad's, the keys' or the cursor's) with a fill and an outline in the
	//! faction colour; check tags in that colour; a reply with nothing new behind it grey, one that
	//! can't be picked faint, its tag too.
	protected void Style()
	{
		Color faint = RZ_UIW.Rgb(110, 106, 96);
		foreach (int i, Widget row : m_aRows)
		{
			bool on = i == m_iSelected;
			RZ_UIW.Show(row, "ReplyEdge", on);
			Widget fill = RZ_UIW.Find(row, "ReplyBg");
			if (on)
				RZ_UIW.Fade(fill, FILL_ALPHA);
			else
				RZ_UIW.Fade(fill, 0);

			Color fg = RZ_UIColor.Bone();
			Color tagColor = m_Accent;
			if (!m_aEnabled[i])
			{
				fg = faint;
				tagColor = faint;
			}
			else if (m_aHeard[i])
			{
				fg = RZ_UIW.Rgb(150, 145, 133);
				if (on)
					fg = RZ_UIW.Rgb(185, 180, 166);
			}
			else if (on)
			{
				fg = RZ_UIW.Rgb(244, 240, 230);
			}

			RZ_UIW.Tint(row, "ReplyText", fg);
			RZ_UIW.Tint(row, "ReplyTag", tagColor);
		}

		// Pad focus on the selected reply, so the D-pad and A act on what's lit
		if (!m_bLine && m_iSelected >= 0 && m_iSelected < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[m_iSelected]);
	}
}
