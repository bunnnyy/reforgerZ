//! The site builder's build bar (docs/SITES.md, "The site builder"; UI/LAYOUTS.md, RZ_SiteHud): while
//! you build a site, a compact panel top right shows it, how many of its spots are done, and three
//! pages of buttons (People, Places, Patrols) to put spots down with Game Master's cursor: pick a
//! spot type, then click the ground (Game Master's own left click, EditorSetSelection) and it's put
//! there. Patrols: up to three loops, each a list of points clicked in order (closed back to the
//! first). Undo takes up the last spot, Save and Stop are the site builder's. The server sends it
//! every two seconds while you build (World/HandBuilt/RZ_SiteBuilderScreen.c); "" hides it.
//! Hidden while the site builder's screen is open.
class RZ_SiteHud
{
	protected static const ResourceName LAYOUT = "{4382F77E3ACA1D25}UI/layouts/ReforgerZ/RZ_SiteHud.layout";
	protected static const int BUTTONS = 8;		// Btn0-Btn7
	protected static const int PAGES = 3;
	protected static const int LOOPS = 3;

	protected static Widget s_wRoot;
	protected static bool s_bHidden;			// the site builder's screen is open: it shows the same
	protected static ref RZ_SiteBarClick s_Click;
	protected static int s_iPage;				// 0 people, 1 places, 2 patrols
	protected static string s_sArmed;			// the spot type the next ground click puts down ("" none)
	protected static int s_iLoop = 1;			// the patrol loop points go to
	protected static bool s_bListening;
	protected static string s_sProgress;
	protected static bool s_bPreviewing;		// the preview follows the cursor while a type is picked
	protected static Widget s_wLit;				// the bar's button under the mouse, lit, its line in the hint

	protected static ref array<ref array<string>> s_aTypes = {};	// T|type|have|want|required
	protected static ref array<ref array<string>> s_aLoops = {};	// L|loop|points|length

	//------------------------------------------------------------------------------------------------
	//! A page's spot types, in its button order.
	protected static void PageTypes(int page, notnull array<string> types, notnull array<string> names)
	{
		array<string> peopleTypes = {"gate", "watch", "sit", "work", "sleep", "lean", "qm", "leader"};
		array<string> peopleNames = {"Gate", "Watch", "Seat", "Work", "Sleep", "Lean", "Quartermaster", "Leader"};
		array<string> placeTypes = {"cook", "meal", "water", "wash", "medic", "stores", "chop", "latrine"};
		array<string> placeNames = {"Cook", "Canteen", "Water", "Wash", "Medic", "Stores", "Chop", "Latrine"};
		if (page == 0)
		{
			types.Copy(peopleTypes);
			names.Copy(peopleNames);
		}
		else if (page == 1)
		{
			types.Copy(placeTypes);
			names.Copy(placeNames);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Hidden while the site builder's screen is open (it would sit over it).
	static void SetHidden(bool hidden)
	{
		s_bHidden = hidden;
		if (s_wRoot)
			s_wRoot.SetVisible(!hidden);
	}

	//------------------------------------------------------------------------------------------------
	static void OnPayload(string payload)
	{
		if (payload.IsEmpty())
		{
			Close();
			return;
		}

		// In Game Master the bar goes inside its screen (a loose widget under it gets no clicks): made
		// again whenever the screen it should be in changes
		Widget parent = Parent();
		if (s_wRoot && s_wRoot.GetParent() != parent)
			Drop();

		if (!s_wRoot && !Make(parent))
			return;

		s_wRoot.SetVisible(!s_bHidden);
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		array<string> head = null;
		s_aTypes.Clear();
		s_aLoops.Clear();
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "B")
				head = f;
			else if (tag == "T")
				s_aTypes.Insert(f);
			else if (tag == "L")
				s_aLoops.Insert(f);
		}

		if (!head)
			return;

		RZ_UIW.SetText(s_wRoot, "BarName", RZ_UIText.Clean(RZ_UIText.Str(head, 1), 30));
		string progress = string.Format("%1/%2", RZ_UIText.Int(head, 6), RZ_UIText.Int(head, 7));
		if (RZ_UIText.Int(head, 4) == 1)
			progress += " · unsaved";

		s_sProgress = progress;
		Draw();
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Where the bar goes: Game Master's screen while it's open (its top menu, not the site builder's
	//! own), else the workspace.
	protected static Widget Parent()
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		MenuManager menus = GetGame().GetMenuManager();
		if (!menus)
			return ws;

		MenuBase top = menus.GetTopMenu();
		if (!top || RZ_SiteBuilderMenu.Cast(top))
			return ws;

		SCR_EditorManagerEntity editor = SCR_EditorManagerEntity.GetInstance();
		if (!editor || !editor.IsOpened())
			return ws;

		Widget root = top.GetRootWidget();
		if (!root)
			return ws;

		return root;
	}

	//------------------------------------------------------------------------------------------------
	//! The bar taken down (kept state: page, picked type, loop).
	protected static void Drop()
	{
		if (s_wRoot)
			s_wRoot.RemoveFromHierarchy();

		s_wRoot = null;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Make(Widget parent)
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!ws)
			return false;

		if (parent && parent != ws)
			s_wRoot = ws.CreateWidgets(LAYOUT, parent); // VERIFY: widgets made inside Game Master's menu root take its mouse clicks
		else
			s_wRoot = ws.CreateWidgets(LAYOUT);

		if (!s_wRoot)
			return false;

		s_wRoot.SetZOrder(1000);
		GetGame().GetCallqueue().Remove(HoverTick);
		GetGame().GetCallqueue().CallLater(HoverTick, 0, true);

		s_Click = new RZ_SiteBarClick();
		array<string> names = {"PagePrev", "PageNext", "BarUndo", "BarSave", "BarStop"};
		foreach (string n : names)
		{
			RZ_UIW.OnClick(s_wRoot, n, s_Click);
		}

		for (int i = 0; i < BUTTONS; i++)
		{
			RZ_UIW.OnClick(s_wRoot, "Btn" + i.ToString(), s_Click);
			RZ_UIW.SetTag(RZ_UIW.Find(s_wRoot, "Btn" + i.ToString()), i);
		}

		if (!s_bListening)
		{
			s_bListening = true;
			GetGame().GetInputManager().AddActionListener("EditorSetSelection", EActionTrigger.DOWN, OnGroundClick);
			GetGame().GetInputManager().AddActionListener("EditorActionsMenu", EActionTrigger.DOWN, OnCancel);
			GetGame().GetInputManager().AddActionListener("EditorCancelPlacing", EActionTrigger.DOWN, OnCancel);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Close()
	{
		Preview(false);
		GetGame().GetCallqueue().Remove(HoverTick);
		s_wLit = null;
		if (s_wRoot)
			s_wRoot.RemoveFromHierarchy();

		s_wRoot = null;
		s_sArmed = "";
		if (s_bListening)
		{
			s_bListening = false;
			GetGame().GetInputManager().RemoveActionListener("EditorSetSelection", EActionTrigger.DOWN, OnGroundClick);
			GetGame().GetInputManager().RemoveActionListener("EditorActionsMenu", EActionTrigger.DOWN, OnCancel);
			GetGame().GetInputManager().RemoveActionListener("EditorCancelPlacing", EActionTrigger.DOWN, OnCancel);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Right click (Game Master's own, its action menu on a pad) with a spot type picked: put it
	//! away, as Game Master cancels placing a thing.
	protected static void OnCancel(float value = 0, EActionTrigger reason = 0)
	{
		if (!s_wRoot || s_bHidden || s_sArmed.IsEmpty())
			return;

		s_sArmed = "";
		Draw();
	}

	//------------------------------------------------------------------------------------------------
	protected static array<string> TypeRecord(string type)
	{
		foreach (array<string> t : s_aTypes)
		{
			if (RZ_UIText.Str(t, 1) == type)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Draw()
	{
		if (!s_wRoot)
			return;

		array<string> titles = {"People", "Places", "Patrols"};
		RZ_UIW.SetText(s_wRoot, "PageTitle", string.Format("%1 · %2/%3", titles[s_iPage], s_iPage + 1, PAGES));
		RZ_UIW.SetText(s_wRoot, "BarProgress", s_sProgress);
		Color idle = RZ_SiteBuilderMenu.Lin(36, 40, 44);
		Color picked = RZ_SiteBuilderMenu.Lin(232, 178, 58);
		Color ink = RZ_SiteBuilderMenu.Lin(236, 233, 225);
		Color dark = RZ_SiteBuilderMenu.Lin(33, 29, 20);
		Color green = RZ_SiteBuilderMenu.Lin(151, 196, 89);
		Color red = RZ_SiteBuilderMenu.Lin(240, 149, 149);
		Color grey = RZ_SiteBuilderMenu.Lin(154, 151, 143);
		for (int i = 0; i < BUTTONS; i++)
		{
			string idx = i.ToString();
			string name = "";
			string state = "";
			Color stateCol = grey;
			bool on = false;
			if (s_iPage < 2)
			{
				array<string> types = {};
				array<string> names = {};
				PageTypes(s_iPage, types, names);
				string type = types[i];
				name = names[i];
				on = s_sArmed == type;
				array<string> rec = TypeRecord(type);
				int have = 0;
				int want = 0;
				bool required = false;
				if (rec)
				{
					have = RZ_UIText.Int(rec, 2);
					want = RZ_UIText.Int(rec, 3);
					required = RZ_UIText.Int(rec, 4) == 1;
				}

				if (want <= 0 || !required)
				{
					state = "+";
					if (have > 0)
						state = have.ToString();
				}
				else if (have >= want)
				{
					state = "done";
					stateCol = green;
				}
				else
				{
					state = string.Format("%1/%2", have, want);
					stateCol = red;
				}
			}
			else
			{
				PatrolButton(i, name, state, on);
			}

			RZ_UIW.Show(s_wRoot, "Btn" + idx, !name.IsEmpty());
			RZ_UIW.SetText(s_wRoot, "BtnName" + idx, name);
			RZ_UIW.SetText(s_wRoot, "BtnState" + idx, state);
			RZ_UIW.Tint(s_wRoot, "BtnBg" + idx, idle);
			RZ_UIW.Tint(s_wRoot, "BtnName" + idx, ink);
			RZ_UIW.Tint(s_wRoot, "BtnState" + idx, stateCol);
			if (on)
			{
				RZ_UIW.Tint(s_wRoot, "BtnBg" + idx, picked);
				RZ_UIW.Tint(s_wRoot, "BtnName" + idx, dark);
				RZ_UIW.Tint(s_wRoot, "BtnState" + idx, dark);
			}
		}

		string hint = "";
		if (s_sArmed == "route")
			hint = string.Format("Click the ground: patrol %1's next point", s_iLoop);
		else if (!s_sArmed.IsEmpty())
			hint = "Click the ground to put it there";

		array<string> fixed = {"PagePrev", "PageNext", "BarUndo", "BarSave", "BarStop"};
		foreach (string f : fixed)
		{
			RZ_UIW.Tint(s_wRoot, f + "Bg", RZ_SiteBuilderMenu.Lin(36, 40, 44));
		}

		RZ_UIW.Tint(s_wRoot, "BarHint", picked);
		// The button under the mouse: lighter, and what it does in the hint line
		if (s_wLit)
		{
			string litName = s_wLit.GetName();
			string litBg = litName + "Bg";
			if (litName.StartsWith("Btn"))
				litBg = "BtnBg" + litName.Substring(3, litName.Length() - 3);

			Color lit = RZ_SiteBuilderMenu.Lin(70, 78, 86);
			if (litName.StartsWith("Btn") && s_iPage < 2 && LitArmed(RZ_UIW.GetTag(s_wLit)))
				lit = RZ_SiteBuilderMenu.Lin(250, 208, 120);

			RZ_UIW.Tint(s_wRoot, litBg, lit);
			string help = Describe(litName, RZ_UIW.GetTag(s_wLit));
			if (!help.IsEmpty())
			{
				hint = help;
				RZ_UIW.Tint(s_wRoot, "BarHint", ink);
			}
		}

		RZ_UIW.SetText(s_wRoot, "BarHint", hint);
		Preview(!s_sArmed.IsEmpty());
	}

	//------------------------------------------------------------------------------------------------
	//! Button `i` of a spots page is the type picked now.
	protected static bool LitArmed(int i)
	{
		array<string> types = {};
		array<string> names = {};
		PageTypes(s_iPage, types, names);
		return i >= 0 && i < types.Count() && types[i] == s_sArmed;
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame while the bar shows: the button under the mouse (the enter and leave events don't
	//! come through a button's own picture and text, so it's polled).
	protected static void HoverTick()
	{
		if (!s_wRoot)
			return;

		Widget lit = null;
		if (!s_bHidden && RZ_UIW.ByMouse())
		{
			Widget p = WidgetManager.GetWidgetUnderCursor();
			while (p && p != s_wRoot && !lit)
			{
				string n = p.GetName();
				if ((n.StartsWith("Btn") && n.Length() == 4) || n == "PagePrev" || n == "PageNext" || n == "BarUndo" || n == "BarSave" || n == "BarStop")
					lit = p;

				p = p.GetParent();
			}

			if (lit && !lit.IsVisible())
				lit = null;
		}

		if (lit == s_wLit)
			return;

		s_wLit = lit;
		Draw();
	}

	//------------------------------------------------------------------------------------------------
	//! One sentence on what a bar button does.
	protected static string Describe(string name, int tag)
	{
		switch (name)
		{
			case "PagePrev": return "The previous page of spots.";
			case "PageNext": return "The next page of spots.";
			case "BarUndo": return "Takes up the last spot you put down.";
			case "BarSave": return "Saves the site as it is now.";
			case "BarStop": return "Stops building; you're asked about anything unsaved.";
		}

		if (!name.StartsWith("Btn"))
			return "";

		if (s_iPage < 2)
		{
			array<string> types = {};
			array<string> names = {};
			PageTypes(s_iPage, types, names);
			if (tag < 0 || tag >= types.Count())
				return "";

			return RZ_HandSpots.Help(types[tag]);
		}

		if (tag < LOOPS)
		{
			string what = "the guards' patrol";
			if (tag == 1)
				what = "the daytime stroll";
			else if (tag == 2)
				what = "the walk taken only under attack";

			return string.Format("Picks patrol %1, %2: new points go to it.", tag + 1, what);
		}

		if (tag == 4)
			return "Click the ground to add the next point to the picked patrol.";

		if (tag == 5)
			return "Deletes every point of the picked patrol.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! While a spot type is picked, its cone's label follows Game Master's cursor on the ground, so
	//! you see where it'll go before you click.
	protected static void Preview(bool on)
	{
		if (on == s_bPreviewing)
			return;

		s_bPreviewing = on;
		if (on)
		{
			GetGame().GetCallqueue().CallLater(PreviewTick, 0, true);
			return;
		}

		GetGame().GetCallqueue().Remove(PreviewTick);
		RZ_SiteBuilderMarks.SetPreview(false, vector.Zero, "", "");
	}

	//------------------------------------------------------------------------------------------------
	protected static void PreviewTick()
	{
		SCR_MenuLayoutEditorComponent layout = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent));
		vector pos;
		if (s_sArmed.IsEmpty() || !s_wRoot || !layout || !layout.GetCursorWorldPos(pos))
		{
			RZ_SiteBuilderMarks.SetPreview(false, vector.Zero, "", "");
			return;
		}

		string name = RZ_HandSpots.Label(s_sArmed) + ": click to put it here";
		if (s_sArmed == "route")
			name = string.Format("Patrol %1: click to add a point", s_iLoop);

		RZ_SiteBuilderMarks.SetPreview(true, pos, s_sArmed, name);
	}

	//------------------------------------------------------------------------------------------------
	//! The patrols page: a button per loop (picked = points go to it), then add a point, undo it
	//! (Undo below does the same for any spot), delete the picked loop.
	protected static void PatrolButton(int i, out string name, out string state, out bool on)
	{
		name = "";
		state = "";
		on = false;
		if (i < LOOPS)
		{
			int loop = i + 1;
			int points = 0;
			int length = 0;
			foreach (array<string> l : s_aLoops)
			{
				if (RZ_UIText.Int(l, 1) == loop)
				{
					points = RZ_UIText.Int(l, 2);
					length = RZ_UIText.Int(l, 3);
				}
			}

			name = string.Format("Patrol %1", loop);
			state = "new";
			if (points > 0)
				state = string.Format("%1 pts", points);

			if (length > 0)
				name += string.Format(" · %1 m", length);

			on = s_iLoop == loop;
			return;
		}

		if (i == 4)
		{
			name = "Add point";
			on = s_sArmed == "route";
		}
		else if (i == 5)
		{
			name = "Delete patrol";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A click on the bar (RZ_SiteBarClick).
	static void OnClicked(Widget w)
	{
		string name = w.GetName();
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (name == "PagePrev")
		{
			s_iPage = (s_iPage + PAGES - 1) % PAGES;
			s_sArmed = "";
		}
		else if (name == "PageNext")
		{
			s_iPage = (s_iPage + 1) % PAGES;
			s_sArmed = "";
		}
		else if (name == "BarUndo" && ui)
		{
			ui.SendSiteAct("bar_spot_undo", "");
		}
		else if (name == "BarSave" && ui)
		{
			ui.SendSiteAct("bar_save", "");
		}
		else if (name == "BarStop" && ui)
		{
			s_sArmed = "";
			ui.SendSiteAct("bar_stop", "");
		}
		else if (name.StartsWith("Btn"))
		{
			PressButton(RZ_UIW.GetTag(w));
		}

		Draw();
	}

	//------------------------------------------------------------------------------------------------
	protected static void PressButton(int i)
	{
		if (s_iPage < 2)
		{
			array<string> types = {};
			array<string> names = {};
			PageTypes(s_iPage, types, names);
			if (i < 0 || i >= types.Count())
				return;

			// Picked again: let go
			if (s_sArmed == types[i])
				s_sArmed = "";
			else
				s_sArmed = types[i];

			return;
		}

		if (i < LOOPS)
		{
			s_iLoop = i + 1;
			s_sArmed = "route";
			return;
		}

		if (i == 4)
		{
			if (s_sArmed == "route")
				s_sArmed = "";
			else
				s_sArmed = "route";

			return;
		}

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (i == 5 && ui)
			ui.SendSiteAct("bar_loop_delete", s_iLoop.ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Game Master's left click with a spot type picked: it's put where the cursor points on the
	//! ground (not when the click was on the bar itself).
	protected static void OnGroundClick(float value = 0, EActionTrigger reason = 0)
	{
		if (!s_wRoot || s_bHidden || s_sArmed.IsEmpty())
			return;

		Widget under = WidgetManager.GetWidgetUnderCursor(); // VERIFY: the widget under the mouse cursor in Game Master (null over the world)
		if (under && RZ_UIW.IsInside(under, s_wRoot))
			return;

		SCR_MenuLayoutEditorComponent layout = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent));
		vector pos;
		if (!layout || !layout.GetCursorWorldPos(pos)) // VERIFY: Game Master's cursor point on the ground, as its own context actions read it
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		ui.SendSiteAct("bar_spot_at", string.Format("%1|%2|%3|%4|%5", s_sArmed, pos[0], pos[1], pos[2], s_iLoop));
		// One of a kind at a time; patrol points go on
		if (s_sArmed != "route")
			s_sArmed = "";

		Draw();
	}
}

//------------------------------------------------------------------------------------------------
//! Passes a click on the build bar to it.
class RZ_SiteBarClick : ScriptedWidgetEventHandler
{
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!w)
			return false;

		RZ_SiteHud.OnClicked(w);
		return true;
	}
}
