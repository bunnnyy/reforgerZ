//! The ReforgerZ menus (docs/UI.md, "Screens"): Standing, Journal, Radio and the Map key, one
//! tab each (Q/E switch tabs, Esc closes), and two on their own: the safehouse notice board and
//! the hall of fame. Layouts and widget names are in UI/LAYOUTS.md; the
//! presets are registered in a chimeraMenus.conf override (WORKBENCH_SETUP.md).
//!
//! Each menu asks RZ_PlayerUIComponent for its data when it opens and redraws when a new copy
//! arrives. They only read; nothing here changes the game.
//!
//! Mouse: the tabs (Tab0-Tab3), the close button (CloseButton), journal rows (MissionRow) and the
//! radio filter chip (FilterButton) are Buttons in the layouts; RZ_MenuClick hears their clicks.
//! The lists scroll with the wheel inside a ScrollLayout. Keys work as before.

modded enum ChimeraMenuPreset // each entry needs a matching chimeraMenus.conf entry
{
	RZ_StandingMenu,
	RZ_JournalMenu,
	RZ_RadioLogMenu,
	RZ_MapKeyMenu,
	RZ_NoticeBoardMenu,
	RZ_HallOfFameMenu
}

//------------------------------------------------------------------------------------------------
//! Passes a clicked widget to its menu.
class RZ_MenuClick : ScriptedWidgetEventHandler
{
	protected RZ_MenuBase m_Menu;

	//------------------------------------------------------------------------------------------------
	void RZ_MenuClick(RZ_MenuBase menu)
	{
		m_Menu = menu;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_Menu || !w)
			return false;

		m_Menu.OnWidgetClick(w);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_MenuBase : ChimeraMenuBase
{
	protected static const float POLL = 0.25;

	protected Widget m_wRoot;
	protected int m_iSeen = -1;
	protected float m_fPoll;
	protected ref RZ_MenuClick m_Click;

	//------------------------------------------------------------------------------------------------
	//! 0 Standing, 1 Journal, 2 Radio, 3 Map key
	protected int TabIndex()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Changes when there is something new to draw.
	protected int DataVersion()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOpened()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void Redraw()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		GetGame().GetInputManager().AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnTabLeft);
		GetGame().GetInputManager().AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnTabRight);
		m_Click = new RZ_MenuClick(this);
		for (int t = 0; t < 4; t++)
		{
			RZ_UIW.OnClick(m_wRoot, "Tab" + t.ToString(), m_Click);
		}

		RZ_UIW.OnClick(m_wRoot, "CloseButton", m_Click);
		OnOpened();
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse (RZ_MenuClick): the tabs and the close button here, the rest in OnClickOther.
	void OnWidgetClick(Widget w)
	{
		string name = w.GetName();
		if (name == "CloseButton")
		{
			Close();
			return;
		}

		for (int t = 0; t < 4; t++)
		{
			if (name == "Tab" + t.ToString())
			{
				if (t != TabIndex())
					SwitchTo(t);

				return;
			}
		}

		OnClickOther(w, name);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickOther(Widget w, string name)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		GetGame().GetInputManager().RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnTabLeft);
		GetGame().GetInputManager().RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnTabRight);
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		m_fPoll -= tDelta;
		if (m_fPoll > 0)
			return;

		m_fPoll = POLL;
		int v = DataVersion();
		if (v == m_iSeen)
			return;

		m_iSeen = v;
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack(float value = 0, EActionTrigger reason = 0)
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabLeft(float value = 0, EActionTrigger reason = 0)
	{
		SwitchTo(TabIndex() + 3);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabRight(float value = 0, EActionTrigger reason = 0)
	{
		SwitchTo(TabIndex() + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SwitchTo(int tab)
	{
		int idx = tab % 4;
		Close();
		RZ_UIW.OpenMenu(Preset(idx));
	}

	//------------------------------------------------------------------------------------------------
	static int Preset(int tab)
	{
		switch (tab)
		{
			case 1: return ChimeraMenuPreset.RZ_JournalMenu;
			case 2: return ChimeraMenuPreset.RZ_RadioLogMenu;
			case 3: return ChimeraMenuPreset.RZ_MapKeyMenu;
		}

		return ChimeraMenuPreset.RZ_StandingMenu;
	}

	//------------------------------------------------------------------------------------------------
	//! A payload's records, each split into fields.
	protected static void Records(string payload, notnull array<ref array<string>> records)
	{
		records.Clear();
		array<string> lines = {};
		RZ_UIText.Split(payload, "\n", lines);
		foreach (string ln : lines)
		{
			if (ln.IsEmpty())
				continue;

			array<string> f = {};
			RZ_UIText.Split(ln, RZ_UIText.SEP, f);
			records.Insert(f);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Fame, Infamy, Favor and tier per faction; the leaders you've met; what each faction needs;
//! truces and feuds (docs/REPUTATION.md).
class RZ_StandingMenu : RZ_MenuBase
{
	//------------------------------------------------------------------------------------------------
	override protected int TabIndex() { return 0; }

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return 0;

		return ui.GetStandingVersion();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_STANDING);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		bool have = false;
		if (ui)
			have = !ui.GetStanding().IsEmpty();

		RZ_UIW.Show(m_wRoot, "Waiting", !have);
		if (!have)
			return;

		array<ref array<string>> recs = {};
		Records(ui.GetStanding(), recs);
		array<string> leaders = {"", "", "", ""};
		string relations = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
			{
				string who = string.Format("%1 - %2 - LIFE %3", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
				who.ToUpper();
				RZ_UIW.SetText(m_wRoot, "Header", who);
			}
			else if (tag == "F")
			{
				DrawFaction(f);
			}
			else if (tag == "L")
			{
				int lf = RZ_UIText.Int(f, 1);
				if (lf >= 0 && lf < 4)
					leaders[lf] = leaders[lf] + string.Format("%1\n%2\n%3\n\n", RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3), RZ_UIText.Str(f, 4));
			}
			else if (tag == "R")
			{
				string kind = "FEUD";
				string effect = "rival Infamy x1.5";
				if (RZ_UIText.Int(f, 1) == RZ_FactionLife.REL_TRUCE)
				{
					kind = "TRUCE";
					effect = "rival Infamy halved";
				}

				relations += string.Format("%1   %2 and %3   %4 min left, %5\n", kind, RZ_Factions.GetName(RZ_UIText.Int(f, 2)),
					RZ_Factions.GetNameInText(RZ_UIText.Int(f, 3)), RZ_UIText.Str(f, 4), effect);
			}
		}

		for (int i = 0; i < 4; i++)
		{
			string text = leaders[i];
			if (text.IsEmpty())
				text = "Nobody's given you a name yet.";

			RZ_UIW.SetText(m_wRoot, "Leaders" + i.ToString(), text);
		}

		if (relations.IsEmpty())
			relations = "No truces, no feuds. For now.";

		RZ_UIW.SetText(m_wRoot, "Relations", relations);
	}

	//------------------------------------------------------------------------------------------------
	//! F|faction|fame|infamy|favor|tier|need|need line
	protected void DrawFaction(array<string> f)
	{
		int fac = RZ_UIText.Int(f, 1);
		if (fac < 0 || fac > 3)
			return;

		string n = fac.ToString();
		int fame = RZ_UIText.Int(f, 2);
		int infamy = RZ_UIText.Int(f, 3);
		RZ_ETier tier = RZ_UIText.Int(f, 5);
		Color accent = RZ_UIColor.Faction(fac);
		Color tc = RZ_UIColor.Tier(tier);
		RZ_UIW.SetText(m_wRoot, "FactionName" + n, RZ_Factions.GetName(fac));
		RZ_UIW.Tint(m_wRoot, "Accent" + n, accent);
		RZ_UIW.Tint(m_wRoot, "Emblem" + n, accent);
		RZ_UIW.Icon(m_wRoot, "Emblem" + n, RZ_UIText.FactionIcon(fac));
		RZ_UIW.Icon(m_wRoot, "TierIcon" + n, RZ_UIText.TierIcon(tier));
		RZ_UIW.Tint(m_wRoot, "TierIcon" + n, tc);
		string tierName = RZ_Reputation.GetTierName(tier);
		tierName.ToUpper();
		RZ_UIW.SetText(m_wRoot, "TierName" + n, tierName);
		RZ_UIW.Tint(m_wRoot, "TierName" + n, tc);
		RZ_UIW.SetText(m_wRoot, "TierNote" + n, RZ_UIText.TierNote(tier));
		RZ_UIW.SetText(m_wRoot, "FameValue" + n, fame.ToString());
		RZ_UIW.Progress(m_wRoot, "FameBar" + n, fame / 100.0);
		RZ_UIW.Tint(m_wRoot, "FameBar" + n, accent);
		RZ_UIW.SetText(m_wRoot, "InfamyValue" + n, infamy.ToString());
		RZ_UIW.Progress(m_wRoot, "InfamyBar" + n, infamy / 100.0);
		RZ_UIW.SetText(m_wRoot, "FavorValue" + n, RZ_UIText.Str(f, 4));
		RZ_UIW.Tint(m_wRoot, "FavorValue" + n, accent);
		string needLine = RZ_UIText.Str(f, 7);
		RZ_UIW.Show(m_wRoot, "NeedBlock" + n, !needLine.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "NeedTitle" + n, RZ_UIText.NeedTitle(RZ_UIText.Int(f, 6)));
		RZ_UIW.SetText(m_wRoot, "Need" + n, needLine);
		RZ_UIW.Show(m_wRoot, "Warn" + n, RZ_Reputation.IsHunted(tier));
	}
}

//------------------------------------------------------------------------------------------------
//! Active contracts and events with type, giver, rough direction, requirements and rewards, the
//! puzzle clues you've read (docs/MISSIONS.md), and the lab's line (research and its backlog).
class RZ_JournalMenu : RZ_MenuBase
{
	protected int m_iSelected;
	protected ref array<ref array<string>> m_aMissions = {};
	protected ref array<ref array<string>> m_aClues = {};

	//------------------------------------------------------------------------------------------------
	override protected int TabIndex() { return 1; }

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return 0;

		return ui.GetJournalVersion() * 100 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		GetGame().GetInputManager().AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		GetGame().GetInputManager().AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_JOURNAL);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		GetGame().GetInputManager().RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Max(0, m_iSelected - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		m_iSelected = Math.Min(Math.Max(0, m_aMissions.Count() - 1), m_iSelected + 1);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		m_aMissions.Clear();
		m_aClues.Clear();
		string lab = "";
		if (ui)
		{
			array<ref array<string>> recs = {};
			Records(ui.GetJournal(), recs);
			foreach (array<string> f : recs)
			{
				if (RZ_UIText.Str(f, 0) == "M")
					m_aMissions.Insert(f);
				else if (RZ_UIText.Str(f, 0) == "C")
					m_aClues.Insert(f);
				else if (RZ_UIText.Str(f, 0) == "S")
					lab = RZ_UIText.Str(f, 1);
			}
		}

		// The lab's line: research, what waits in its backlog and how long that takes
		RZ_UIW.Show(m_wRoot, "LabStatus", !lab.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "LabStatus", lab);
		RZ_UIW.Show(m_wRoot, "Empty", m_aMissions.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "Header", string.Format("%1 OPEN", m_aMissions.Count()));
		DrawList();
		if (!m_aMissions.IsEmpty())
			DrawDetail(m_aMissions[Math.Min(m_iSelected, m_aMissions.Count() - 1)]);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawList()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "List");
		RZ_UIW.ClearChildren(list);
		ResourceName rowLayout = RZ_UISettings.Get().m_sMissionRowLayout;
		foreach (int i, array<string> m : m_aMissions)
		{
			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				return;

			int giver = RZ_UIText.Int(m, 3);
			Color accent = RZ_UIColor.Hazard();
			string sub = RZ_UIText.Str(m, 6);
			if (giver >= 0)
			{
				accent = RZ_UIColor.Faction(giver);
				sub = RZ_Factions.GetName(giver) + " - " + sub;
			}

			RZ_UIW.Icon(row, "RowIcon", RZ_UIText.MissionIcon(RZ_UIText.Int(m, 2)));
			RZ_UIW.Tint(row, "RowIcon", accent);
			RZ_UIW.Tint(row, "RowAccent", accent);
			RZ_UIW.Show(row, "RowSelected", i == m_iSelected);
			RZ_UIW.SetText(row, "RowTitle", RZ_UIText.Str(m, 4));
			RZ_UIW.SetText(row, "RowSub", sub);
			RZ_UIW.SetText(row, "RowDir", RZ_UIText.Str(m, 7));
			// A questline has no clock (minutes -1)
			string rowTime = RZ_UIText.Str(m, 8) + " min left";
			if (RZ_UIText.Int(m, 8) < 0)
				rowTime = "Questline";

			RZ_UIW.SetText(row, "RowTime", rowTime);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A click on a row (its root, MissionRow, is a Button) selects it.
	override protected void OnClickOther(Widget w, string name)
	{
		int idx = RZ_UIW.GetTag(w);
		if (name == "MissionRow" && idx >= 0 && idx < m_aMissions.Count())
			m_iSelected = idx;
	}

	//------------------------------------------------------------------------------------------------
	//! M|id|kind|giver|title|given by|where|direction|minutes|requirements|Fame|Favor|brief|taking part
	protected void DrawDetail(array<string> m)
	{
		int giver = RZ_UIText.Int(m, 3);
		Color accent = RZ_UIColor.Hazard();
		string icon = RZ_UIText.MissionIcon(RZ_UIText.Int(m, 2));
		if (giver >= 0)
		{
			accent = RZ_UIColor.Faction(giver);
			icon = RZ_UIText.FactionIcon(giver);
		}

		RZ_UIW.Tint(m_wRoot, "DetailAccent", accent);
		RZ_UIW.Tint(m_wRoot, "DetailIcon", accent);
		RZ_UIW.Icon(m_wRoot, "DetailIcon", icon);
		RZ_UIW.SetText(m_wRoot, "DetailTitle", RZ_UIText.Str(m, 4));
		string by = RZ_UIText.Str(m, 5);
		if (by.IsEmpty() && giver >= 0)
			by = RZ_Factions.GetName(giver);

		RZ_UIW.SetText(m_wRoot, "DetailBy", by);
		RZ_UIW.SetText(m_wRoot, "DetailBrief", RZ_UIText.Str(m, 12));
		RZ_UIW.SetText(m_wRoot, "DetailWhere", string.Format("%1\n%2", RZ_UIText.Str(m, 7), RZ_UIText.Str(m, 6)));
		string req = RZ_UIText.Str(m, 9);
		if (req.IsEmpty())
			req = "Open to anyone.";

		RZ_UIW.SetText(m_wRoot, "DetailReq", req);
		string rewards = "";
		if (RZ_UIText.Int(m, 8) < 0)
			rewards = "It depends on how it ends.";
		else if (giver >= 0)
			rewards = string.Format("About %1 Fame and %2 Favor with %3", RZ_UIText.Str(m, 10), RZ_UIText.Str(m, 11), RZ_Factions.GetNameInText(giver));

		RZ_UIW.SetText(m_wRoot, "DetailRewards", rewards);

		string id = RZ_UIText.Str(m, 1);
		string clues = "";
		foreach (array<string> c : m_aClues)
		{
			if (RZ_UIText.Str(c, 1) == id)
				clues += string.Format("%1\n%2\n\n", RZ_UIText.Str(c, 2), RZ_UIText.Str(c, 3));
		}

		RZ_UIW.Show(m_wRoot, "CluesBlock", !clues.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "DetailClues", clues);
	}
}

//------------------------------------------------------------------------------------------------
//! What you heard this life: the broadcast, open channels and cracked secure channels, newest
//! first. Kept on the client (RZ_PlayerUIComponent.OnNotice); F cycles the filter.
class RZ_RadioLogMenu : RZ_MenuBase
{
	protected int m_iFilter;	// 0 all, 1 open, 2 secure, 3 broadcast

	//------------------------------------------------------------------------------------------------
	override protected int TabIndex() { return 2; }

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return m_iFilter;

		return ui.GetRadioVersion() * 10 + m_iFilter;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		GetGame().GetInputManager().AddActionListener("RZ_RadioFilter", EActionTrigger.DOWN, OnFilter);
		RZ_UIW.OnClick(m_wRoot, "FilterButton", m_Click);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "FilterButton")
			OnFilter();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("RZ_RadioFilter", EActionTrigger.DOWN, OnFilter);
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFilter(float value = 0, EActionTrigger reason = 0)
	{
		m_iFilter = (m_iFilter + 1) % 4;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Passes(RZ_RadioEntry e)
	{
		if (m_iFilter == 1)
			return e.m_iKind == RZ_ENoticeKind.OPEN_CHANNEL;
		if (m_iFilter == 2)
			return e.m_iKind == RZ_ENoticeKind.SECURE_CHANNEL;
		if (m_iFilter == 3)
			return e.m_iKind == RZ_ENoticeKind.BROADCAST;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Redraw()
	{
		array<string> names = {"ALL", "OPEN", "SECURE", "BROADCAST"};
		RZ_UIW.SetText(m_wRoot, "FilterText", names[m_iFilter]);
		Widget list = RZ_UIW.Find(m_wRoot, "Log");
		RZ_UIW.ClearChildren(list);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		ResourceName rowLayout = RZ_UISettings.Get().m_sRadioRowLayout;
		int shown = 0;
		foreach (RZ_RadioEntry e : ui.GetRadioLog())
		{
			if (!Passes(e))
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

		RZ_UIW.Show(m_wRoot, "Empty", shown == 0);
	}
}

//------------------------------------------------------------------------------------------------
//! The map key: a static legend of the marker icons (the layout holds it all).
class RZ_MapKeyMenu : RZ_MenuBase
{
	//------------------------------------------------------------------------------------------------
	override protected int TabIndex() { return 3; }
}

//------------------------------------------------------------------------------------------------
//! A menu that isn't one of the four tabs: Q/E do nothing, Esc and the close button close it.
class RZ_StandaloneMenu : RZ_MenuBase
{
	//------------------------------------------------------------------------------------------------
	override protected int TabIndex() { return -1; }

	//------------------------------------------------------------------------------------------------
	override protected void OnTabLeft(float value = 0, EActionTrigger reason = 0)
	{
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTabRight(float value = 0, EActionTrigger reason = 0)
	{
	}
}

//------------------------------------------------------------------------------------------------
//! The notice board at the safehouse you're at (World/RZ_NoticeBoard.c): the notes, newest first,
//! and a box to write one (NoteInput, PostButton). Opened by the server when the board is read
//! (UI > Board menus); without it the board is a notice and notes are written with /note.
class RZ_NoticeBoardMenu : RZ_StandaloneMenu
{
	protected static bool s_bOpen;

	//------------------------------------------------------------------------------------------------
	static bool IsBoardOpen()
	{
		return s_bOpen;
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return 0;

		return ui.GetBoardVersion();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_bOpen = true;
		RZ_UIW.OnClick(m_wRoot, "PostButton", m_Click);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_BOARD);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		s_bOpen = false;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClickOther(Widget w, string name)
	{
		if (name != "PostButton")
			return;

		string text = RZ_UIW.GetEditText(m_wRoot, "NoteInput");
		if (text.IsEmpty())
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCommand(RZ_PlayerUIComponent.CMD_NOTE, text);

		RZ_UIW.SetEditText(m_wRoot, "NoteInput", "");
	}

	//------------------------------------------------------------------------------------------------
	//! B|safehouse|notes|longest note|can write, then N|author|age|text newest first
	override protected void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		array<ref array<string>> recs = {};
		Records(ui.GetBoard(), recs);
		string notes = "";
		bool canWrite = false;
		int limit = 160;
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "B")
			{
				string head = "NOTICE BOARD: " + RZ_UIText.Str(f, 1);
				head.ToUpper();
				RZ_UIW.SetText(m_wRoot, "Header", head);
				limit = RZ_UIText.Int(f, 3);
				canWrite = RZ_UIText.Int(f, 4) == 1;
			}
			else if (tag == "N")
			{
				notes += string.Format("%1, %2\n%3\n\n", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
			}
		}

		RZ_UIW.Show(m_wRoot, "Empty", notes.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "Notes", notes);
		RZ_UIW.Show(m_wRoot, "WriteBlock", canWrite);
		RZ_UIW.SetText(m_wRoot, "NoteLimit", string.Format("%1 characters at most. Everyone who comes by can read it.", limit));
	}
}

//------------------------------------------------------------------------------------------------
//! The hall of fame (Persistence/RZ_HallOfFame.c): this season's names, all time, and past seasons.
//! Opened with RZ_OpenHall; /hall in chat gives the same as a notice.
class RZ_HallOfFameMenu : RZ_StandaloneMenu
{
	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return 0;

		return ui.GetHallVersion();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_HALL);
	}

	//------------------------------------------------------------------------------------------------
	//! H|season, S and A|category|category name|name|value, P|season|winner|lines
	override protected void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		bool have = false;
		if (ui)
			have = !ui.GetHall().IsEmpty();

		RZ_UIW.Show(m_wRoot, "Waiting", !have);
		if (!have)
			return;

		array<ref array<string>> recs = {};
		Records(ui.GetHall(), recs);
		string seasonText = "";
		string allText = "";
		string pastText = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
			{
				RZ_UIW.SetText(m_wRoot, "Header", "SEASON " + RZ_UIText.Str(f, 1));
			}
			else if (tag == "S" || tag == "A")
			{
				string line = string.Format("%1\n%2", RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
				string value = RZ_UIText.Str(f, 4);
				if (!value.IsEmpty())
					line += "  (" + value + ")";

				if (tag == "S")
					seasonText += line + "\n\n";
				else
					allText += line + "\n\n";
			}
			else if (tag == "P")
			{
				pastText += string.Format("Season %1: the cure went to %2.\n%3\n\n", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2), RZ_UIText.Str(f, 3));
			}
		}

		if (seasonText.IsEmpty())
			seasonText = "Nobody's name is on it yet.";

		RZ_UIW.SetText(m_wRoot, "SeasonList", seasonText);
		RZ_UIW.SetText(m_wRoot, "AllTimeList", allText);
		RZ_UIW.SetText(m_wRoot, "PastList", pastText);
	}
}
