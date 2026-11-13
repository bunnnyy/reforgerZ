//! The car repair screen (docs/UI.md, "The car repair screen"): a job card (each job, what the car
//! has and what it lacks, what you need, the button that does it) beside a rough side view of the
//! car with each job lit where it is. Opened by the server when a player looks over a fixable car
//! (RZ_Cars.Inspect); every job goes back to the server, which checks it like the car's actions
//! (RZ_Cars.DoJob).
//!
//! Hotwire pulls the panel under the column: the loom takes the picture's place, five wires and
//! the sticker's lines, and you cut the right two (Interactions/RZ_CarLoom.c; the server decides
//! every cut). Back leaves the loom as it is and returns to the jobs.
//!
//! Keys: up/down pick a job, Enter does it, T starts the engine, Esc closes. At the loom:
//! left/right pick a wire, Enter cuts it, Esc goes back to the jobs. Pad: D-pad or left stick, A,
//! Y starts the engine, B. Mouse: click a job to do it or a wire to cut it, the Start button, the
//! close button. You can't move while it's open; a job under way goes on, and danger closes the
//! screen (the server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_CarMenu : RZ_MenuBase
{
	protected static const float CUT_GAP = 0.6;			// the server takes a pick at most every 0.5 s
	protected static const float STATUS_X = 48;
	protected static const float STATUS_Y = 392;		// CarStatus: under the car picture
	protected static const float STATUS_Y_LOOM = 574;	// and under the loom's sticker
	protected static const float WIRE_BOX = 120;		// a wire's box, across
	protected static const float WIRE_THIN = 14;
	protected static const float WIRE_THICK = 30;

	protected static RZ_CarMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static float s_fDataAt;
	protected static bool s_bFailed;			// the menu couldn't open here: the actions come back
	protected static float s_fFallbackUntil;	// the server was at its cap: the actions for a minute

	protected int m_iSelected;
	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected int m_iLocal;						// bumps when only this client changed something
	protected bool m_bDone;						// closed by the server: nothing to send back
	protected bool m_bCanStart;
	protected int m_iWorking = -1;
	protected float m_fLeft;
	protected float m_fTotal;
	protected bool m_bWantLoom;					// Hotwire was picked: the loom shows once the server sends it
	protected bool m_bLoom;						// the loom is up, in the picture's place
	protected int m_iWire;						// the wire picked at the loom
	protected float m_fCutAt = -100;
	protected string m_sLocalStatus;			// said here, not by the server (the feed isn't cut)
	protected ref array<string> m_aHead = {};
	protected ref array<string> m_aLoomHead = {};
	protected ref array<ref array<string>> m_aJobs = {};
	protected ref array<ref array<string>> m_aWires = {};
	protected ref array<string> m_aClues = {};
	protected ref array<Widget> m_aRows = {};
	protected Widget m_wSelectPrompt;			// the footer's A: Do it, or Cut at the loom
	protected Widget m_wStartPrompt;
	protected Widget m_wBackPrompt;				// Close, or Back to the jobs at the loom

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
		input.AddActionListener("MenuLeft", EActionTrigger.DOWN, OnLeft);
		input.AddActionListener("MenuRight", EActionTrigger.DOWN, OnRight);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.AddActionListener("RZ_CarStart", EActionTrigger.DOWN, OnStart);
		RZ_UIW.OnClick(m_wRoot, "StartButton", m_Click);
		for (int i = 0; i < RZ_CarLoom.WIRES; i++)
		{
			string wire = "Wire" + i.ToString();
			RZ_UIW.SetTag(RZ_UIW.Find(m_wRoot, wire), i);
			RZ_UIW.OnClick(m_wRoot, wire, m_Click);
		}

		m_wSelectPrompt = RZ_StatusStrip.MakePrompt(m_wRoot, "Prompts", "MenuSelect", "Do it");
		m_wStartPrompt = RZ_StatusStrip.MakePrompt(m_wRoot, "Prompts", "RZ_CarStart", "Start the engine");
		m_wBackPrompt = RZ_StatusStrip.MakePrompt(m_wRoot, "Prompts", "MenuBack", "Close");
		bool prompts = m_wSelectPrompt != null && m_wStartPrompt != null && m_wBackPrompt != null;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		RZ_UIW.Texture(m_wRoot, "CarPicture", RZ_UISettings.Get().m_sCarPicture);
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
	//! New data, a new selection (a job, or a wire at the loom), something said here, and once a
	//! second while a job is under way (its countdown).
	override protected int DataVersion()
	{
		int tick = 0;
		if (m_iWorking >= 0)
			tick = RZ_Util.RoundToInt(Math.Floor(RZ_UIW.Now())) % 1000;

		return s_iVersion * 1000000 + (m_iLocal % 10) * 100000 + tick * 100 + m_iWire * 10 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		if (!m_bLoom)
			m_iSelected = Math.Max(0, m_iSelected - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		if (!m_bLoom)
			m_iSelected = Math.Min(Math.Max(0, m_aJobs.Count() - 1), m_iSelected + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeft(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bLoom)
			m_iWire = Math.Max(0, m_iWire - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRight(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bLoom)
			m_iWire = Math.Min(Math.Max(0, m_aWires.Count() - 1), m_iWire + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		if (m_bLoom)
			CutWire(m_iWire);
		else
			Do(m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnStart(float value = 0, EActionTrigger reason = 0)
	{
		if (!m_bCanStart || m_bLoom)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCarJob(m_iSerial, RZ_Cars.JOB_START);
	}

	//------------------------------------------------------------------------------------------------
	//! Esc or B: at the loom, back to the jobs (the loom stays as it is on the car); else close.
	override protected void Back()
	{
		if (!m_bLoom)
		{
			Close();
			return;
		}

		m_bWantLoom = false;
		m_bLoom = false;
		m_sLocalStatus = "";
		m_iLocal++;
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a job row does it, on a wire cuts it; the Start button starts the engine.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "StartButton")
		{
			OnStart();
			return;
		}

		if (IsWireName(name))
		{
			CutWire(RZ_UIW.GetTag(w));
			return;
		}

		if (name == "JobRow" && !m_bLoom)
			Do(RZ_UIW.GetTag(w));
	}

	//------------------------------------------------------------------------------------------------
	//! "Wire0" to "Wire4": the buttons over the loom's wires.
	protected static bool IsWireName(string name)
	{
		return name.Length() == 5 && name.StartsWith("Wire");
	}

	//------------------------------------------------------------------------------------------------
	//! Do job row `index` if it has a button (the server checks it again). The ignition's button
	//! may be Hotwire: the loom shows when the server sends it.
	protected void Do(int index)
	{
		if (index < 0 || index >= m_aJobs.Count())
			return;

		m_iSelected = index;
		array<string> f = m_aJobs[index];
		if (RZ_UIText.Str(f, 5).IsEmpty())
			return;

		int job = RZ_UIText.Int(f, 1);
		if (job == RZ_Cars.JOB_IGNITION)
			m_bWantLoom = true;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCarJob(m_iSerial, job);
	}

	//------------------------------------------------------------------------------------------------
	//! Cut the wire at `index` of the loom (the server decides what it was). The feed is the one
	//! everybody knows: it's said here and never sent.
	protected void CutWire(int index)
	{
		if (!m_bLoom || index < 0 || index >= m_aWires.Count())
			return;

		m_iWire = index;
		array<string> f = m_aWires[index];
		if (RZ_UIText.Int(f, 5) != RZ_CarLoom.CUT_NONE)
			return;

		if (RZ_UIText.Int(f, 3) == 1)
		{
			m_sLocalStatus = "That's the battery feed. Cut that and the whole loom shorts. Leave it.";
			m_iLocal++;
			return;
		}

		float now = RZ_UIW.Now();
		if (now - m_fCutAt < CUT_GAP)
			return;

		m_fCutAt = now;
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCarJob(m_iSerial, RZ_Cars.JOB_CUT + RZ_UIText.Int(f, 1));
	}

	//------------------------------------------------------------------------------------------------
	//! C|serial|title|done|jobs|status|can start|start need|what's missing|1 if anything is;
	//! I|text; W|job|left|total; J|job|state|name|need|button|tag|tone; and at the loom
	//! L|wrong cuts|allowed|right cuts, K|place|colour|thick|striped|cut|label, Q|sticker line
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		RZ_UIW.SetText(m_wRoot, "CarTitle", RZ_UIText.Str(m_aHead, 2));
		string missing = RZ_UIText.Str(m_aHead, 8);
		Color missingColour = ToneColour(RZ_Cars.TONE_MISSING);
		if (missing.IsEmpty())
			missing = string.Format("%1 of %2 done", RZ_UIText.Int(m_aHead, 3), RZ_UIText.Int(m_aHead, 4));

		if (RZ_UIText.Int(m_aHead, 9) != 1)
			missingColour = ToneColour(RZ_Cars.TONE_HAVE);

		RZ_UIW.SetText(m_wRoot, "CarDone", missing);
		RZ_UIW.Tint(m_wRoot, "CarDone", missingColour);
		string status = m_sLocalStatus;
		if (status.IsEmpty())
			status = RZ_UIText.Str(m_aHead, 5);

		RZ_UIW.SetText(m_wRoot, "CarStatus", status);
		RZ_UIW.Show(m_wRoot, "CarStatus", !status.IsEmpty());

		m_bCanStart = RZ_UIText.Int(m_aHead, 6) == 1;
		string startNeed = RZ_UIText.Str(m_aHead, 7);
		RZ_UIW.Tint(m_wRoot, "StartText", StartColour());
		RZ_UIW.Tint(m_wRoot, "StartBg", StartBack());
		RZ_UIW.SetText(m_wRoot, "StartNeed", startNeed);
		RZ_UIW.Show(m_wRoot, "StartNeed", !m_bCanStart && !startNeed.IsEmpty());
		DrawLoom();
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
		m_aLoomHead.Clear();
		m_aJobs.Clear();
		m_aWires.Clear();
		m_aClues.Clear();
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
			else if (tag == "L")
			{
				m_aLoomHead.Copy(f);
			}
			else if (tag == "K" && m_aWires.Count() < RZ_CarLoom.WIRES)
			{
				m_aWires.Insert(f);
			}
			else if (tag == "Q")
			{
				m_aClues.Insert(RZ_UIText.Str(f, 1));
			}
		}

		// The server's word replaces what this client said. No loom in it: the car is hotwired, the
		// loom shorted, or Hotwire meant the key; back to the jobs
		m_sLocalStatus = "";
		bool wasLoom = m_bLoom;
		if (m_aWires.IsEmpty())
			m_bWantLoom = false;

		m_bLoom = m_bWantLoom;
		if (m_bLoom && !wasLoom)
			m_iWire = FirstWire();

		m_iWire = Math.Clamp(m_iWire, 0, Math.Max(0, m_aWires.Count() - 1));
		m_iSelected = Math.Clamp(m_iSelected, 0, Math.Max(0, m_aJobs.Count() - 1));
	}

	//------------------------------------------------------------------------------------------------
	//! The first wire worth a look: not the feed, not cut.
	protected int FirstWire()
	{
		foreach (int i, array<string> f : m_aWires)
		{
			if (RZ_UIText.Int(f, 3) != 1 && RZ_UIText.Int(f, 5) == RZ_CarLoom.CUT_NONE)
				return i;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A job's marker and number square: olive done, amber you can do it now (or are), red it's
	//! missing and you've nothing for it.
	static Color StateColour(int state)
	{
		if (state == RZ_Cars.STATE_DONE)
			return ToneColour(RZ_Cars.TONE_HAVE);

		if (state == RZ_Cars.STATE_NEED)
			return ToneColour(RZ_Cars.TONE_MISSING);

		return ToneColour(RZ_Cars.TONE_NOW);
	}

	//------------------------------------------------------------------------------------------------
	//! A tag's colour: olive it's there, red it isn't (the status strip's off, #D9574C), amber
	//! under way.
	static Color ToneColour(int tone)
	{
		if (tone == RZ_Cars.TONE_HAVE)
			return RZ_UIColor.Faction(RZ_EFaction.MILITARY);

		if (tone == RZ_Cars.TONE_MISSING)
			return RZ_UIW.Rgb(217, 87, 76);

		return RZ_UIColor.Faction(RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawJobs()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Jobs");
		RZ_UIW.ClearChildren(list);
		m_aRows.Clear();
		ResourceName rowLayout = RZ_UISettings.Get().m_sCarJobLayout;
		bool icons = !RZ_UISettings.Get().m_sIconImageset.IsEmpty();
		float left = Math.Max(0, m_fLeft - (RZ_UIW.Now() - s_fDataAt));
		foreach (int i, array<string> f : m_aJobs)
		{
			int job = RZ_UIText.Int(f, 1);
			int state = RZ_UIText.Int(f, 2);
			bool on = i == m_iSelected && !m_bLoom;
			Color c = StateColour(state);
			string need = RZ_UIText.Str(f, 4);
			string button = RZ_UIText.Str(f, 5);
			string tag = RZ_UIText.Str(f, 6);
			int tone = RZ_UIText.Int(f, 7);
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
			RZ_UIW.Tint(row, "JobBar", RZ_UIColor.Faction(RZ_EFaction.SURVIVORS));

			// What the car has, in words and a colour: a tick when it's there, a cross when it isn't
			RZ_UIW.Show(row, "JobTagBox", !tag.IsEmpty());
			RZ_UIW.SetText(row, "JobTag", tag);
			RZ_UIW.Tint(row, "JobTagBg", ToneColour(tone));
			bool tick = tone == RZ_Cars.TONE_HAVE && icons;
			RZ_UIW.Show(row, "JobTagTick", tick);
			if (tick)
				RZ_UIW.Icon(row, "JobTagTick", "track_done");

			RZ_UIW.Show(row, "JobTagCross", tone == RZ_Cars.TONE_MISSING);

			bool working = state == RZ_Cars.STATE_WORKING && m_fTotal > 0;
			RZ_UIW.Show(row, "JobProgress", working);
			if (working)
				RZ_UIW.Progress(row, "JobProgress", 1 - left / m_fTotal);

			LightRow(row, i);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRows.Insert(row);
		}

		// Pad focus on what's lit, so the D-pad and A act on it: the selected job, or at the loom
		// the picked wire
		if (m_bLoom)
			RZ_UIW.Focus(m_wRoot, "Wire" + m_iWire.ToString());
		else if (m_iSelected >= 0 && m_iSelected < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[m_iSelected]);
	}

	//------------------------------------------------------------------------------------------------
	//! Job `i`'s row lit when it's selected (a lighter fill, the amber bar, bright text), and the
	//! ring on its marker on the car. No job is lit while the loom is up.
	protected void LightRow(Widget row, int i)
	{
		array<string> f = m_aJobs[i];
		bool on = i == m_iSelected && !m_bLoom;
		int job = RZ_UIText.Int(f, 1);
		RZ_UIW.Show(m_wRoot, "MarkerRing" + job.ToString(), on);
		RZ_UIW.Show(row, "JobBg", on);
		RZ_UIW.Show(row, "JobBar", on);
		Color fg = RZ_UIColor.Bone();
		if (on)
			fg = RZ_UIW.Rgb(244, 240, 230);

		RZ_UIW.Tint(row, "JobName", fg);
	}

	// ================================================================ The loom
	//------------------------------------------------------------------------------------------------
	//! The loom in the picture's place while it's up: the wires, the sticker's lines and the count
	//! of wrong cuts; what the last cut said moves under it, and the footer's A and B say Cut and
	//! Back to the jobs.
	protected void DrawLoom()
	{
		RZ_UIW.Show(m_wRoot, "CarView", !m_bLoom);
		RZ_UIW.Show(m_wRoot, "Loom", m_bLoom);
		float statusY = STATUS_Y;
		if (m_bLoom)
			statusY = STATUS_Y_LOOM;

		RZ_UIW.SetPos(RZ_UIW.Find(m_wRoot, "CarStatus"), STATUS_X, statusY); // VERIFY: FrameSlot.SetPos moves a text widget placed by a frame slot (as the object menu's body)
		if (!m_bLoom)
		{
			RZ_StatusStrip.SetPrompt(m_wSelectPrompt, "Do it", true);
			RZ_StatusStrip.SetPrompt(m_wStartPrompt, "", m_bCanStart);
			RZ_StatusStrip.SetPrompt(m_wBackPrompt, "Close", true);
			RZ_UIW.SetText(m_wRoot, "PromptsText", "UP / DOWN  PICK    A  DO IT    Y  START    B  CLOSE");
			return;
		}

		RZ_StatusStrip.SetPrompt(m_wSelectPrompt, "Cut", true);
		RZ_StatusStrip.SetPrompt(m_wStartPrompt, "", false);
		RZ_StatusStrip.SetPrompt(m_wBackPrompt, "Back to the jobs", true);
		RZ_UIW.SetText(m_wRoot, "PromptsText", "LEFT / RIGHT  PICK A WIRE    A  CUT    B  BACK TO THE JOBS");

		int strikes = RZ_UIText.Int(m_aLoomHead, 1);
		RZ_UIW.SetText(m_wRoot, "LoomStrikes", string.Format("WRONG CUTS %1 OF %2", strikes, RZ_UIText.Int(m_aLoomHead, 2)));
		Color strikeColour = RZ_UIColor.Dim();
		if (strikes > 0)
			strikeColour = ToneColour(RZ_Cars.TONE_MISSING);

		RZ_UIW.Tint(m_wRoot, "LoomStrikes", strikeColour);
		string clues = "";
		foreach (int n, string clue : m_aClues)
		{
			if (n > 0)
				clues += "\n";

			clues += clue;
		}

		RZ_UIW.SetText(m_wRoot, "LoomClues", clues);
		foreach (int i, array<string> f : m_aWires)
		{
			DrawWire(i, f);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One wire: its colour (and its name in words under it), thick or striped or plain, cut or
	//! whole, and what it's known to be. Colour is never the only thing that tells two apart.
	protected void DrawWire(int i, array<string> f)
	{
		string idx = i.ToString();
		int colour = RZ_UIText.Int(f, 2);
		bool thick = RZ_UIText.Int(f, 3) == 1;
		bool striped = RZ_UIText.Int(f, 4) == 1;
		int cut = RZ_UIText.Int(f, 5);
		string label = RZ_UIText.Str(f, 6);
		bool on = i == m_iWire;
		Color c = WireColour(colour);
		RZ_UIW.Show(m_wRoot, "WireBg" + idx, on);
		RZ_UIW.Show(m_wRoot, "WireFocus" + idx, on);

		// The wire: two halves and the middle, which goes when it's cut
		float w = WIRE_THIN;
		if (thick)
			w = WIRE_THICK;

		float x = (WIRE_BOX - w) * 0.5;
		Bar("WireTop" + idx, x, 34, w, 70, c);
		Bar("WireMid" + idx, x, 104, w, 20, c);
		Bar("WireBot" + idx, x, 124, w, 70, c);
		RZ_UIW.Show(m_wRoot, "WireMid" + idx, cut == RZ_CarLoom.CUT_NONE);

		// Cut ends: bright copper where it was stripped, soot where it shorted
		Color ends = RZ_UIW.Rgb(214, 148, 84);
		if (cut == RZ_CarLoom.CUT_BAD)
			ends = RZ_UIW.Rgb(70, 64, 58);

		Bar("WireEndA" + idx, x - 3, 98, w + 6, 8, ends);
		Bar("WireEndB" + idx, x - 3, 122, w + 6, 8, ends);
		RZ_UIW.Show(m_wRoot, "WireEndA" + idx, cut != RZ_CarLoom.CUT_NONE);
		RZ_UIW.Show(m_wRoot, "WireEndB" + idx, cut != RZ_CarLoom.CUT_NONE);
		RZ_UIW.Show(m_wRoot, "WireStripes" + idx, striped);

		string name = RZ_CarLoom.ColourWord(colour);
		name.ToUpper();
		RZ_UIW.SetText(m_wRoot, "WireName" + idx, name);
		string kind = "plain";
		if (thick)
			kind = "thick";
		else if (striped)
			kind = "striped";

		RZ_UIW.SetText(m_wRoot, "WireKind" + idx, kind);

		// What it's known to be: the feed, a wire once it's cut, a Mechanic's horn
		Color known = RZ_UIColor.Dim();
		if (cut == RZ_CarLoom.CUT_GOOD)
			known = ToneColour(RZ_Cars.TONE_HAVE);
		else if (cut == RZ_CarLoom.CUT_BAD)
			known = ToneColour(RZ_Cars.TONE_MISSING);
		else if (!thick && !label.IsEmpty())
			known = RZ_UIColor.Hazard();

		RZ_UIW.SetText(m_wRoot, "WireLabel" + idx, label);
		RZ_UIW.Tint(m_wRoot, "WireLabel" + idx, known);
	}

	//------------------------------------------------------------------------------------------------
	//! Places, sizes and tints one of a wire's bars in its box.
	//! VERIFY: FrameSlot.SetPos and SetSize on an image in a frame (anchored top left) move and
	//! size it as the layout's PositionX/Y and SizeX/Y would.
	protected void Bar(string name, float x, float y, float w, float h, Color c)
	{
		Widget bar = RZ_UIW.Find(m_wRoot, name);
		RZ_UIW.SetPos(bar, x, y);
		RZ_UIW.SetSize(bar, w, h);
		RZ_UIW.Tint(m_wRoot, name, c);
	}

	//------------------------------------------------------------------------------------------------
	//! A wire's colour on the screen (RZ_CarLoom.ColourWord names it).
	static Color WireColour(int colour)
	{
		switch (colour)
		{
			case 0: return RZ_UIW.Rgb(200, 53, 43);
			case 1: return RZ_UIW.Rgb(242, 194, 48);
			case 2: return RZ_UIW.Rgb(94, 158, 92);
			case 3: return RZ_UIW.Rgb(79, 140, 200);
		}

		return RZ_UIW.Rgb(232, 226, 208);
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: the job (or at the loom, the wire) under the cursor becomes the selected one (a click
	//! does it, and A or Enter then does the same), drawn in place.
	override protected bool OnHoverRow(Widget w, string name, int tag, bool on)
	{
		if (IsWireName(name))
		{
			if (!on || !m_bLoom || tag < 0 || tag >= m_aWires.Count() || tag == m_iWire)
				return true;

			m_iWire = tag;
			foreach (int i, array<string> f : m_aWires)
			{
				DrawWire(i, f);
			}

			RZ_UIW.FocusWidget(w);
			return true;
		}

		if (name != "JobRow")
			return false;

		if (!on || m_bLoom || tag < 0 || tag >= m_aJobs.Count() || tag == m_iSelected)
			return true;

		m_iSelected = tag;
		foreach (Widget row : m_aRows)
		{
			int idx = RZ_UIW.GetTag(row);
			if (idx >= 0 && idx < m_aJobs.Count())
				LightRow(row, idx);
		}

		RZ_UIW.FocusWidget(w);
		return true;
	}
}
