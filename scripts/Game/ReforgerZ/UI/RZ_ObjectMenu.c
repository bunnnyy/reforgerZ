//! The object menu (docs/UI.md, "The object menu"), in the crafting and car repair screens' style:
//! what a prop or a place is (its title, what you see) and its options as a list. Opened by the
//! server when a player uses the one prompt left on it (Interactions/RZ_ObjectScreen.c); every pick
//! goes back to the server, which runs that option's own action code.
//!
//! Keys: up/down pick, Enter does it, Esc closes. Pad: D-pad or left stick, A does it, B closes.
//! Mouse: click an option, the close button. You can't move while it's open; work you start (boiling
//! water, cracking a channel) goes on after it closes, as long as you stay close, and danger closes
//! it (the server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_ObjectMenu : RZ_MenuBase
{
	protected static RZ_ObjectMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static float s_fDataAt;
	protected static bool s_bFailed;			// the menu couldn't open here: the prompts come back
	protected static float s_fFallbackUntil;	// the server was at its cap: the prompts for a minute

	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected bool m_bDone;						// closed by the server: nothing to send back
	protected int m_iSelected;
	protected float m_fLeft;
	protected float m_fTotal;
	protected string m_sWorking;
	protected ref array<string> m_aHead = {};
	protected ref array<ref array<string>> m_aRows = {};
	protected ref array<Widget> m_aRowWidgets = {};

	//------------------------------------------------------------------------------------------------
	//! The object menu is used on this machine: UI > Object row layout set (the server checks the
	//! same), it hasn't failed to open here, and the server isn't at its cap of open screens.
	static bool Available()
	{
		return !s_bFailed && RZ_UIW.Now() >= s_fFallbackUntil && !RZ_UISettings.Get().m_sObjectRowLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! An object payload (RZ_PlayerUIComponent): opens or redraws the menu; X|reason closes it;
	//! F|why: the server couldn't open one, so the prompts show for a minute.
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

		if (payload.StartsWith("F" + RZ_UIText.SEP))
		{
			s_fFallbackUntil = RZ_UIW.Now() + 60;
			return;
		}

		s_sData = payload;
		s_iVersion++;
		s_fDataAt = RZ_UIW.Now();
		if (s_Open || RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_ObjectMenu))
			return;

		// No preset or layout here: what it says as a card, the prompts back, and the server told
		s_bFailed = true;
		ui.SendObjectFailed();
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		string title = "";
		string text = "";
		foreach (array<string> f : recs)
		{
			if (RZ_UIText.Str(f, 0) == "O")
			{
				title = RZ_UIText.Str(f, 2);
				text = Unflat(RZ_UIText.Str(f, 4));
			}
		}

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && !text.IsEmpty())
			gm.ShowNotice(title, text, RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Unflat(string text)
	{
		string t = text;
		t.Replace("^", "\n");
		return t;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Do it");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		if (s_Open == this)
			s_Open = null;

		// Esc, B, the close button, or another menu on top: the server forgets the menu
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendScreenClosed(RZ_ScreenGuard.KIND_OBJECT);

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! New data, a new selection, and once a second while work is under way (its countdown).
	override protected int DataVersion()
	{
		int tick = 0;
		if (!m_sWorking.IsEmpty())
			tick = RZ_Util.RoundToInt(Math.Floor(RZ_UIW.Now())) % 1000;

		return s_iVersion * 100000 + tick * 20 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Max(0, m_iSelected - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Min(Math.Max(0, m_aRows.Count() - 1), m_iSelected + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		Do(m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	//! A click on an option does it.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "OptionRow")
			Do(RZ_UIW.GetTag(w));
	}

	//------------------------------------------------------------------------------------------------
	//! Do option `index` if it can be done (the server checks it again).
	protected void Do(int index)
	{
		if (index < 0 || index >= m_aRows.Count())
			return;

		m_iSelected = index;
		array<string> f = m_aRows[index];
		if (RZ_UIText.Int(f, 3) != 1)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendObjectPick(m_iSerial, RZ_UIText.Int(f, 1));
	}

	//------------------------------------------------------------------------------------------------
	//! O|serial|title|sub|text|status|working|left|total; R|index|label|enabled|reason
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		RZ_UIW.SetText(m_wRoot, "ObjectTitle", RZ_UIText.Str(m_aHead, 2));
		string sub = RZ_UIText.Str(m_aHead, 3);
		RZ_UIW.SetText(m_wRoot, "ObjectSub", sub);
		RZ_UIW.Show(m_wRoot, "ObjectSub", !sub.IsEmpty());
		string text = Unflat(RZ_UIText.Str(m_aHead, 4));
		RZ_UIW.SetText(m_wRoot, "ObjectText", text);
		RZ_UIW.Show(m_wRoot, "ObjectText", !text.IsEmpty());

		string status = Unflat(RZ_UIText.Str(m_aHead, 5));
		RZ_UIW.SetText(m_wRoot, "ObjectStatus", status);
		RZ_UIW.Show(m_wRoot, "ObjectStatus", !status.IsEmpty());

		// Work under way: "Working on the boiling... 12 s" over a thin bar
		float left = Math.Max(0, m_fLeft - (RZ_UIW.Now() - s_fDataAt));
		bool working = !m_sWorking.IsEmpty() && left > 0;
		RZ_UIW.Show(m_wRoot, "ObjectWorking", working);
		RZ_UIW.Show(m_wRoot, "ObjectProgress", working && m_fTotal > 0);
		if (working)
		{
			RZ_UIW.SetText(m_wRoot, "ObjectWorking", string.Format("Busy with %1... %2 s", m_sWorking, RZ_Util.RoundToInt(Math.Ceil(left))));
			if (m_fTotal > 0)
				RZ_UIW.Progress(m_wRoot, "ObjectProgress", 1 - left / m_fTotal);
		}

		DrawRows();
	}

	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aRows.Clear();
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "O")
			{
				m_aHead.Copy(f);
				m_iSerial = RZ_UIText.Int(f, 1);
				m_sWorking = RZ_UIText.Str(f, 6);
				m_fLeft = RZ_UIText.Float(f, 7);
				m_fTotal = RZ_UIText.Float(f, 8);
			}
			else if (tag == "R")
			{
				m_aRows.Insert(f);
			}
		}

		m_iSelected = Math.Clamp(m_iSelected, 0, Math.Max(0, m_aRows.Count() - 1));
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawRows()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Options");
		RZ_UIW.ClearChildren(list);
		m_aRowWidgets.Clear();
		RZ_UIW.Show(m_wRoot, "OptionsEmpty", m_aRows.IsEmpty());
		ResourceName rowLayout = RZ_UISettings.Get().m_sObjectRowLayout;
		Color amber = RZ_UIColor.Faction(RZ_EFaction.SURVIVORS);
		foreach (int i, array<string> f : m_aRows)
		{
			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				return;

			bool on = i == m_iSelected;
			bool can = RZ_UIText.Int(f, 3) == 1;
			string reason = RZ_UIText.Str(f, 4);
			Color fg = RZ_UIColor.Bone();
			if (!can)
				fg = RZ_UIW.Rgb(110, 106, 96);
			else if (on)
				fg = RZ_UIW.Rgb(244, 240, 230);

			RZ_UIW.SetText(row, "OptionName", RZ_UIText.Str(f, 2));
			RZ_UIW.Tint(row, "OptionName", fg);
			RZ_UIW.SetText(row, "OptionNeed", reason);
			RZ_UIW.Show(row, "OptionNeed", !can && !reason.IsEmpty());
			RZ_UIW.Show(row, "OptionBg", on);
			RZ_UIW.Show(row, "OptionBar", on);
			RZ_UIW.Tint(row, "OptionBar", amber);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRowWidgets.Insert(row);
		}

		// Pad focus on the selected option, kept in view
		if (m_iSelected >= 0 && m_iSelected < m_aRowWidgets.Count())
			RZ_UIW.FocusWidget(m_aRowWidgets[m_iSelected]);

		ScrollLayoutWidget sc = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "OptionScroll"));
		if (sc && m_aRowWidgets.Count() > 1)
			sc.SetSliderPos(0, m_iSelected / (m_aRowWidgets.Count() - 1.0));
	}
}
