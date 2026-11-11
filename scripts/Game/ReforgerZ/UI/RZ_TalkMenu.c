//! The talk screen (docs/UI.md, "Talking to people"), as approved in UI/Mockups/talk.html: the
//! world stays in view, dimmed; a panel across the bottom with a top border in the person's faction
//! colour; their name and "The Haven - council head - knows your name"; what they say in large
//! bright type; your replies as a numbered list, the selected one lighter with an accent bar, a
//! dim tag on the right, what a reply needs when it can't be picked; the last is always a way out.
//!
//! Keys: up/down or 1-7, Enter; mouse: click; Esc leaves. Pad: D-pad or left stick, A says it,
//! B leaves; LB/RB (Q/E) scroll what they say when it's long. You can't move while it's open; the
//! world keeps running and danger closes it (the server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_TalkMenu : RZ_MenuBase
{
	protected static RZ_TalkMenu s_Open;
	protected static string s_sNode;
	protected static int s_iVersion;
	protected static bool s_bFailed;		// the menu couldn't open here: the actions come back

	protected int m_iSelected;
	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected bool m_bDone;					// left, or closed by the server: nothing more to send
	protected ref array<int> m_aIds = {};
	protected ref array<bool> m_aEnabled = {};
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
	static bool IsOpen()
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
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnScrollUp);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnScrollDown);
		input.AddActionListener("RZ_Reply1", EActionTrigger.DOWN, OnReply1);
		input.AddActionListener("RZ_Reply2", EActionTrigger.DOWN, OnReply2);
		input.AddActionListener("RZ_Reply3", EActionTrigger.DOWN, OnReply3);
		input.AddActionListener("RZ_Reply4", EActionTrigger.DOWN, OnReply4);
		input.AddActionListener("RZ_Reply5", EActionTrigger.DOWN, OnReply5);
		input.AddActionListener("RZ_Reply6", EActionTrigger.DOWN, OnReply6);
		input.AddActionListener("RZ_Reply7", EActionTrigger.DOWN, OnReply7);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Say it");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Leave") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "More") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnScrollUp);
		input.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnScrollDown);
		input.RemoveActionListener("RZ_Reply1", EActionTrigger.DOWN, OnReply1);
		input.RemoveActionListener("RZ_Reply2", EActionTrigger.DOWN, OnReply2);
		input.RemoveActionListener("RZ_Reply3", EActionTrigger.DOWN, OnReply3);
		input.RemoveActionListener("RZ_Reply4", EActionTrigger.DOWN, OnReply4);
		input.RemoveActionListener("RZ_Reply5", EActionTrigger.DOWN, OnReply5);
		input.RemoveActionListener("RZ_Reply6", EActionTrigger.DOWN, OnReply6);
		input.RemoveActionListener("RZ_Reply7", EActionTrigger.DOWN, OnReply7);
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
	override protected int DataVersion()
	{
		return s_iVersion * 100 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Max(0, m_iSelected - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Min(Math.Max(0, m_aIds.Count() - 1), m_iSelected + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		Say(m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScrollUp(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.Scroll(m_wRoot, "SaidScroll", -1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScrollDown(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.Scroll(m_wRoot, "SaidScroll", 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnReply1(float value = 0, EActionTrigger reason = 0) { Say(0); }
	protected void OnReply2(float value = 0, EActionTrigger reason = 0) { Say(1); }
	protected void OnReply3(float value = 0, EActionTrigger reason = 0) { Say(2); }
	protected void OnReply4(float value = 0, EActionTrigger reason = 0) { Say(3); }
	protected void OnReply5(float value = 0, EActionTrigger reason = 0) { Say(4); }
	protected void OnReply6(float value = 0, EActionTrigger reason = 0) { Say(5); }
	protected void OnReply7(float value = 0, EActionTrigger reason = 0) { Say(6); }

	//------------------------------------------------------------------------------------------------
	//! Esc or B: leave (the last reply).
	override protected void Back()
	{
		Say(m_aIds.Count() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a reply row says it.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "ReplyRow")
			Say(RZ_UIW.GetTag(w));
	}

	//------------------------------------------------------------------------------------------------
	//! Pick reply `index` (it must be enabled); Leave closes the screen.
	protected void Say(int index)
	{
		if (index < 0 || index >= m_aIds.Count() || !m_aEnabled[index])
		{
			if (index >= 0 && index < m_aIds.Count())
				m_iSelected = index;

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
	//! T|serial|faction|name|sub|said, then R|id|label|tag|need|enabled.
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		int faction = RZ_UIText.Int(m_aHead, 2);
		Color accent = RZ_UIColor.Bone();
		if (faction >= 0 && !RZ_UIText.Str(m_aHead, 2).IsEmpty())
			accent = RZ_UIColor.Faction(faction);

		RZ_UIW.Tint(m_wRoot, "TalkAccent", accent);
		RZ_UIW.SetText(m_wRoot, "TalkName", RZ_UIText.Str(m_aHead, 3));
		RZ_UIW.Tint(m_wRoot, "TalkName", accent);
		RZ_UIW.SetText(m_wRoot, "TalkSub", RZ_UIText.Str(m_aHead, 4));
		RZ_UIW.SetText(m_wRoot, "TalkSaid", Unbreak(RZ_UIText.Str(m_aHead, 5)));
		DrawReplies(accent);
	}

	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sNode, recs);
		m_aHead.Clear();
		m_aIds.Clear();
		m_aEnabled.Clear();
		m_aReplies.Clear();
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "T")
			{
				m_aHead.Copy(f);
				m_iSerial = RZ_UIText.Int(f, 1);
			}
			else if (tag == "R" && m_aIds.Count() < 7)
			{
				m_aReplies.Insert(f);
				m_aIds.Insert(RZ_UIText.Int(f, 1));
				m_aEnabled.Insert(RZ_UIText.Int(f, 5) == 1);
			}
		}

		// A new node starts on its first reply that can be picked; what they say starts at the top
		m_iSelected = 0;
		foreach (int i, bool on : m_aEnabled)
		{
			if (on)
			{
				m_iSelected = i;
				break;
			}
		}

		ScrollLayoutWidget said = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "SaidScroll"));
		if (said)
			said.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawReplies(Color accent)
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

			bool on = i == m_iSelected;
			bool enabled = m_aEnabled[i];
			string need = RZ_UIText.Str(f, 4);
			int num = i + 1;
			RZ_UIW.SetText(row, "ReplyNum", num.ToString());
			RZ_UIW.SetText(row, "ReplyText", RZ_UIText.Str(f, 2));
			RZ_UIW.SetText(row, "ReplyTag", RZ_UIText.Str(f, 3));
			RZ_UIW.Show(row, "ReplyTag", enabled && !RZ_UIText.Str(f, 3).IsEmpty());
			RZ_UIW.SetText(row, "ReplyNeed", "(" + need + ")");
			RZ_UIW.Show(row, "ReplyNeed", !enabled && !need.IsEmpty());
			RZ_UIW.Show(row, "ReplyBg", on);
			RZ_UIW.Show(row, "ReplyBar", on);
			RZ_UIW.Tint(row, "ReplyBar", accent);
			RZ_UIW.Tint(row, "ReplyNum", accent);
			Color fg = RZ_UIColor.Bone();
			if (!enabled)
				fg = RZ_UIW.Rgb(110, 106, 96);
			else if (on)
				fg = RZ_UIW.Rgb(244, 240, 230);

			RZ_UIW.Tint(row, "ReplyText", fg);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRows.Insert(row);
		}

		// Pad focus on the selected reply, so the D-pad and A act on what's lit
		if (m_iSelected >= 0 && m_iSelected < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[m_iSelected]);
	}
}
