//! The table (docs/UI.md, "The table"): the sorting game three duties share (the dead, the line at
//! the gate, the sample trays), drawn as a row of cards. Each card is one subject: its tell, three
//! signs to turn over and three verdicts. Opened by the server when the duty's work site is held
//! (Missions/Errands/RZ_DutyTable.c); every move goes back to the server, which holds the truth.
//!
//! Keys: left/right pick a card, up/down a sign, a verdict or the hand-in button, Enter turns the
//! sign over, gives the verdict or hands it in, Esc closes (the table keeps what you did). Pad:
//! D-pad or left stick, A, B. Mouse: click a sign, a verdict, the button, the close button. You
//! can't move while it's open; danger closes it (the server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_DutyMenu : RZ_MenuBase
{
	protected static const float MOVE_GAP = 0.55;	// the server takes a move at most every 0.5 s
	protected static const int ROW_DONE = 6;		// rows 0-2 the signs, 3-5 the verdicts, 6 the button

	protected static RZ_DutyMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static bool s_bFailed;				// the screen couldn't open here

	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected bool m_bDone;							// closed by the server: nothing to send back
	protected bool m_bJudged;
	protected int m_iRow;
	protected int m_iCol;
	protected float m_fMovedAt = -100;
	protected ref array<string> m_aHead = {};
	protected ref array<string> m_aLegend = {};
	protected ref array<string> m_aVerdicts = {};
	protected ref array<ref array<string>> m_aSubjects = {};
	protected ref array<Widget> m_aCards = {};

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A duty payload (RZ_PlayerUIComponent): opens or redraws the table; X|reason closes it.
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

		s_sData = payload;
		s_iVersion++;
		if (s_Open)
			return;

		// VERIFY: UI > Duty card layout's default (a layout written as text, its GUID in the .meta) resolves on a client without being set in RZ_GameMode.et
		if (!s_bFailed && !RZ_UISettings.Get().m_sDutyCardLayout.IsEmpty() && RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_DutyMenu))
			return;

		// No preset or layout here: the server judges the table itself, the careful way
		s_bFailed = true;
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		int serial = -1;
		foreach (array<string> f : recs)
		{
			if (RZ_UIText.Str(f, 0) == "T")
				serial = RZ_UIText.Int(f, 1);
		}

		ui.SendDutyMove(serial, RZ_DutyTables.MOVE_FAILED);
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
		RZ_UIW.OnClick(m_wRoot, "DoneButton", m_Click);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Turn over or choose");
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
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		if (s_Open == this)
			s_Open = null;

		// Esc, B, the close button, or another menu on top: the server keeps the table as it is
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendScreenClosed(RZ_ScreenGuard.KIND_DUTY);

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		return s_iVersion * 100 + m_iRow * 10 + m_iCol;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bJudged)
			return;

		m_iRow = Math.Max(0, m_iRow - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bJudged)
			return;

		m_iRow = Math.Min(ROW_DONE, m_iRow + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeft(float value = 0, EActionTrigger reason = 0)
	{
		m_iCol = Math.Max(0, m_iCol - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRight(float value = 0, EActionTrigger reason = 0)
	{
		m_iCol = Math.Min(Math.Max(0, m_aSubjects.Count() - 1), m_iCol + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		Act(m_iCol, m_iRow);
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a sign or a verdict does it; on the button hands the table in (judged: closes).
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "DoneButton")
		{
			Act(m_iCol, ROW_DONE);
			return;
		}

		int row = RowOf(name);
		if (row >= 0)
			Act(RZ_UIW.GetTag(w), row);
	}

	//------------------------------------------------------------------------------------------------
	//! "Sign1" is row 1, "Verdict2" row 5; -1 for anything else.
	protected static int RowOf(string name)
	{
		for (int i = 0; i < 3; i++)
		{
			if (name == "Sign" + i.ToString())
				return i;

			if (name == "Verdict" + i.ToString())
				return 3 + i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The thing at this card and row (the server checks it again).
	protected void Act(int col, int row)
	{
		if (row == ROW_DONE)
		{
			m_iRow = ROW_DONE;
			if (m_bJudged)
				Close();
			else
				SendMove(RZ_DutyTables.MOVE_DONE);

			return;
		}

		if (m_bJudged || col < 0 || col >= m_aSubjects.Count() || row < 0)
			return;

		m_iCol = col;
		m_iRow = row;
		SendMove(col * 10 + row);
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
			ui.SendDutyMove(m_iSerial, move);
	}

	//------------------------------------------------------------------------------------------------
	//! T|serial|kind|title|where|looks|judged|status; L|line; V|pass|check|stop;
	//! S|index|title|tell|verdict|result|name|word|look|name|word|look|name|word|look
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		RZ_UIW.SetText(m_wRoot, "DutyTitle", RZ_UIText.Str(m_aHead, 3));
		RZ_UIW.SetText(m_wRoot, "DutySub", RZ_UIText.Str(m_aHead, 4));
		for (int l = 0; l < 3; l++)
		{
			string line = "";
			if (l < m_aLegend.Count())
				line = m_aLegend[l];

			RZ_UIW.SetText(m_wRoot, "Legend" + l.ToString(), line);
		}

		int looks = RZ_UIText.Int(m_aHead, 5);
		RZ_UIW.SetText(m_wRoot, "LooksText", string.Format("LOOKS LEFT: %1", looks));
		RZ_UIW.Show(m_wRoot, "LooksText", !m_bJudged);
		if (looks > 0)
			RZ_UIW.Tint(m_wRoot, "LooksText", RZ_UIColor.Faction(RZ_EFaction.SURVIVORS));
		else
			RZ_UIW.Tint(m_wRoot, "LooksText", RZ_UIColor.Bad());

		string status = RZ_UIText.Str(m_aHead, 7);
		if (status.IsEmpty() && !m_bJudged)
			status = "Turn signs over while you have looks. Then give every one a verdict. Not sure? The last verdict is the careful one, for half.";

		RZ_UIW.SetText(m_wRoot, "DutyStatus", status);
		DrawCards();
		DrawDone();
		FocusSelected();
	}

	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aLegend.Clear();
		m_aVerdicts.Clear();
		m_aSubjects.Clear();
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "T")
			{
				m_aHead.Copy(f);
				m_iSerial = RZ_UIText.Int(f, 1);
				m_bJudged = RZ_UIText.Int(f, 6) == 1;
			}
			else if (tag == "L")
			{
				m_aLegend.Insert(RZ_UIText.Str(f, 1));
			}
			else if (tag == "V")
			{
				m_aVerdicts.Insert(RZ_UIText.Str(f, 1));
				m_aVerdicts.Insert(RZ_UIText.Str(f, 2));
				m_aVerdicts.Insert(RZ_UIText.Str(f, 3));
			}
			else if (tag == "S" && m_aSubjects.Count() < 5)
			{
				m_aSubjects.Insert(f);
			}
		}

		m_iCol = Math.Clamp(m_iCol, 0, Math.Max(0, m_aSubjects.Count() - 1));
		if (m_bJudged)
			m_iRow = ROW_DONE;
	}

	//------------------------------------------------------------------------------------------------
	//! One card a subject: the tell, the three signs (turned: its word, green, amber or red), the
	//! three verdicts (the one given is filled), and once judged how it went.
	protected void DrawCards()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Cards");
		RZ_UIW.ClearChildren(list);
		m_aCards.Clear();
		ResourceName layout = RZ_UISettings.Get().m_sDutyCardLayout;
		Color amber = RZ_UIColor.Faction(RZ_EFaction.SURVIVORS);
		foreach (int i, array<string> f : m_aSubjects)
		{
			Widget w = RZ_UIW.Create(layout, list);
			if (!w)
				break;

			int verdict = RZ_UIText.Int(f, 4);
			int result = RZ_UIText.Int(f, 5);
			RZ_UIW.SetText(w, "CardTitle", RZ_UIText.Str(f, 2));
			RZ_UIW.SetText(w, "CardTell", RZ_UIText.Str(f, 3));
			for (int s = 0; s < 3; s++)
			{
				string sn = s.ToString();
				int look = RZ_UIText.Int(f, 8 + s * 3);
				bool on = !m_bJudged && i == m_iCol && s == m_iRow;
				RZ_UIW.SetText(w, "SignName" + sn, RZ_UIText.Str(f, 6 + s * 3));
				RZ_UIW.SetText(w, "SignWord" + sn, RZ_UIText.Str(f, 7 + s * 3));
				RZ_UIW.Tint(w, "SignWord" + sn, SignColor(look));
				RZ_UIW.Show(w, "SignLit" + sn, on);
				RZ_UIW.Show(w, "SignBar" + sn, on);
				RZ_UIW.SetTag(RZ_UIW.Find(w, "Sign" + sn), i);
				RZ_UIW.OnClick(w, "Sign" + sn, m_Click);
			}

			for (int v = 0; v < 3; v++)
			{
				string vn = v.ToString();
				bool lit = !m_bJudged && i == m_iCol && 3 + v == m_iRow;
				string label = "";
				if (v < m_aVerdicts.Count())
					label = m_aVerdicts[v];

				RZ_UIW.SetText(w, "VerdictText" + vn, label);
				RZ_UIW.Show(w, "VerdictOn" + vn, verdict == v);
				RZ_UIW.Show(w, "VerdictLit" + vn, lit);
				RZ_UIW.Show(w, "VerdictBar" + vn, lit);
				if (verdict == v)
					RZ_UIW.Tint(w, "VerdictText" + vn, RZ_UIW.Rgb(22, 24, 27));
				else
					RZ_UIW.Tint(w, "VerdictText" + vn, RZ_UIColor.Bone());

				RZ_UIW.SetTag(RZ_UIW.Find(w, "Verdict" + vn), i);
				RZ_UIW.OnClick(w, "Verdict" + vn, m_Click);
			}

			RZ_UIW.SetText(w, "CardResult", ResultWord(result));
			RZ_UIW.Tint(w, "CardResult", ResultColor(result));
			RZ_UIW.Show(w, "CardResult", m_bJudged);
			RZ_UIW.Tint(w, "CardBar", amber);
			if (m_bJudged)
				RZ_UIW.Tint(w, "CardBar", ResultColor(result));

			RZ_UIW.SetTag(w, i);
			m_aCards.Insert(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static Color SignColor(int look)
	{
		if (look == RZ_DutyTable.SIGN_CLEAN)
			return RZ_UIColor.Good();

		if (look == RZ_DutyTable.SIGN_ODD)
			return RZ_UIColor.Hazard();

		if (look == RZ_DutyTable.SIGN_BAD)
			return RZ_UIColor.Bad();

		return RZ_UIColor.Dim();
	}

	//------------------------------------------------------------------------------------------------
	protected static string ResultWord(int result)
	{
		if (result == RZ_DutyTable.RESULT_RIGHT)
			return "RIGHT";

		if (result == RZ_DutyTable.RESULT_HALF)
			return "TOO CAREFUL";

		if (result == RZ_DutyTable.RESULT_WRONG)
			return "MISSED";

		if (result == RZ_DutyTable.RESULT_DANGER)
			return "LET THROUGH";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static Color ResultColor(int result)
	{
		if (result == RZ_DutyTable.RESULT_RIGHT)
			return RZ_UIColor.Good();

		if (result == RZ_DutyTable.RESULT_HALF)
			return RZ_UIColor.Hazard();

		return RZ_UIColor.Bad();
	}

	//------------------------------------------------------------------------------------------------
	//! The button: "Hand it in" (dark until every card has a verdict), then "Close".
	protected void DrawDone()
	{
		bool all = !m_aSubjects.IsEmpty();
		foreach (array<string> f : m_aSubjects)
		{
			if (RZ_UIText.Int(f, 4) < 0)
				all = false;
		}

		bool on = m_iRow == ROW_DONE;
		string label = "Hand it in";
		if (m_bJudged)
			label = "Close";

		RZ_UIW.SetText(m_wRoot, "DoneText", label);
		if (all || m_bJudged)
			RZ_UIW.Tint(m_wRoot, "DoneText", RZ_UIColor.Faction(RZ_EFaction.SURVIVORS));
		else
			RZ_UIW.Tint(m_wRoot, "DoneText", RZ_UIW.Rgb(110, 106, 96));

		RZ_UIW.Show(m_wRoot, "DoneLit", on);
		RZ_UIW.Show(m_wRoot, "DoneBar", on);
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: the sign, verdict or button under the cursor becomes the selected one, drawn in place.
	override protected bool OnHoverRow(Widget w, string name, int tag, bool on)
	{
		int row = RowOf(name);
		if (name == "DoneButton")
			row = ROW_DONE;

		if (row < 0)
			return false;

		if (!on || m_bJudged)
			return true;

		int col = m_iCol;
		if (row != ROW_DONE)
			col = tag;

		if (col < 0 || col >= m_aSubjects.Count() || (row == m_iRow && col == m_iCol))
			return true;

		m_iRow = row;
		m_iCol = col;
		foreach (int i, Widget card : m_aCards)
		{
			for (int s = 0; s < 3; s++)
			{
				bool sign = i == m_iCol && s == m_iRow;
				RZ_UIW.Show(card, "SignLit" + s.ToString(), sign);
				RZ_UIW.Show(card, "SignBar" + s.ToString(), sign);
				bool verdict = i == m_iCol && 3 + s == m_iRow;
				RZ_UIW.Show(card, "VerdictLit" + s.ToString(), verdict);
				RZ_UIW.Show(card, "VerdictBar" + s.ToString(), verdict);
			}
		}

		DrawDone();
		RZ_UIW.FocusWidget(w);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Pad focus on what's selected, so the D-pad and A act on what's lit.
	protected void FocusSelected()
	{
		if (m_iRow == ROW_DONE || m_iCol < 0 || m_iCol >= m_aCards.Count())
		{
			RZ_UIW.Focus(m_wRoot, "DoneButton");
			return;
		}

		int verdict = m_iRow - 3;
		if (m_iRow <= 2)
			RZ_UIW.Focus(m_aCards[m_iCol], "Sign" + m_iRow.ToString());
		else
			RZ_UIW.Focus(m_aCards[m_iCol], "Verdict" + verdict.ToString());
	}
}
