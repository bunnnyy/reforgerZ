//! The car repair screen (docs/UI.md, "The car repair screen"), as approved: a job card (each job,
//! what you need, the button that does it) beside a rough side view of the car with each job lit
//! where it is. Opened by the server when a player looks over a fixable car (RZ_Cars.Inspect);
//! every job goes back to the server, which checks it like the car's actions (RZ_Cars.DoJob).
//!
//! Keys: up/down pick a job, Enter does it, T starts the engine, Esc closes. Pad: D-pad or left
//! stick, A does it, Y starts the engine, B closes. Mouse: click a job to do it, the Start button,
//! the close button. You can't move while it's open; a job under way goes on, and danger closes
//! the screen (the server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_CarMenu : RZ_MenuBase
{
	protected static RZ_CarMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static float s_fDataAt;
	protected static bool s_bFailed;			// the menu couldn't open here: the actions come back
	protected static float s_fFallbackUntil;	// the server was at its cap: the actions for a minute

	protected int m_iSelected;
	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected bool m_bDone;						// closed by the server: nothing to send back
	protected bool m_bCanStart;
	protected int m_iWorking = -1;
	protected float m_fLeft;
	protected float m_fTotal;
	protected ref array<string> m_aHead = {};
	protected ref array<ref array<string>> m_aJobs = {};
	protected ref array<Widget> m_aRows = {};

	//------------------------------------------------------------------------------------------------
	//! The repair screen is used on this machine: UI > Car job layout set (the server checks the
	//! same), it hasn't failed to open here, and the server isn't at its cap of open screens.
	static bool Available()
	{
		return !s_bFailed && RZ_UIW.Now() >= s_fFallbackUntil && !RZ_UISettings.Get().m_sCarJobLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! A fixable car's action slots hide while the screen is used: they're its jobs.
	static bool HidesSlotsOn(IEntity owner)
	{
		if (!owner || !Available())
			return false;

		RZ_ClueComponent clue = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		return clue && clue.GetReadLabel() == RZ_Cars.LOOK_LABEL;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A car payload (RZ_PlayerUIComponent): opens or redraws the screen; X|reason closes it;
	//! F|why: the server couldn't open one, so the car's actions show for a minute.
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
		if (s_Open || RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_CarMenu))
			return;

		// No preset or layout here: what a look over the car says as a card, and the actions back
		s_bFailed = true;
		ui.SendScreenClosed(RZ_ScreenGuard.KIND_CAR);
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		string title = "The car";
		string text = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "C")
				title = RZ_UIText.Str(f, 2);
			else if (tag == "I")
				text = RZ_UIText.Str(f, 1);
		}

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && !text.IsEmpty())
			gm.ShowNotice(title, text, RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.AddActionListener("RZ_CarStart", EActionTrigger.DOWN, OnStart);
		RZ_UIW.OnClick(m_wRoot, "StartButton", m_Click);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Do it");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "RZ_CarStart", "Start the engine") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		RZ_UIW.Texture(m_wRoot, "CarPicture", RZ_UISettings.Get().m_sCarPicture);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.RemoveActionListener("RZ_CarStart", EActionTrigger.DOWN, OnStart);
		if (s_Open == this)
			s_Open = null;

		// Esc, B, the close button, or another menu on top: the server forgets the screen
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendScreenClosed(RZ_ScreenGuard.KIND_CAR);

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! New data, a new selection, and once a second while a job is under way (its countdown).
	override protected int DataVersion()
	{
		int tick = 0;
		if (m_iWorking >= 0)
			tick = RZ_Util.RoundToInt(Math.Floor(RZ_UIW.Now())) % 1000;

		return s_iVersion * 100000 + tick * 10 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Max(0, m_iSelected - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Min(Math.Max(0, m_aJobs.Count() - 1), m_iSelected + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		Do(m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnStart(float value = 0, EActionTrigger reason = 0)
	{
		if (!m_bCanStart)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCarJob(m_iSerial, RZ_Cars.JOB_START);
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a job row does it; the Start button starts the engine.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "JobRow")
			Do(RZ_UIW.GetTag(w));
		else if (name == "StartButton")
			OnStart();
	}

	//------------------------------------------------------------------------------------------------
	//! Do job row `index` if it has a button (the server checks it again).
	protected void Do(int index)
	{
		if (index < 0 || index >= m_aJobs.Count())
			return;

		m_iSelected = index;
		array<string> f = m_aJobs[index];
		if (RZ_UIText.Str(f, 5).IsEmpty())
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCarJob(m_iSerial, RZ_UIText.Int(f, 1));
	}

	//------------------------------------------------------------------------------------------------
	//! C|serial|title|done|jobs|status|can start|start need; I|text; W|job|left|total; J|job|state|name|need|button
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		RZ_UIW.SetText(m_wRoot, "CarTitle", RZ_UIText.Str(m_aHead, 2));
		RZ_UIW.SetText(m_wRoot, "CarDone", string.Format("%1 of %2 done", RZ_UIText.Int(m_aHead, 3), RZ_UIText.Int(m_aHead, 4)));
		string status = RZ_UIText.Str(m_aHead, 5);
		RZ_UIW.SetText(m_wRoot, "CarStatus", status);
		RZ_UIW.Show(m_wRoot, "CarStatus", !status.IsEmpty());

		m_bCanStart = RZ_UIText.Int(m_aHead, 6) == 1;
		string startNeed = RZ_UIText.Str(m_aHead, 7);
		RZ_UIW.Tint(m_wRoot, "StartText", StartColour());
		RZ_UIW.Tint(m_wRoot, "StartBg", StartBack());
		RZ_UIW.SetText(m_wRoot, "StartNeed", startNeed);
		RZ_UIW.Show(m_wRoot, "StartNeed", !m_bCanStart && !startNeed.IsEmpty());
		DrawJobs();
	}

	//------------------------------------------------------------------------------------------------
	protected Color StartColour()
	{
		if (m_bCanStart)
			return RZ_UIW.Rgb(21, 23, 26);

		return RZ_UIW.Rgb(110, 106, 96);
	}

	//------------------------------------------------------------------------------------------------
	protected Color StartBack()
	{
		if (m_bCanStart)
			return RZ_UIColor.Faction(RZ_EFaction.SURVIVORS);

		return RZ_UIW.Rgb(27, 31, 34);
	}

	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aJobs.Clear();
		m_iWorking = -1;
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "C")
			{
				m_aHead.Copy(f);
				m_iSerial = RZ_UIText.Int(f, 1);
			}
			else if (tag == "W")
			{
				m_iWorking = RZ_UIText.Int(f, 1);
				m_fLeft = RZ_UIText.Float(f, 2);
				m_fTotal = RZ_UIText.Float(f, 3);
			}
			else if (tag == "J" && m_aJobs.Count() < RZ_Cars.JOBS)
			{
				m_aJobs.Insert(f);
			}
		}

		m_iSelected = Math.Clamp(m_iSelected, 0, Math.Max(0, m_aJobs.Count() - 1));
	}

	//------------------------------------------------------------------------------------------------
	//! Olive done, amber can do now or under way, grey when something's needed first.
	static Color StateColour(int state)
	{
		if (state == RZ_Cars.STATE_DONE)
			return RZ_UIColor.Faction(RZ_EFaction.MILITARY);

		if (state == RZ_Cars.STATE_NEED)
			return RZ_UIW.Rgb(110, 106, 96);

		return RZ_UIColor.Faction(RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawJobs()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Jobs");
		RZ_UIW.ClearChildren(list);
		m_aRows.Clear();
		ResourceName rowLayout = RZ_UISettings.Get().m_sCarJobLayout;
		float left = Math.Max(0, m_fLeft - (RZ_UIW.Now() - s_fDataAt));
		foreach (int i, array<string> f : m_aJobs)
		{
			int job = RZ_UIText.Int(f, 1);
			int state = RZ_UIText.Int(f, 2);
			bool on = i == m_iSelected;
			Color c = StateColour(state);
			string need = RZ_UIText.Str(f, 4);
			string button = RZ_UIText.Str(f, 5);
			if (state == RZ_Cars.STATE_WORKING)
				need = string.Format("%1... %2 s", need, RZ_Util.RoundToInt(Math.Ceil(left)));

			// The marker on the car: its colour, and the ring on the selected job
			string idx = job.ToString();
			RZ_UIW.Tint(m_wRoot, "MarkerDot" + idx, c);
			RZ_UIW.Show(m_wRoot, "MarkerRing" + idx, on);
			RZ_UIW.Tint(m_wRoot, "MarkerRing" + idx, c);

			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				continue;

			int num = job + 1;
			RZ_UIW.SetText(row, "JobNum", num.ToString());
			RZ_UIW.Tint(row, "JobDot", c);
			RZ_UIW.SetText(row, "JobName", RZ_UIText.Str(f, 3));
			RZ_UIW.SetText(row, "JobNeed", need);
			RZ_UIW.SetText(row, "JobButton", button);
			RZ_UIW.Show(row, "JobButtonBox", !button.IsEmpty());
			RZ_UIW.Show(row, "JobBg", on);
			RZ_UIW.Show(row, "JobBar", on);
			RZ_UIW.Tint(row, "JobBar", RZ_UIColor.Faction(RZ_EFaction.SURVIVORS));
			bool working = state == RZ_Cars.STATE_WORKING && m_fTotal > 0;
			RZ_UIW.Show(row, "JobProgress", working);
			if (working)
				RZ_UIW.Progress(row, "JobProgress", 1 - left / m_fTotal);

			Color fg = RZ_UIColor.Bone();
			if (state == RZ_Cars.STATE_NEED)
				fg = RZ_UIW.Rgb(163, 158, 144);
			else if (on)
				fg = RZ_UIW.Rgb(244, 240, 230);

			RZ_UIW.Tint(row, "JobName", fg);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRows.Insert(row);
		}

		// Pad focus on the selected job, so the D-pad and A act on what's lit
		if (m_iSelected >= 0 && m_iSelected < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[m_iSelected]);
	}
}
