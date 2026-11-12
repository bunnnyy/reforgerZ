//! The power panel (docs/UI.md, "The power panel"): the power station's control as a screen. What
//! you see, the card inside the door, the breaker board, the spin-up, the gauge and the step's
//! button. Opened by the server when a player uses the control's one prompt, Open the panel
//! (World/RZ_PowerPanel.c); every move goes back to the server, which checks it.
//!
//! Keys: left/right pick a breaker or a button, up/down between the board and the buttons, Enter
//! flips the breaker or presses the button, Esc closes. Pad: D-pad or left stick, A, B. Mouse: click
//! a breaker or a button, the close button. You can't move while it's open; danger closes it (the
//! server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_PowerMenu : RZ_MenuBase
{
	protected static const float MOVE_GAP = 0.55;	// the server takes a move at most every 0.5 s

	protected static RZ_PowerMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static float s_fDataAt;
	protected static bool s_bFailed;				// the screen couldn't open here

	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected bool m_bDone;							// closed by the server: nothing to send back
	protected int m_iRow;							// 0 the breakers, 1 the buttons
	protected int m_iCol;
	protected int m_iStep;
	protected int m_iLastStep = -1;
	protected float m_fLeft;
	protected float m_fTotal;
	protected float m_fMovedAt = -100;
	protected ref array<string> m_aHead = {};
	protected ref array<ref array<string>> m_aBreakers = {};
	protected ref array<ref array<string>> m_aButtons = {};
	protected ref array<string> m_aCard = {};
	protected ref array<Widget> m_aBreakerWidgets = {};

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A power payload (RZ_PlayerUIComponent): opens or redraws the panel; X|reason closes it;
	//! F|busy: the server was at its cap (it said what you see as a notice).
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
			return;

		s_sData = payload;
		s_iVersion++;
		s_fDataAt = RZ_UIW.Now();
		if (s_Open)
			return;

		if (!s_bFailed && RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_PowerMenu))
			return;

		// No preset or layout here: the server puts the old prompts back and says what you see
		s_bFailed = true;
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		int serial = -1;
		foreach (array<string> f : recs)
		{
			if (RZ_UIText.Str(f, 0) == "P")
				serial = RZ_UIText.Int(f, 1);
		}

		ui.SendPowerMove(serial, RZ_PowerPanel.MOVE_FAILED);
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
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		RZ_UIW.OnClick(m_wRoot, "Action0", m_Click);
		RZ_UIW.OnClick(m_wRoot, "Action1", m_Click);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Flip or press");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		m_iRow = -1;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnLeft);
		input.RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnRight);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		if (s_Open == this)
			s_Open = null;

		// Esc, B, the close button, or another menu on top: the server forgets the panel
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendScreenClosed(RZ_ScreenGuard.KIND_POWER);

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! New data, a new selection, and once a second while the generator spins up (its countdown).
	override protected int DataVersion()
	{
		int tick = 0;
		if (m_iStep == RZ_PowerGrid.STEP_SPINNING)
			tick = RZ_Util.RoundToInt(Math.Floor(RZ_UIW.Now())) % 1000;

		return s_iVersion * 100000 + tick * 40 + (m_iRow + 1) * 10 + m_iCol;
	}

	//------------------------------------------------------------------------------------------------
	protected int RowCount(int row)
	{
		if (row == 0)
			return m_aBreakers.Count();

		return m_aButtons.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Up and down: between the board and the buttons (when both are there).
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		if (m_iRow == 1 && RowCount(0) > 0)
		{
			m_iRow = 0;
			m_iCol = Math.Clamp(m_iCol, 0, RowCount(0) - 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		if (m_iRow == 0 && RowCount(1) > 0)
		{
			m_iRow = 1;
			m_iCol = Math.Clamp(m_iCol, 0, RowCount(1) - 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeft(float value = 0, EActionTrigger reason = 0)
	{
		m_iCol = Math.Max(0, m_iCol - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRight(float value = 0, EActionTrigger reason = 0)
	{
		m_iCol = Math.Min(Math.Max(0, RowCount(m_iRow) - 1), m_iCol + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		if (m_iRow == 0)
			FlipBreaker(m_iCol);
		else if (m_iRow == 1)
			PressButton(m_iCol);
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a breaker flips it; on a button presses it.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "BreakerRow")
			FlipBreaker(RZ_UIW.GetTag(w));
		else if (name == "Action0")
			PressButton(0);
		else if (name == "Action1")
			PressButton(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Breaker `index`, while the board can be worked (the server checks it again).
	protected void FlipBreaker(int index)
	{
		if (index < 0 || index >= m_aBreakers.Count() || m_iStep != RZ_PowerGrid.STEP_OPEN)
			return;

		m_iRow = 0;
		m_iCol = index;
		SendMove(RZ_UIText.Int(m_aBreakers[index], 1));
	}

	//------------------------------------------------------------------------------------------------
	//! Button `index` if it can be pressed now (the server checks it again).
	protected void PressButton(int index)
	{
		if (index < 0 || index >= m_aButtons.Count())
			return;

		m_iRow = 1;
		m_iCol = index;
		array<string> f = m_aButtons[index];
		if (RZ_UIText.Int(f, 3) != 1)
			return;

		SendMove(RZ_PowerPanel.MOVE_BUTTON + RZ_UIText.Int(f, 1));
	}

	//------------------------------------------------------------------------------------------------
	protected void SendMove(int move)
	{
		float now = RZ_UIW.Now();
		if (now - m_fMovedAt < MOVE_GAP)
			return;

		m_fMovedAt = now;
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendPowerMove(m_iSerial, move);
	}

	//------------------------------------------------------------------------------------------------
	//! P|serial|title|state|text|status|step|left|total|gauge|ready; B|index|name|closed; C|line;
	//! A|index|label|enabled|need
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		RZ_UIW.SetText(m_wRoot, "PowerTitle", RZ_UIText.Str(m_aHead, 2));
		RZ_UIW.SetText(m_wRoot, "PowerSub", RZ_UIText.Str(m_aHead, 3));
		RZ_UIW.SetText(m_wRoot, "PowerText", Unflat(RZ_UIText.Str(m_aHead, 4)));
		string status = Unflat(RZ_UIText.Str(m_aHead, 5));
		RZ_UIW.SetText(m_wRoot, "PowerStatus", status);
		RZ_UIW.Show(m_wRoot, "PowerStatus", !status.IsEmpty());

		DrawCard();
		DrawBoard();
		DrawProgress();
		DrawButtons();
		FocusSelected();
	}

	//------------------------------------------------------------------------------------------------
	protected static string Unflat(string text)
	{
		string t = text;
		t.Replace("^", "\n");
		return t;
	}

	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aBreakers.Clear();
		m_aButtons.Clear();
		m_aCard.Clear();
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "P")
			{
				m_aHead.Copy(f);
				m_iSerial = RZ_UIText.Int(f, 1);
				m_iStep = RZ_UIText.Int(f, 6);
				m_fLeft = RZ_UIText.Float(f, 7);
				m_fTotal = RZ_UIText.Float(f, 8);
			}
			else if (tag == "B" && m_aBreakers.Count() < RZ_PowerPanel.BREAKERS)
			{
				m_aBreakers.Insert(f);
			}
			else if (tag == "C")
			{
				m_aCard.Insert(RZ_UIText.Str(f, 1));
			}
			else if (tag == "A" && m_aButtons.Count() < 2)
			{
				m_aButtons.Insert(f);
			}
		}

		// Where the selection goes: the board while it's worked, else the buttons; kept in range
		if (m_iStep != m_iLastStep || m_iRow < 0 || RowCount(m_iRow) == 0)
		{
			m_iRow = 1;
			if (m_iStep == RZ_PowerGrid.STEP_OPEN && !m_aBreakers.IsEmpty())
				m_iRow = 0;

			m_iCol = 0;
		}

		m_iLastStep = m_iStep;
		m_iCol = Math.Clamp(m_iCol, 0, Math.Max(0, RowCount(m_iRow) - 1));
	}

	//------------------------------------------------------------------------------------------------
	//! The card inside the door; before the door's forced, "Behind a padlocked door."
	protected void DrawCard()
	{
		bool open = !m_aCard.IsEmpty();
		string text = "";
		foreach (int i, string ln : m_aCard)
		{
			if (i > 0)
				text += "\n";

			text += ln;
		}

		RZ_UIW.SetText(m_wRoot, "CardText", text);
		RZ_UIW.Show(m_wRoot, "CardText", open);
		RZ_UIW.Show(m_wRoot, "CardShut", !open);
	}

	//------------------------------------------------------------------------------------------------
	//! The six breakers: up and lit when closed, down and dark when open; the board's READY lamp.
	protected void DrawBoard()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Breakers");
		RZ_UIW.ClearChildren(list);
		m_aBreakerWidgets.Clear();
		bool shown = !m_aBreakers.IsEmpty();
		RZ_UIW.Show(m_wRoot, "BoardShut", !shown);
		RZ_UIW.Show(m_wRoot, "BoardHint", shown && m_iStep == RZ_PowerGrid.STEP_OPEN);
		ResourceName layout = RZ_UISettings.Get().m_sPowerBreakerLayout;
		Color amber = RZ_UIColor.Faction(RZ_EFaction.SURVIVORS);
		Color dark = RZ_UIW.Rgb(40, 44, 48);
		foreach (int i, array<string> f : m_aBreakers)
		{
			Widget w = RZ_UIW.Create(layout, list);
			if (!w)
				break;

			bool closed = RZ_UIText.Int(f, 3) == 1;
			bool on = m_iRow == 0 && i == m_iCol;
			int num = i + 1;
			RZ_UIW.SetText(w, "BreakerNum", num.ToString());
			RZ_UIW.SetText(w, "BreakerName", RZ_UIText.Str(f, 2));
			RZ_UIW.Show(w, "BreakerUp", closed);
			RZ_UIW.Show(w, "BreakerDown", !closed);
			if (closed)
				RZ_UIW.Tint(w, "BreakerLamp", amber);
			else
				RZ_UIW.Tint(w, "BreakerLamp", dark);

			RZ_UIW.Show(w, "BreakerBg", on);
			RZ_UIW.Show(w, "BreakerFocus", on);
			RZ_UIW.Tint(w, "BreakerFocus", amber);
			RZ_UIW.SetTag(w, i);
			RZ_UIW.OnClick(w, "", m_Click);
			m_aBreakerWidgets.Insert(w);
		}

		bool ready = RZ_UIText.Int(m_aHead, 10) == 1;
		RZ_UIW.Show(m_wRoot, "ReadyLamp", shown);
		RZ_UIW.Show(m_wRoot, "ReadyText", shown);
		if (ready)
		{
			RZ_UIW.Tint(m_wRoot, "ReadyLamp", RZ_UIColor.Good());
			RZ_UIW.SetText(m_wRoot, "ReadyText", "READY");
		}
		else
		{
			RZ_UIW.Tint(m_wRoot, "ReadyLamp", dark);
			RZ_UIW.SetText(m_wRoot, "ReadyText", "NOT READY");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The spin-up countdown, or the running generator's service gauge.
	protected void DrawProgress()
	{
		float left = Math.Max(0, m_fLeft - (RZ_UIW.Now() - s_fDataAt));
		bool spinning = m_iStep == RZ_PowerGrid.STEP_SPINNING && m_fTotal > 0;
		int gauge = RZ_UIText.Int(m_aHead, 9);
		bool running = m_iStep == RZ_PowerGrid.STEP_POWERED && gauge >= 0;
		RZ_UIW.Show(m_wRoot, "MeterText", spinning || running);
		RZ_UIW.Show(m_wRoot, "MeterBar", spinning || running);
		if (spinning)
		{
			RZ_UIW.SetText(m_wRoot, "MeterText", string.Format("SPINNING UP: %1 s. STAY WITHIN 60 M.", RZ_Util.RoundToInt(Math.Ceil(left))));
			RZ_UIW.Progress(m_wRoot, "MeterBar", 1 - left / m_fTotal);
		}
		else if (running)
		{
			RZ_UIW.SetText(m_wRoot, "MeterText", string.Format("SERVICE GAUGE: %1 PERCENT", gauge));
			RZ_UIW.Progress(m_wRoot, "MeterBar", gauge / 100.0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Up to two buttons: amber when they can be pressed, dark with why when not.
	protected void DrawButtons()
	{
		Color amber = RZ_UIColor.Faction(RZ_EFaction.SURVIVORS);
		for (int i = 0; i < 2; i++)
		{
			string idx = i.ToString();
			bool has = i < m_aButtons.Count();
			RZ_UIW.Show(m_wRoot, "Action" + idx, has);
			RZ_UIW.Show(m_wRoot, "ActionNeed" + idx, false);
			if (!has)
				continue;

			array<string> f = m_aButtons[i];
			bool can = RZ_UIText.Int(f, 3) == 1;
			bool on = m_iRow == 1 && i == m_iCol;
			string need = RZ_UIText.Str(f, 4);
			RZ_UIW.SetText(m_wRoot, "ActionText" + idx, RZ_UIText.Str(f, 2));
			if (can)
				RZ_UIW.Tint(m_wRoot, "ActionText" + idx, amber);
			else
				RZ_UIW.Tint(m_wRoot, "ActionText" + idx, RZ_UIW.Rgb(110, 106, 96));

			RZ_UIW.Show(m_wRoot, "ActionBar" + idx, on);
			RZ_UIW.Tint(m_wRoot, "ActionBar" + idx, amber);
			RZ_UIW.Show(m_wRoot, "ActionLit" + idx, on);
			RZ_UIW.SetText(m_wRoot, "ActionNeed" + idx, need);
			RZ_UIW.Show(m_wRoot, "ActionNeed" + idx, !can && !need.IsEmpty());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Pad focus on what's selected, so the D-pad and A act on what's lit.
	protected void FocusSelected()
	{
		if (m_iRow == 0 && m_iCol >= 0 && m_iCol < m_aBreakerWidgets.Count())
			RZ_UIW.FocusWidget(m_aBreakerWidgets[m_iCol]);
		else if (m_iRow == 1)
			RZ_UIW.Focus(m_wRoot, "Action" + m_iCol.ToString());
	}
}
