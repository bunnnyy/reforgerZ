//! The radio screen (docs/UI.md, "The radio screen"), in the crafting screen's style: every channel
//! the radio can hear, each with its latest line, and under them the log of what you heard this
//! life, filtered by the picked channel. Opened by the server when a player uses View radio
//! (Interactions/RZ_RadioScreen.c). The log is the one this client keeps for the notebook's Radio
//! tab (RZ_PlayerUIComponent.GetRadioLog), read here; the cracked channel is the HUD's feed. It
//! sends nothing back but its closing.
//!
//! Keys: up/down pick a channel (the log follows), Q/E scroll the log, Esc closes. Pad: D-pad or
//! left stick, LB/RB scroll the log, B closes. Mouse: click a channel, the wheel scrolls either
//! list, the close button. You can't move while it's open; danger closes it (the server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_RadioMenu : RZ_MenuBase
{
	protected static RZ_RadioMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static bool s_bFailed;			// the menu couldn't open here: View radio hides
	protected static float s_fFallbackUntil;	// the server was at its cap: View radio hides for a minute
	protected static int s_iChannel;			// the picked channel, kept for the session (0: everything)

	protected static const int LOG_ROWS = 60;

	protected int m_iParsed = -1;
	protected bool m_bDone;						// closed by the server: nothing to send back
	protected ref array<string> m_aHead = {};
	protected ref array<ref array<string>> m_aChannels = {};
	protected ref array<Widget> m_aRows = {};

	//------------------------------------------------------------------------------------------------
	//! The radio screen is used on this machine: UI > Radio channel layout set (the server checks the
	//! same), it hasn't failed to open here, and the server isn't at its cap of open screens.
	static bool Available()
	{
		return !s_bFailed && RZ_UIW.Now() >= s_fFallbackUntil && !RZ_UISettings.Get().m_sRadioChannelLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! A radio payload (RZ_PlayerUIComponent): opens or redraws the screen; X|reason closes it;
	//! F|why: the server couldn't open one.
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

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (payload.StartsWith("F" + RZ_UIText.SEP))
		{
			s_fFallbackUntil = RZ_UIW.Now() + 60;
			if (gm)
				gm.ShowNotice("Radio", "Too much going on at once. What you've heard is in your notebook, on the Radio tab.", RZ_ENoticeTag.AUTO);

			return;
		}

		s_sData = payload;
		s_iVersion++;
		if (s_Open || RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_RadioMenu))
			return;

		// No preset or layout here: View radio hides from now on
		s_bFailed = true;
		ui.SendScreenClosed(RZ_ScreenGuard.KIND_RADIO);
		if (gm)
			gm.ShowNotice("Radio", "What you've heard is in your notebook, on the Radio tab.", RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnLogUp);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnLogDown);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuDown", "Channel");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "Scroll the log") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnLogUp);
		input.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnLogDown);
		if (s_Open == this)
			s_Open = null;

		// Esc, B, the close button, or another menu on top: the server forgets the screen
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendScreenClosed(RZ_ScreenGuard.KIND_RADIO);

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! New data from the server, a new line in the log, another channel, and the cracked
	//! channel's minutes.
	override protected int DataVersion()
	{
		int log = 0;
		int minutes = 0;
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
		{
			log = ui.GetRadioVersion();
			minutes = RZ_Util.RoundToInt(Math.Ceil(ui.GetListenLeft() / 60));
		}

		return s_iVersion * 100000 + log * 1000 + minutes * 40 + s_iChannel;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		Pick(s_iChannel - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		Pick(s_iChannel + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! LB / Q: the log a step up.
	protected void OnLogUp(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.Scroll(m_wRoot, "LogScroll", -1);
	}

	//------------------------------------------------------------------------------------------------
	//! RB / E: the log a step down.
	protected void OnLogDown(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.Scroll(m_wRoot, "LogScroll", 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Another channel: the log shows its lines from the newest.
	protected void Pick(int index)
	{
		int picked = Math.Clamp(index, 0, Math.Max(0, m_aChannels.Count() - 1));
		if (picked == s_iChannel)
			return;

		s_iChannel = picked;
		ScrollLayoutWidget log = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "LogScroll"));
		if (log)
			log.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: a channel row picks it.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "ChannelRow")
			Pick(RZ_UIW.GetTag(w));
	}

	//------------------------------------------------------------------------------------------------
	//! V|title|sub|powered; C|kind|faction|name|state|line. The first channel is made here: everything.
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aChannels.Clear();
		array<string> all = {"C", "-1", "-1", "Everything on the radio", "", ""};
		m_aChannels.Insert(all);
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "V")
				m_aHead.Copy(f);
			else if (tag == "C")
				m_aChannels.Insert(f);
		}

		s_iChannel = Math.Clamp(s_iChannel, 0, m_aChannels.Count() - 1);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		RZ_UIW.SetText(m_wRoot, "RadioTitle", RZ_UIText.Str(m_aHead, 1));
		string sub = RZ_UIText.Str(m_aHead, 2);
		if (RZ_UIText.Int(m_aHead, 3) != 1)
		{
			if (!sub.IsEmpty())
				sub += ". ";

			sub += "The batteries are weak: with power it would pull in more.";
		}

		RZ_UIW.SetText(m_wRoot, "RadioSub", sub);
		RZ_UIW.Show(m_wRoot, "RadioSub", !sub.IsEmpty());
		DrawChannels();
		DrawLog();
	}

	//------------------------------------------------------------------------------------------------
	//! The channel's state now: the server's for the broadcast and the open channels, this client's
	//! for a secure one (cracked, with minutes left, or scrambled) and for everything (lines heard).
	protected string StateOf(array<string> ch)
	{
		int kind = RZ_UIText.Int(ch, 1);
		if (kind < 0)
		{
			int heard = 0;
			RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
			if (ui)
			{
				foreach (RZ_RadioEntry e : ui.GetRadioLog())
				{
					if (Matches(ch, e))
						heard++;
				}
			}

			return string.Format("%1 HEARD", heard);
		}

		if (kind != RZ_ENoticeKind.SECURE_CHANNEL)
			return RZ_UIText.Str(ch, 4);

		RZ_PlayerUIComponent local = RZ_PlayerUIComponent.GetLocal();
		if (local && local.GetListenFaction() == RZ_UIText.Int(ch, 2) && local.GetListenLeft() > 0)
			return string.Format("CRACKED: %1 MIN", RZ_Util.RoundToInt(Math.Ceil(local.GetListenLeft() / 60)));

		return "SCRAMBLED";
	}

	//------------------------------------------------------------------------------------------------
	//! Its latest line: what's on the air now (the server's), else the newest you heard on it.
	protected string LatestOf(array<string> ch)
	{
		string line = RZ_UIText.Str(ch, 5);
		if (!line.IsEmpty())
			return line;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
		{
			foreach (RZ_RadioEntry e : ui.GetRadioLog())
			{
				if (Matches(ch, e))
					return e.m_sText;
			}
		}

		if (RZ_UIText.Int(ch, 1) == RZ_ENoticeKind.SECURE_CHANNEL)
			return "Chopped into pieces. A Radio Operator could put it back together.";

		return "Nothing yet.";
	}

	//------------------------------------------------------------------------------------------------
	//! A log entry belongs to the channel: everything on the radio, the broadcast, or an open or
	//! secure channel by its faction.
	protected bool Matches(array<string> ch, RZ_RadioEntry e)
	{
		int kind = RZ_UIText.Int(ch, 1);
		if (kind < 0)
			return e.m_iKind == RZ_ENoticeKind.OPEN_CHANNEL || e.m_iKind == RZ_ENoticeKind.SECURE_CHANNEL || e.m_iKind == RZ_ENoticeKind.BROADCAST;

		if (e.m_iKind != kind)
			return false;

		return kind == RZ_ENoticeKind.BROADCAST || e.m_iFaction == RZ_UIText.Int(ch, 2);
	}

	//------------------------------------------------------------------------------------------------
	//! The channel's look: its notice style (icon, colour), the radio's own for everything.
	protected RZ_NoticeStyle StyleOf(array<string> ch)
	{
		RZ_NoticeStyle st = new RZ_NoticeStyle();
		st.m_iKind = RZ_UIText.Int(ch, 1);
		st.m_iFaction = RZ_UIText.Int(ch, 2);
		return st;
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawChannels()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Channels");
		RZ_UIW.ClearChildren(list);
		m_aRows.Clear();
		ResourceName rowLayout = RZ_UISettings.Get().m_sRadioChannelLayout;
		foreach (int i, array<string> ch : m_aChannels)
		{
			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				return;

			string state = StateOf(ch);
			Color accent = RZ_UIColor.Bone();
			string icon = "glyph_radio";
			if (RZ_UIText.Int(ch, 1) >= 0)
			{
				RZ_NoticeStyle st = StyleOf(ch);
				accent = st.Accent();
				icon = st.Icon();
			}

			RZ_UIW.SetText(row, "ChannelName", RZ_UIText.Str(ch, 3));
			RZ_UIW.SetText(row, "ChannelState", state);
			RZ_UIW.Tint(row, "ChannelState", accent);
			RZ_UIW.SetText(row, "ChannelLine", RZ_UIText.Clean(LatestOf(ch), 110));
			RZ_UIW.Show(row, "ChannelIcon", !RZ_UISettings.Get().m_sIconImageset.IsEmpty());
			RZ_UIW.Icon(row, "ChannelIcon", icon);
			RZ_UIW.Tint(row, "ChannelIcon", accent);
			RZ_UIW.Tint(row, "ChannelBar", accent);
			LightRow(row, state, i == s_iChannel || Hovered("ChannelRow", i));
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRows.Insert(row);
		}

		// Pad focus on the picked channel, kept in view
		if (s_iChannel >= 0 && s_iChannel < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[s_iChannel]);

		RZ_UIW.KeepInView(m_wRoot, "ChannelScroll", s_iChannel, m_aRows.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! A channel row (its state now) lit or not: the picked row and the row under the mouse have a
	//! lighter fill, the bar and bright text; dim when nothing comes through (scrambled, too faint, quiet).
	protected void LightRow(Widget row, string state, bool on)
	{
		Color fg = RZ_UIColor.Bone();
		if (state == "SCRAMBLED" || state == "TOO FAINT" || state == "QUIET")
			fg = RZ_UIW.Rgb(110, 106, 96);
		else if (on)
			fg = RZ_UIW.Rgb(244, 240, 230);

		RZ_UIW.Tint(row, "ChannelName", fg);
		RZ_UIW.Show(row, "ChannelBg", on);
		RZ_UIW.Show(row, "ChannelBar", on);
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: the channel under the cursor is lit like the picked one until the cursor leaves; a click
	//! picks it (the log changes only then).
	override protected bool OnHoverRow(Widget w, string name, int tag, bool on)
	{
		if (name != "ChannelRow")
			return false;

		if (tag >= 0 && tag < m_aChannels.Count())
			LightRow(w, StateOf(m_aChannels[tag]), on || tag == s_iChannel);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The picked channel's lines this life, newest first, in the notebook's radio rows.
	protected void DrawLog()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Log");
		RZ_UIW.ClearChildren(list);
		array<string> ch = null;
		if (s_iChannel >= 0 && s_iChannel < m_aChannels.Count())
			ch = m_aChannels[s_iChannel];

		string head = "THE LOG";
		if (ch)
			head = "THE LOG: " + RZ_UIText.Str(ch, 3);

		head.ToUpper();
		RZ_UIW.SetText(m_wRoot, "LogLabel", head);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		int shown = 0;
		ResourceName rowLayout = RZ_UISettings.Get().m_sRadioRowLayout;
		if (ui && ch)
		{
			foreach (RZ_RadioEntry e : ui.GetRadioLog())
			{
				if (shown >= LOG_ROWS || !Matches(ch, e))
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
		}

		RZ_UIW.Show(m_wRoot, "LogEmpty", shown == 0);
	}
}
