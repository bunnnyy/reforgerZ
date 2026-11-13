//! The radio screen (docs/UI.md, "The radio screen"): every channel the radio can hear as a tile,
//! all on the screen at once (four to a row, no list to scroll), and under them the picked
//! channel's lines this life, a page at a time. Opened by the server when a player uses View radio
//! (Interactions/RZ_RadioScreen.c). The log is the one this client keeps for the notebook's Radio
//! tab (RZ_PlayerUIComponent.GetRadioLog), read here; the cracked channel is the HUD's feed. It
//! sends nothing back but its closing.
//!
//! Keys: the arrows or WASD move over the tiles (the log follows), Q/E turn the log's page, Esc
//! closes. Pad: D-pad or left stick, LB newer, RB older, B closes. Mouse: hover lights a tile, a
//! click picks it, the < and > buttons turn the page, the close button. You can't move while it's
//! open; danger closes it (the server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_RadioMenu : RZ_MenuBase
{
	protected static RZ_RadioMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static bool s_bFailed;			// the menu couldn't open here: View radio hides
	protected static float s_fFallbackUntil;	// the server was at its cap: View radio hides for a minute
	protected static int s_iChannel;			// the picked channel, kept for the session (0: everything)

	protected static const int COLS = 4;			// tiles to a row
	protected static const int CELLS = 12;			// Cell0-Cell11 in the layout: three rows
	protected static const int LOG_ROWS = 4;		// Log0-Log3 in the layout: lines to a page
	protected static const int LOG_CHARS = 235;		// a line's text fits two rows of the log at 22 px

	protected int m_iParsed = -1;
	protected int m_iPage;						// the log's page, 0 the newest
	protected int m_iPages = 1;
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
		input.AddActionListener("MenuLeft", EActionTrigger.DOWN, OnLeft);
		input.AddActionListener("MenuRight", EActionTrigger.DOWN, OnRight);
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnNewer);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnOlder);
		RZ_UIW.OnClick(m_wRoot, "LogNewer", m_Click);
		RZ_UIW.OnClick(m_wRoot, "LogOlder", m_Click);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuDown", "Channel");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabLeft", "Newer") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "Older") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnLeft);
		input.RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnRight);
		input.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnNewer);
		input.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnOlder);
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
	//! New data from the server, a new line in the log, another channel, another page, and the
	//! cracked channel's minutes.
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

		return ((s_iVersion % 500) * 1000 + (log % 1000)) * 2000 + (minutes % 10) * 200 + (m_iPage % 16) * 12 + (s_iChannel % CELLS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		MoveRow(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		MoveRow(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeft(float value = 0, EActionTrigger reason = 0)
	{
		Step(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRight(float value = 0, EActionTrigger reason = 0)
	{
		Step(1);
	}

	//------------------------------------------------------------------------------------------------
	//! A row up or down in the same column, round the ends. The last row can be short: from above
	//! an empty cell it's the row's last tile, never the empty cell.
	protected void MoveRow(int dir)
	{
		int count = m_aChannels.Count();
		if (count <= 0)
			return;

		int rows = (count + COLS - 1) / COLS;
		int row = s_iChannel / COLS + dir;
		if (row < 0)
			row = rows - 1;
		else if (row >= rows)
			row = 0;

		int target = row * COLS + s_iChannel % COLS;
		if (target >= count)
			target = count - 1;

		Pick(target);
	}

	//------------------------------------------------------------------------------------------------
	//! The tile before or after, round the ends.
	protected void Step(int dir)
	{
		int count = m_aChannels.Count();
		if (count <= 0)
			return;

		int target = s_iChannel + dir;
		if (target < 0)
			target = count - 1;
		else if (target >= count)
			target = 0;

		Pick(target);
	}

	//------------------------------------------------------------------------------------------------
	//! LB / Q, or the < button: the log's newer page.
	protected void OnNewer(float value = 0, EActionTrigger reason = 0)
	{
		m_iPage = Math.Max(0, m_iPage - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! RB / E, or the > button: the log's older page.
	protected void OnOlder(float value = 0, EActionTrigger reason = 0)
	{
		m_iPage = Math.Min(Math.Max(0, m_iPages - 1), m_iPage + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Another channel: the log shows its lines from the newest.
	protected void Pick(int index)
	{
		int picked = Math.Clamp(index, 0, Math.Max(0, m_aChannels.Count() - 1));
		if (picked == s_iChannel)
			return;

		s_iChannel = picked;
		m_iPage = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: a tile picks its channel; < and > turn the log's page.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "ChannelRow")
			Pick(RZ_UIW.GetTag(w));
		else if (name == "LogNewer")
			OnNewer();
		else if (name == "LogOlder")
			OnOlder();
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
	//! Nothing comes through on it: scrambled, or too faint for a set without power.
	protected static bool Silent(string state)
	{
		return state == "SCRAMBLED" || state == "TOO FAINT";
	}

	//------------------------------------------------------------------------------------------------
	//! The tile's small note: when it was last heard, or why nothing is.
	protected string NoteOf(array<string> ch, string state)
	{
		if (state == "SCRAMBLED")
			return "operator only";

		if (state == "TOO FAINT")
			return "needs power";

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
		{
			foreach (RZ_RadioEntry e : ui.GetRadioLog())
			{
				if (Matches(ch, e))
					return "last " + e.m_sTime;
			}
		}

		return "";
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
	//! One tile per channel, each made in its own cell of the layout (Cell0 on: four to a row), so
	//! every channel is on the screen and nothing scrolls.
	protected void DrawChannels()
	{
		m_aRows.Clear();
		ResourceName tileLayout = RZ_UISettings.Get().m_sRadioChannelLayout;
		bool icons = !RZ_UISettings.Get().m_sIconImageset.IsEmpty();
		for (int i = 0; i < CELLS; i++)
		{
			Widget cell = RZ_UIW.Find(m_wRoot, "Cell" + i.ToString());
			if (!cell)
				continue;

			RZ_UIW.ClearChildren(cell);
			if (i >= m_aChannels.Count())
				continue;

			Widget tile = RZ_UIW.Create(tileLayout, cell); // VERIFY: a layout made under an Overlay cell (the tile's root in an OverlayWidgetSlot, both aligns Stretch) fills the cell
			if (!tile)
				return;

			array<string> ch = m_aChannels[i];
			string state = StateOf(ch);
			Color accent = RZ_UIColor.Bone();
			string icon = "glyph_radio";
			if (RZ_UIText.Int(ch, 1) >= 0)
			{
				RZ_NoticeStyle st = StyleOf(ch);
				accent = st.Accent();
				icon = st.Icon();
			}

			RZ_UIW.SetText(tile, "ChannelName", RZ_UIText.Str(ch, 3));
			RZ_UIW.SetText(tile, "ChannelState", state);
			RZ_UIW.Tint(tile, "ChannelState", accent);
			RZ_UIW.SetText(tile, "ChannelNote", NoteOf(ch, state));
			RZ_UIW.Show(tile, "ChannelIcon", icons);
			RZ_UIW.Icon(tile, "ChannelIcon", icon);
			RZ_UIW.Tint(tile, "ChannelIcon", accent);
			RZ_UIW.Tint(tile, "ChannelBar", accent);
			LightTile(tile, state, i == s_iChannel, Hovered("ChannelRow", i));
			RZ_UIW.SetTag(tile, i);
			RZ_UIW.OnClick(tile, "", m_Click);
			m_aRows.Insert(tile);
		}

		// Pad focus on the picked tile
		if (s_iChannel >= 0 && s_iChannel < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[s_iChannel]);
	}

	//------------------------------------------------------------------------------------------------
	//! A tile as it stands: the picked one has the amber outline and a lighter fill, the one under
	//! the mouse the lighter fill; one nothing comes through on (scrambled, too faint) is dim
	//! unless it's picked or under the mouse.
	protected void LightTile(Widget tile, string state, bool picked, bool hovered)
	{
		Color fill = RZ_UIW.Rgb(27, 31, 34);
		if (picked || hovered)
			fill = RZ_UIW.Rgb(38, 43, 47);

		Color fg = RZ_UIColor.Bone();
		if (picked)
			fg = RZ_UIW.Rgb(244, 240, 230);

		RZ_UIW.Show(tile, "ChannelFocus", picked);
		RZ_UIW.Tint(tile, "ChannelBg", fill);
		RZ_UIW.Tint(tile, "ChannelName", fg);
		float alpha = 1;
		if (Silent(state) && !picked && !hovered)
			alpha = 0.45;

		tile.SetOpacity(alpha);
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: the tile under the cursor is lit until the cursor leaves; a click picks it (the log
	//! changes only then).
	override protected bool OnHoverRow(Widget w, string name, int tag, bool on)
	{
		if (name != "ChannelRow")
			return false;

		if (tag >= 0 && tag < m_aChannels.Count())
			LightTile(w, StateOf(m_aChannels[tag]), tag == s_iChannel, on);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The picked channel's lines, newest first: what's on the air on it now (the server's, unless
	//! it's the newest line you heard), then what you heard on it this life.
	protected void LinesOf(array<string> ch, notnull array<ref RZ_RadioEntry> lines)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
		{
			foreach (RZ_RadioEntry e : ui.GetRadioLog())
			{
				if (Matches(ch, e))
					lines.Insert(e);
			}
		}

		string now = RZ_UIText.Str(ch, 5);
		if (now.IsEmpty() || RZ_UIText.Int(ch, 1) < 0)
			return;

		if (!lines.IsEmpty() && lines[0].m_sText == now)
			return;

		RZ_RadioEntry air = new RZ_RadioEntry();
		air.m_sTime = "NOW";
		air.m_iKind = RZ_UIText.Int(ch, 1);
		air.m_iFaction = RZ_UIText.Int(ch, 2);
		air.m_sTitle = "";
		air.m_sText = now;
		lines.InsertAt(air, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! The picked channel's lines in the layout's four rows (Log0 on), a page at a time: no list to
	//! scroll. Each line's text is cut to what two rows hold; the whole of it is in the notebook.
	protected void DrawLog()
	{
		array<string> ch = null;
		if (s_iChannel >= 0 && s_iChannel < m_aChannels.Count())
			ch = m_aChannels[s_iChannel];

		string head = "THE RADIO";
		string state = "";
		array<ref RZ_RadioEntry> lines = {};
		if (ch)
		{
			head = RZ_UIText.Str(ch, 3);
			state = StateOf(ch);
			LinesOf(ch, lines);
		}

		head.ToUpper();
		RZ_UIW.SetText(m_wRoot, "LogLabel", head);
		m_iPages = Math.Max(1, (lines.Count() + LOG_ROWS - 1) / LOG_ROWS);
		m_iPage = Math.Clamp(m_iPage, 0, m_iPages - 1);
		bool icons = !RZ_UISettings.Get().m_sIconImageset.IsEmpty();
		for (int r = 0; r < LOG_ROWS; r++)
		{
			string idx = r.ToString();
			int at = m_iPage * LOG_ROWS + r;
			bool there = at < lines.Count();
			RZ_UIW.Show(m_wRoot, "Log" + idx, there);
			if (!there)
				continue;

			RZ_RadioEntry e = lines[at];
			RZ_NoticeStyle st = new RZ_NoticeStyle();
			st.m_iKind = e.m_iKind;
			st.m_iFaction = e.m_iFaction;
			Color accent = st.Accent();
			string who = st.KindText();
			if (e.m_sTime == "NOW")
				who = "ON THE AIR NOW";
			else if (!e.m_sTitle.IsEmpty())
				who = who + "  ·  " + e.m_sTitle;

			RZ_UIW.Tint(m_wRoot, "LogBar" + idx, accent);
			RZ_UIW.Tint(m_wRoot, "LogIcon" + idx, accent);
			RZ_UIW.Tint(m_wRoot, "LogHead" + idx, accent);
			RZ_UIW.Show(m_wRoot, "LogIcon" + idx, icons);
			RZ_UIW.Icon(m_wRoot, "LogIcon" + idx, st.Icon());
			RZ_UIW.SetText(m_wRoot, "LogTime" + idx, e.m_sTime);
			RZ_UIW.SetText(m_wRoot, "LogHead" + idx, RZ_UIText.Clean(who, 110));
			RZ_UIW.SetText(m_wRoot, "LogText" + idx, RZ_UIText.Clean(e.m_sText, LOG_CHARS));
		}

		string empty = "Nothing heard on it this life.";
		if (state == "SCRAMBLED")
			empty = "Chopped into pieces. A Radio Operator could put it back together.";
		else if (state == "TOO FAINT")
			empty = "Too faint for this set. With power it would come in.";

		RZ_UIW.SetText(m_wRoot, "LogEmpty", empty);
		RZ_UIW.Show(m_wRoot, "LogEmpty", lines.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "LogPage", string.Format("PAGE %1 / %2", m_iPage + 1, m_iPages));
		RZ_UIW.Show(m_wRoot, "LogPage", m_iPages > 1);
		RZ_UIW.Show(m_wRoot, "LogNewer", m_iPage > 0);
		RZ_UIW.Show(m_wRoot, "LogOlder", m_iPage < m_iPages - 1);
	}
}
