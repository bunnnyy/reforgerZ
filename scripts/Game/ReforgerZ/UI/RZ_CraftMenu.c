//! The crafting screen at a workbench (docs/UI.md, "The crafting screen"), in the shop's and the
//! car repair screen's style: the recipes as a list, and for the picked one a picture of what it
//! makes (vanilla's item preview of its prefab) with its description, what it takes with what you
//! have and what's missing, how long it takes, and Craft. Opened by the server when a player uses
//! a workbench (RZ_Crafting.Look); every Craft goes back to the server, which checks and takes the
//! inputs as the bench's actions do (RZ_Crafting.DoCraft).
//!
//! The same screen is the cooking screen at a stove (docs/UI.md, "The cooking screen"): opened by
//! "Use the stove" (RZ_Crafting.UseStove) with PAYLOAD_COOK, its rows from UI > Cook row layout, the
//! button named for what the picked row does (Boil, Brew, Cook) and its text about the stove, the
//! stove's status strip (FIRE, FUEL, ON THE HOB; UI/RZ_StatusStrip.c) under the header, and the
//! rows' tags READY, SHORT 1 or NONE.
//!
//! Keys: up/down pick a recipe, Enter crafts, Esc closes. Pad: D-pad or left stick, A crafts, B
//! closes. Mouse: click a recipe, the Craft button, the close button. You can't move while it's
//! open; work under way goes on, and danger closes the screen (the server's screen guard).

//------------------------------------------------------------------------------------------------
class RZ_CraftMenu : RZ_MenuBase
{
	protected static RZ_CraftMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;
	protected static float s_fDataAt;
	protected static bool s_bFailed;			// the menu couldn't open here: the actions come back
	protected static bool s_bStove;				// the data is a stove's (the cooking screen)
	protected static float s_fFallbackUntil;	// the server was at its cap: the actions for a minute

	protected static const int INPUT_ROWS = 3;

	// The status strip's room (RZ_CraftMenu.layout): with it, the body sits lower and the panel is taller
	protected static const float BODY_Y_STRIP = 262;
	protected static const float BODY_Y_PLAIN = 160;
	protected static const float PANEL_W = 1600;
	protected static const float PANEL_H_STRIP = 960;
	protected static const float PANEL_H_PLAIN = 858;

	protected int m_iSerial;
	protected int m_iParsed = -1;
	protected bool m_bDone;						// closed by the server: nothing to send back
	protected int m_iSelected;
	protected int m_iLocal;						// bumps when only this client changed something (a local status)
	protected string m_sLocalStatus;
	protected int m_iWorking = -1;
	protected float m_fLeft;
	protected float m_fTotal;
	protected ref array<string> m_aHead = {};
	protected ref array<ref array<string>> m_aRecipes = {};
	protected ref array<ref array<string>> m_aInputs = {};
	protected ref array<ref array<string>> m_aTiles = {};		// a stove's status strip (S records)
	protected ref array<Widget> m_aRows = {};
	protected Widget m_wSelectPrompt;							// the footer's A: the button's verb, dimmed when it can't go
	protected ref RZ_ItemPreview m_Preview = new RZ_ItemPreview();
	protected ResourceName m_sDescribed;
	protected string m_sDescription;

	//------------------------------------------------------------------------------------------------
	//! The crafting screen (or with `stove` the cooking screen) is used on this machine: its row
	//! layout set (the server checks the same), it hasn't failed to open here, and the server isn't
	//! at its cap of open screens.
	static bool Available(bool stove = false)
	{
		if (s_bFailed || RZ_UIW.Now() < s_fFallbackUntil)
			return false;

		if (stove)
			return !RZ_UISettings.Get().m_sCookRowLayout.IsEmpty();

		return !RZ_UISettings.Get().m_sCraftRowLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! A workbench's Make slots, and a stove's Boil, Brew and Cook slots, hide while the screen is
	//! used: they're its rows. The server marks such a spot by its read label.
	static bool HidesSlotsOn(IEntity owner)
	{
		if (!owner)
			return false;

		RZ_ClueComponent clue = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		if (!clue)
			return false;

		string label = clue.GetReadLabel();
		if (label == RZ_Crafting.BENCH_LABEL)
			return Available(false);

		if (label == RZ_Crafting.STOVE_LABEL)
			return Available(true);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The screen kind the server knows this one by.
	protected static int Kind()
	{
		if (s_bStove)
			return RZ_ScreenGuard.KIND_COOK;

		return RZ_ScreenGuard.KIND_CRAFT;
	}

	//------------------------------------------------------------------------------------------------
	//! Its notices' title.
	protected static string Title()
	{
		if (s_bStove)
			return "Stove";

		return "Workbench";
	}

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A crafting payload, or with `stove` a cooking one (RZ_PlayerUIComponent): opens or redraws
	//! the screen; X|reason closes it; F|why: the server couldn't open one, so the bench's or the
	//! stove's actions show for a minute.
	static void OnPayload(RZ_PlayerUIComponent ui, string payload, bool stove)
	{
		if (payload.StartsWith("X" + RZ_UIText.SEP))
		{
			if (s_Open && s_bStove == stove)
			{
				s_Open.m_bDone = true;
				s_Open.Close();
			}

			return;
		}

		s_bStove = stove;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (payload.StartsWith("F" + RZ_UIText.SEP))
		{
			s_fFallbackUntil = RZ_UIW.Now() + 60;
			if (gm)
				gm.ShowNotice(Title(), "Too much going on at once. The recipes are in the action menu for now.", RZ_ENoticeTag.AUTO);

			return;
		}

		s_sData = payload;
		s_iVersion++;
		s_fDataAt = RZ_UIW.Now();
		if (s_Open || RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_CraftMenu))
			return;

		// No preset or layout here: the actions come back
		s_bFailed = true;
		ui.SendScreenClosed(Kind());
		if (gm)
			gm.ShowNotice(Title(), "The recipes are in the action menu.", RZ_ENoticeTag.AUTO);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		RZ_UIW.OnClick(m_wRoot, "CraftButton", m_Click);
		string verb = "Craft";
		if (s_bStove)
			verb = "Cook";

		m_wSelectPrompt = RZ_StatusStrip.MakePrompt(m_wRoot, "Prompts", "MenuSelect", verb);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && m_wSelectPrompt != null;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		m_Preview.Make(RZ_UIW.Find(m_wRoot, "PreviewSlot"));
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

		// Esc, B, the close button, or another menu on top: the server forgets the screen
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendScreenClosed(Kind());

		m_bDone = true;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! New data, a new selection, a local status, and once a second while work is under way or a
	//! stove's tile counts down.
	override protected int DataVersion()
	{
		int tick = 0;
		if (m_iWorking >= 0 || RZ_StatusStrip.Counting(m_aTiles))
			tick = RZ_Util.RoundToInt(Math.Floor(RZ_UIW.Now())) % 100;

		return s_iVersion * 1000000 + m_iLocal * 10000 + tick * 100 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Max(0, m_iSelected - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Min(Math.Max(0, m_aRecipes.Count() - 1), m_iSelected + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		TryCraft();
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: a recipe row selects it, the Craft button crafts the selected one.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "CraftButton")
			TryCraft();
		else if (name == "RecipeRow")
			m_iSelected = RZ_UIW.GetTag(w);
	}

	//------------------------------------------------------------------------------------------------
	//! The selected recipe's record, or null.
	protected array<string> Selected()
	{
		if (m_iSelected < 0 || m_iSelected >= m_aRecipes.Count())
			return null;

		return m_aRecipes[m_iSelected];
	}

	//------------------------------------------------------------------------------------------------
	//! Craft the selected recipe if you have it all and nothing is under way (the server checks
	//! it all again); else say why here, without asking the server.
	protected void TryCraft()
	{
		array<string> f = Selected();
		if (!f)
			return;

		if (m_iWorking >= 0)
		{
			m_sLocalStatus = "You're already at work. Wait for it to finish.";
			m_iLocal++;
			return;
		}

		if (RZ_UIText.Int(f, 7) != 1)
		{
			m_sLocalStatus = string.Format("%1 takes more than you're carrying. Missing: %2.", RZ_UIText.Str(f, 2), MissingText(RZ_UIText.Int(f, 1)));
			m_iLocal++;
			return;
		}

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCraft(m_iSerial, RZ_UIText.Int(f, 1));
	}

	//------------------------------------------------------------------------------------------------
	//! The inputs of recipe `index` you're short of: "Empty tin, Wire or string".
	protected string MissingText(int index)
	{
		string text = "";
		foreach (array<string> n : m_aInputs)
		{
			if (RZ_UIText.Int(n, 1) != index || RZ_UIText.Int(n, 3) >= RZ_UIText.Int(n, 4))
				continue;

			if (!text.IsEmpty())
				text += ", ";

			text += RZ_UIText.Str(n, 2);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aRecipes.Clear();
		m_aInputs.Clear();
		m_aTiles.Clear();
		m_iWorking = -1;
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "S")
				m_aTiles.Insert(f);
			else if (tag == "B")
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
			else if (tag == "R")
			{
				m_aRecipes.Insert(f);
			}
			else if (tag == "N")
			{
				m_aInputs.Insert(f);
			}
		}

		// The server's word replaces what this client said
		m_sLocalStatus = "";
		m_iSelected = Math.Clamp(m_iSelected, 0, Math.Max(0, m_aRecipes.Count() - 1));
	}

	//------------------------------------------------------------------------------------------------
	//! B|serial|title|look|status|ready|recipes|town|hint|lit; W|recipe|left|total; S|label|value|state|left;
	//! R|index|name|prefab|icon|seconds|heat|ready|verb; N|index|name|have|need
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		RZ_UIW.SetText(m_wRoot, "CraftTitle", RZ_UIText.Str(m_aHead, 2));
		string town = RZ_UIText.Str(m_aHead, 7);
		RZ_UIW.SetText(m_wRoot, "CraftTown", town);
		RZ_UIW.Show(m_wRoot, "CraftTown", !town.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "CraftSub", RZ_UIText.Str(m_aHead, 3));
		DrawStrip();
		string readyText = "You can make %1 of %2";
		if (s_bStove)
			readyText = "You can cook %1 of %2";

		RZ_UIW.SetText(m_wRoot, "CraftReady", string.Format(readyText, RZ_UIText.Int(m_aHead, 5), RZ_UIText.Int(m_aHead, 6)));
		string status = m_sLocalStatus;
		if (status.IsEmpty())
			status = RZ_UIText.Str(m_aHead, 4);

		RZ_UIW.SetText(m_wRoot, "CraftStatus", status);
		RZ_UIW.Show(m_wRoot, "CraftStatus", !status.IsEmpty());
		DrawRecipes();
		DrawDetails();
	}

	//------------------------------------------------------------------------------------------------
	//! A stove's status strip (FIRE, FUEL, ON THE HOB) under the header; at a bench there's none,
	//! so the body moves up and the panel shrinks.
	protected void DrawStrip()
	{
		RZ_StatusStrip.Draw(m_wRoot, m_aTiles, RZ_UIW.Now() - s_fDataAt);
		float bodyY = BODY_Y_PLAIN;
		float panelH = PANEL_H_PLAIN;
		if (!m_aTiles.IsEmpty())
		{
			bodyY = BODY_Y_STRIP;
			panelH = PANEL_H_STRIP;
		}

		RZ_UIW.SetPos(RZ_UIW.Find(m_wRoot, "CraftBody"), 0, bodyY);
		RZ_UIW.SetSize(RZ_UIW.Find(m_wRoot, "CraftPanel"), PANEL_W, panelH); // VERIFY: FrameSlot.SetSize on the centred panel (anchor and alignment 0.5) keeps it centred
	}

	//------------------------------------------------------------------------------------------------
	static Color Amber()
	{
		return RZ_UIColor.Faction(RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	//! Olive: you have it; red: you're short.
	static Color HaveColour(bool have)
	{
		if (have)
			return RZ_UIColor.Faction(RZ_EFaction.MILITARY);

		return RZ_UIW.Rgb(210, 85, 63);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawRecipes()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Recipes");
		RZ_UIW.ClearChildren(list);
		m_aRows.Clear();
		RZ_UIW.Show(m_wRoot, "RecipesEmpty", m_aRecipes.IsEmpty());
		ResourceName rowLayout = RZ_UISettings.Get().m_sCraftRowLayout;
		if (s_bStove)
			rowLayout = RZ_UISettings.Get().m_sCookRowLayout;

		foreach (int i, array<string> f : m_aRecipes)
		{
			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				return;

			bool on = i == m_iSelected;
			bool ready = RZ_UIText.Int(f, 7) == 1;
			int index = RZ_UIText.Int(f, 1);
			string icon = RZ_UIText.Str(f, 4);
			string state = "READY";
			Color stateColour = HaveColour(true);
			Color fg = RZ_UIColor.Bone();
			if (!ready)
			{
				state = string.Format("%1 MISSING", MissingCount(index));
				stateColour = RZ_UIColor.Dim();
				fg = RZ_UIW.Rgb(110, 106, 96);

				// At a stove: SHORT 1 in red, or NONE dim when you carry none of it
				if (s_bStove && HasNone(index))
				{
					state = "NONE";
				}
				else if (s_bStove)
				{
					state = string.Format("SHORT %1", MissingCount(index));
					stateColour = HaveColour(false);
				}
			}
			else if (on)
			{
				fg = RZ_UIW.Rgb(244, 240, 230);
			}

			if (index == m_iWorking)
			{
				state = "WORKING";
				stateColour = Amber();
			}

			RZ_UIW.SetText(row, "RecipeName", RZ_UIText.Str(f, 2));
			RZ_UIW.Tint(row, "RecipeName", fg);
			RZ_UIW.SetText(row, "RecipeState", state);
			RZ_UIW.Tint(row, "RecipeState", stateColour);
			RZ_UIW.SetText(row, "RecipeTime", string.Format("%1 s", RZ_UIText.Int(f, 5)));
			RZ_UIW.Show(row, "RecipeIcon", !icon.IsEmpty() && !RZ_UISettings.Get().m_sIconImageset.IsEmpty());
			RZ_UIW.Icon(row, "RecipeIcon", icon);
			RZ_UIW.Tint(row, "RecipeIcon", fg);
			RZ_UIW.Show(row, "RecipeBg", on);
			RZ_UIW.Show(row, "RecipeBar", on);
			RZ_UIW.Tint(row, "RecipeBar", Amber());
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRows.Insert(row);
		}

		// Pad focus on the selected recipe, kept in view
		if (m_iSelected >= 0 && m_iSelected < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[m_iSelected]);

		ScrollLayoutWidget sc = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "RecipeScroll"));
		if (sc && m_aRows.Count() > 1)
			sc.SetSliderPos(0, m_iSelected / (m_aRows.Count() - 1.0));
	}

	//------------------------------------------------------------------------------------------------
	//! How many inputs of recipe `index` you're short of (kinds, not items).
	protected int MissingCount(int index)
	{
		int n = 0;
		foreach (array<string> f : m_aInputs)
		{
			if (RZ_UIText.Int(f, 1) == index && RZ_UIText.Int(f, 3) < RZ_UIText.Int(f, 4))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! You carry nothing at all that recipe `index` takes.
	protected bool HasNone(int index)
	{
		foreach (array<string> f : m_aInputs)
		{
			if (RZ_UIText.Int(f, 1) == index && RZ_UIText.Int(f, 3) > 0)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDetails()
	{
		array<string> f = Selected();
		RZ_UIW.Show(m_wRoot, "Details", f != null);
		if (!f)
		{
			m_Preview.Show("");
			RZ_StatusStrip.SetPrompt(m_wSelectPrompt, "", false);
			return;
		}

		int index = RZ_UIText.Int(f, 1);
		bool ready = RZ_UIText.Int(f, 7) == 1;
		ResourceName prefab = RZ_UIText.Str(f, 3);
		if (prefab != m_sDescribed)
		{
			m_sDescribed = prefab;
			m_sDescription = RZ_ItemPreview.Describe(prefab);
		}

		RZ_UIW.SetText(m_wRoot, "DetailName", RZ_UIText.Str(f, 2));
		RZ_UIW.SetText(m_wRoot, "DetailDesc", m_sDescription);
		m_Preview.Show(prefab);
		DrawInputs(index);

		// How long, how loud, and the work under way
		int seconds = RZ_UIText.Int(f, 5);
		bool working = index == m_iWorking && m_fTotal > 0;
		float left = Math.Max(0, m_fLeft - (RZ_UIW.Now() - s_fDataAt));
		string time = string.Format("About %1 seconds at the bench", seconds);
		if (s_bStove)
			time = string.Format("About %1 seconds on the stove", seconds);

		if (working)
			time = string.Format("Working... %1 s", RZ_Util.RoundToInt(Math.Ceil(left)));

		RZ_UIW.SetText(m_wRoot, "DetailTime", time);
		RZ_UIW.SetText(m_wRoot, "DetailNoise", NoiseText(RZ_UIText.Int(f, 6), s_bStove));

		// At a stove: whether its fire is going (it doesn't stop you: the work lights it)
		RZ_UIW.Show(m_wRoot, "DetailLit", s_bStove);
		if (RZ_UIText.Int(m_aHead, 9) == 1)
		{
			RZ_UIW.SetText(m_wRoot, "DetailLit", "A lit stove: YES");
			RZ_UIW.Tint(m_wRoot, "DetailLit", HaveColour(true));
		}
		else
		{
			RZ_UIW.SetText(m_wRoot, "DetailLit", "A lit stove: NO, you'll light it");
			RZ_UIW.Tint(m_wRoot, "DetailLit", RZ_UIColor.Dim());
		}
		RZ_UIW.Show(m_wRoot, "CraftProgress", working);
		if (working)
			RZ_UIW.Progress(m_wRoot, "CraftProgress", 1 - left / m_fTotal);

		// Craft: amber when it can go now, dark while you're short or at work
		bool canGo = ready && m_iWorking < 0;
		Color bg = RZ_UIW.Rgb(27, 31, 34);
		Color text = RZ_UIW.Rgb(110, 106, 96);
		if (canGo)
		{
			bg = Amber();
			text = RZ_UIW.Rgb(21, 23, 26);
		}

		RZ_UIW.Tint(m_wRoot, "CraftBg", bg);
		RZ_UIW.Tint(m_wRoot, "CraftText", text);

		// The button says what it does: CRAFT at the bench, BOIL, BREW or COOK at a stove
		string verb = RZ_UIText.Str(f, 8);
		if (verb.IsEmpty())
			verb = "Craft";

		// The footer's A says the same ("A Boil"), dimmed while it can't go
		RZ_StatusStrip.SetPrompt(m_wSelectPrompt, verb, canGo);
		verb.ToUpper();
		RZ_UIW.SetText(m_wRoot, "CraftText", verb);
	}

	//------------------------------------------------------------------------------------------------
	//! "IT TAKES": one row per input, its name, "1 of 2" and HAVE or MISSING; at a stove its name
	//! and "1 / 1" on the right, olive when you have it, red when you don't.
	protected void DrawInputs(int index)
	{
		int row = 0;
		foreach (array<string> n : m_aInputs)
		{
			if (RZ_UIText.Int(n, 1) != index || row >= INPUT_ROWS)
				continue;

			string idx = row.ToString();
			int have = RZ_UIText.Int(n, 3);
			int need = RZ_UIText.Int(n, 4);
			bool enough = have >= need;
			string mark = "MISSING";
			if (enough)
				mark = "HAVE";

			if (s_bStove)
				mark = string.Format("%1 / %2", have, need);

			RZ_UIW.Show(m_wRoot, "Input" + idx + "Count", !s_bStove);
			RZ_UIW.Show(m_wRoot, "Input" + idx, true);
			RZ_UIW.SetText(m_wRoot, "Input" + idx + "Name", RZ_UIText.Str(n, 2));
			RZ_UIW.SetText(m_wRoot, "Input" + idx + "Count", string.Format("%1 of %2", have, need));
			RZ_UIW.SetText(m_wRoot, "Input" + idx + "State", mark);
			RZ_UIW.Tint(m_wRoot, "Input" + idx + "State", HaveColour(enough));
			RZ_UIW.Tint(m_wRoot, "Input" + idx + "Bar", HaveColour(enough));
			row++;
		}

		for (int r = row; r < INPUT_ROWS; r++)
		{
			RZ_UIW.Show(m_wRoot, "Input" + r.ToString(), false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What the work's noise means (the heat it adds where you stand); at a stove, the fire's smoke.
	static string NoiseText(int heat, bool stove)
	{
		if (stove)
		{
			if (heat < 3)
				return "A little smoke from the chimney. Someone may see it.";

			return "Smoke, and the smell carries. Anything close will come and look.";
		}

		if (heat <= 0)
			return "Quiet work.";

		if (heat < 3)
			return "Some hammering. It carries a little.";

		return "Loud work. Anything close will hear it.";
	}
}
