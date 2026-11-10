//! The ReforgerZ UI: shared data, settings and small widget helpers (docs/UI.md).
//!
//! Isolation: nothing outside scripts/Game/ReforgerZ/UI/ needs the UI to work. The server
//! feeds the owning client through RZ_PlayerUIComponent (on the player controller); the client
//! draws it with RZ_HudDisplay (an SCR_InfoDisplay) and the menus in RZ_Menus.c. Missing
//! layouts or a missing component just mean the vanilla hint is used, as before.
//!
//! Every engine UI call is in RZ_UIW below, so a wrong name is fixed in one place.

//------------------------------------------------------------------------------------------------
//! What a notice is, for its card style and the radio log.
enum RZ_ENoticeKind
{
	PLAIN,			// anything else: plain card, no accent
	FACTION,		// a faction's people (leaders, safehouses, loudspeakers)
	OPEN_CHANNEL,	// faction chatter, the Haven's channel, the radio station, the wind-up radio
	SECURE_CHANNEL,	// a cracked secure channel
	BROADCAST,		// the government broadcast
	INFECTION,		// the bite, the stages, cures
	EVENT			// world events
}

//------------------------------------------------------------------------------------------------
//! The infection timer's state, as sent to the client.
enum RZ_EInfectionUI
{
	NONE,
	GOLDEN,		// bitten, the golden window is open (the timer stands still)
	FEVER,
	WEAKNESS,
	FINAL
}

//------------------------------------------------------------------------------------------------
//! The journal's mission types (icons in docs/UI.md).
enum RZ_EMissionKindUI
{
	CONTRACT,
	EVENT,
	PUZZLE,
	LEADER,
	UNARMED,
	PERIMETER,
	ARC
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > UI. Read on every machine (the attribute values are on the
//! game mode prefab), so clients use them too.
[BaseContainerProps()]
class RZ_UISettings
{
	[Attribute("1", desc: "Styled notice cards (faction-coloured header, speaker) instead of the vanilla hint. Off, or with no HUD layout, every message uses the vanilla hint as before")]
	bool m_bStyledNotices = true;

	[Attribute("9", desc: "Seconds a notice card stays on screen")]
	float m_fNoticeSeconds = 9;

	[Attribute("3", desc: "Notice cards shown at once (newest on top); the rest wait their turn")]
	int m_iNoticeStack = 3;

	[Attribute("8", desc: "Notices waiting at most; older ones are dropped under heavy traffic")]
	int m_iNoticeQueue = 8;

	[Attribute("40", desc: "Radio log: messages kept on the client for this life")]
	int m_iRadioLogSize = 40;

	[Attribute("1", desc: "Show the infection timer (golden window, stages)")]
	bool m_bInfectionTimer = true;

	[Attribute("1", desc: "Show the noise meter (subtle: how much attention you draw where you stand)")]
	bool m_bNoiseMeter = true;

	[Attribute("60", desc: "Seconds between infection timer resyncs (the client counts down between them)")]
	float m_fResyncSeconds = 60;

	[Attribute("", UIWidgets.ResourceNamePicker, "Imageset with the ReforgerZ icons (UI/LAYOUTS.md). Empty = icons stay as set in the layouts", "imageset")]
	ResourceName m_sIconImageset;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout of one notice card (UI/LAYOUTS.md, NoticeCard)", "layout")]
	ResourceName m_sNoticeLayout;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout of one journal row (UI/LAYOUTS.md, MissionRow)", "layout")]
	ResourceName m_sMissionRowLayout;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout of one radio log row (UI/LAYOUTS.md, RadioRow)", "layout")]
	ResourceName m_sRadioRowLayout;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout of one map icon (UI/LAYOUTS.md, MapIcon). Set, the map shows each player what they know with the ReforgerZ icons (RZ_MapIconsUI on the map config) instead of vanilla text markers", "layout")]
	ResourceName m_sMapIconLayout;

	[Attribute("20", desc: "Map icons: seconds between refreshes while the map is open")]
	float m_fMapRefreshSeconds = 20;

	[Attribute("1", desc: "The deploy menu refuses background loadouts the player hasn't unlocked (UI/RZ_LoadoutLock.c). Off: picking one plays as the default background with its gear")]
	bool m_bLockLoadouts = true;

	[Attribute("0", desc: "The safehouse notice board opens as a menu with a box to write in (RZ_NoticeBoardMenu; needs its layout and chimeraMenus entry, UI/LAYOUTS.md). Off = notices, and notes are written with /note in chat")]
	bool m_bBoardMenus = false;

	protected static ref RZ_UISettings s_Defaults;

	//------------------------------------------------------------------------------------------------
	//! The icon layer replaces the vanilla text markers (RZ_GameModeComponent.AddMapMarker).
	bool UsesMapIcons()
	{
		return !m_sMapIconLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! The game mode's settings, else the defaults (always non-null).
	static RZ_UISettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetUISettings())
			return gm.GetUISettings();

		if (!s_Defaults)
			s_Defaults = new RZ_UISettings();

		return s_Defaults;
	}
}

//------------------------------------------------------------------------------------------------
//! Payload text: one record per line, fields split by '|'.
class RZ_UIText
{
	static const string SEP = "|";

	//------------------------------------------------------------------------------------------------
	//! A field that can't break the payload: no separators, no line breaks, at most maxLen characters.
	static string Clean(string text, int maxLen = 400)
	{
		string s = text;
		s.Replace("|", "/");
		s.Replace("\n", " ");
		if (maxLen > 0 && s.Length() > maxLen)
			s = s.Substring(0, maxLen - 3) + "...";

		return s;
	}

	//------------------------------------------------------------------------------------------------
	static void Split(string text, string delimiter, notnull array<string> parts)
	{
		parts.Clear();
		text.Split(delimiter, parts, false);
	}

	//------------------------------------------------------------------------------------------------
	static int Int(array<string> f, int i)
	{
		if (i >= f.Count())
			return 0;

		return f[i].ToInt();
	}

	//------------------------------------------------------------------------------------------------
	static float Float(array<string> f, int i)
	{
		if (i >= f.Count())
			return 0;

		return f[i].ToFloat();
	}

	//------------------------------------------------------------------------------------------------
	static string Str(array<string> f, int i)
	{
		if (i >= f.Count())
			return "";

		return f[i];
	}

	//------------------------------------------------------------------------------------------------
	//! "23:41" from seconds.
	static string Clock(float seconds)
	{
		int s = Math.Max(0, RZ_Util.RoundToInt(seconds));
		int m = s / 60;
		int r = s % 60;
		if (r < 10)
			return string.Format("%1:0%2", m, r);

		return string.Format("%1:%2", m, r);
	}

	//------------------------------------------------------------------------------------------------
	//! "19:42" from the time of day (hours), or "" if the world has no clock.
	static string TimeOfDay()
	{
		float h = RZ_GameTime.GetHour();
		if (h < 0)
			return "";

		int total = RZ_Util.RoundToInt(Math.Floor(h * 60));
		int hh = total / 60;
		int mm = total % 60;
		string hs = hh.ToString();
		string ms = mm.ToString();
		if (hh < 10)
			hs = "0" + hs;
		if (mm < 10)
			ms = "0" + ms;

		return hs + ":" + ms;
	}

	//------------------------------------------------------------------------------------------------
	//! A tier's access in words (docs/REPUTATION.md, "Standing tiers").
	static string TierNote(RZ_ETier tier)
	{
		switch (tier)
		{
			case RZ_ETier.STRANGER: return "They don't know you yet";
			case RZ_ETier.KNOWN: return "Their quartermaster will deal with you";
			case RZ_ETier.TRUSTED: return "Better stock, and their perk";
			case RZ_ETier.CHAMPION: return "Everything they have";
			case RZ_ETier.UNEASY: return "Counts as Stranger for access";
			case RZ_ETier.TOLERATED: return "Counts as Known for access";
			case RZ_ETier.RESPECTED: return "Counts as Trusted for access";
			case RZ_ETier.WILDCARD: return "Counts as Known for access";
			case RZ_ETier.WATCHED: return "They have an eye on you";
			case RZ_ETier.HUNTED: return "Their people shoot on sight";
			case RZ_ETier.KILL_ON_SIGHT: return "A price on your head";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The icon name for a tier (UI/Icons).
	static string TierIcon(RZ_ETier tier)
	{
		switch (tier)
		{
			case RZ_ETier.STRANGER: return "tier_stranger";
			case RZ_ETier.KNOWN: return "tier_known";
			case RZ_ETier.TRUSTED: return "tier_trusted";
			case RZ_ETier.CHAMPION: return "tier_champion";
			case RZ_ETier.WATCHED: return "tier_watched";
			case RZ_ETier.HUNTED: return "tier_hunted";
			case RZ_ETier.KILL_ON_SIGHT: return "tier_kill_on_sight";
		}

		return "tier_mixed";
	}

	//------------------------------------------------------------------------------------------------
	static string FactionIcon(int f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "faction_haven";
			case RZ_EFaction.MILITARY: return "faction_bastion";
			case RZ_EFaction.BANDITS: return "faction_redhand";
			case RZ_EFaction.INSTITUTE: return "faction_halcyon";
		}

		return "faction_infected";
	}

	//------------------------------------------------------------------------------------------------
	static string MissionIcon(int kind)
	{
		switch (kind)
		{
			case RZ_EMissionKindUI.EVENT: return "mission_event";
			case RZ_EMissionKindUI.PUZZLE: return "mission_puzzle";
			case RZ_EMissionKindUI.LEADER: return "mission_leader";
			case RZ_EMissionKindUI.UNARMED: return "mission_unarmed";
			case RZ_EMissionKindUI.PERIMETER: return "mission_perimeter";
			case RZ_EMissionKindUI.ARC: return "mission_arc";
		}

		return "mission_contract";
	}

	//------------------------------------------------------------------------------------------------
	static string NeedTitle(int need)
	{
		switch (need)
		{
			case RZ_ENeed.FOOD: return "FOOD";
			case RZ_ENeed.SUPPLIES: return "SUPPLIES";
			case RZ_ENeed.SECURITY: return "SECURITY";
			case RZ_ENeed.SAMPLES: return "SAMPLES";
			case RZ_ENeed.PEOPLE: return "PEOPLE";
			case RZ_ENeed.REVENGE: return "REVENGE";
		}

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! The Zone's palette (docs/UI.md, "Colours"). One accent per faction, hazard yellow for infection
//! and the Perimeter.
class RZ_UIColor
{
	//------------------------------------------------------------------------------------------------
	static Color Faction(int f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return RZ_UIW.Rgb(227, 163, 59);
			case RZ_EFaction.MILITARY: return RZ_UIW.Rgb(154, 163, 94);
			case RZ_EFaction.BANDITS: return RZ_UIW.Rgb(200, 53, 43);
			case RZ_EFaction.INSTITUTE: return RZ_UIW.Rgb(79, 184, 176);
		}

		return Bone();
	}

	static Color Bone() { return RZ_UIW.Rgb(232, 226, 208); }
	static Color Dim() { return RZ_UIW.Rgb(163, 158, 144); }
	static Color Hazard() { return RZ_UIW.Rgb(242, 194, 48); }
	static Color Good() { return RZ_UIW.Rgb(191, 211, 155); }
	static Color Bad() { return RZ_UIW.Rgb(228, 130, 111); }

	//------------------------------------------------------------------------------------------------
	static Color Stage(int state)
	{
		switch (state)
		{
			case RZ_EInfectionUI.GOLDEN: return Hazard();
			case RZ_EInfectionUI.FEVER: return RZ_UIW.Rgb(226, 179, 60);
			case RZ_EInfectionUI.WEAKNESS: return RZ_UIW.Rgb(224, 122, 46);
			case RZ_EInfectionUI.FINAL: return RZ_UIW.Rgb(210, 65, 47);
		}

		return Bone();
	}

	//------------------------------------------------------------------------------------------------
	static Color Tier(RZ_ETier tier)
	{
		if (RZ_Reputation.IsWatchedOrWorse(tier))
			return Bad();

		if (tier == RZ_ETier.KNOWN || tier == RZ_ETier.TRUSTED || tier == RZ_ETier.CHAMPION)
			return Good();

		return Dim();
	}
}

//------------------------------------------------------------------------------------------------
//! Classifies a notice by its title, on the client, so the 600-odd NotifyPlayer calls keep their
//! signature. Titles are the speaker ("Ines Calder, council head in Morton"), a channel
//! ("Radio: Bastion net", "Cracked: Bastion command net") or a subject ("Fever").
class RZ_NoticeStyle
{
	int m_iKind = RZ_ENoticeKind.PLAIN;
	int m_iFaction = -1;

	//------------------------------------------------------------------------------------------------
	static RZ_NoticeStyle Classify(string title)
	{
		RZ_NoticeStyle st = new RZ_NoticeStyle();
		string t = title;
		t.ToLower();

		if (t.Contains("government broadcast"))
		{
			st.m_iKind = RZ_ENoticeKind.BROADCAST;
			return st;
		}

		if (Starts(t, "cracked") || t.Contains("command net") || t.Contains("boss's channel") || t.Contains("lab channel") || t.Contains("scrambled channel"))
		{
			st.m_iKind = RZ_ENoticeKind.SECURE_CHANNEL;
			st.m_iFaction = FactionIn(t);
			return st;
		}

		if (t.Contains("radio station"))
		{
			st.m_iKind = RZ_ENoticeKind.OPEN_CHANNEL;
			return st;
		}

		if (Starts(t, "radio") || Starts(t, "wind-up radio") || t.Contains("open channel") || t.Contains("on your set"))
		{
			st.m_iKind = RZ_ENoticeKind.OPEN_CHANNEL;
			st.m_iFaction = FactionIn(t);
			// RZ_FactionVoice.ChannelTitle: the Red Hand use "the open channel"
			if (st.m_iFaction < 0 && t.Contains("open channel"))
				st.m_iFaction = RZ_EFaction.BANDITS;

			return st;
		}

		if (t == "bitten" || t == "infected" || t == "fever" || t == "weakness" || t == "the end of it" || t == "the bite" || t == "cured" || t == "infection" || t == "stopped")
		{
			st.m_iKind = RZ_ENoticeKind.INFECTION;
			return st;
		}

		if (Starts(t, "world event"))
		{
			st.m_iKind = RZ_ENoticeKind.EVENT;
			return st;
		}

		st.m_iFaction = FactionIn(t);
		if (st.m_iFaction >= 0)
			st.m_iKind = RZ_ENoticeKind.FACTION;

		return st;
	}

	//------------------------------------------------------------------------------------------------
	static bool Starts(string t, string prefix)
	{
		return t.IndexOf(prefix) == 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A faction from words in a lower-case title (names, leader titles from RZ_LeaderVoice.Title), or -1.
	static int FactionIn(string t)
	{
		if (t.Contains("haven") || t.Contains("safehouse") || t.Contains("council"))
			return RZ_EFaction.SURVIVORS;
		if (t.Contains("bastion") || t.Contains("outpost commander") || t.Contains("commanding officer") || t.Contains("loudspeaker"))
			return RZ_EFaction.MILITARY;
		if (t.Contains("red hand") || t.Contains("crew boss") || t.Contains("on a roof"))
			return RZ_EFaction.BANDITS;
		if (t.Contains("halcyon") || t.Contains("lab") || t.Contains("researcher") || t.Contains("research director"))
			return RZ_EFaction.INSTITUTE;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The card's header line: what kind of message, in capitals.
	string KindText()
	{
		switch (m_iKind)
		{
			case RZ_ENoticeKind.BROADCAST: return "GOVERNMENT BROADCAST";
			case RZ_ENoticeKind.SECURE_CHANNEL: return "SECURE CHANNEL";
			case RZ_ENoticeKind.OPEN_CHANNEL:
			{
				if (m_iFaction >= 0)
					return "OPEN CHANNEL - " + RZ_Factions.GetShortName(m_iFaction);

				return "OPEN CHANNEL";
			}
			case RZ_ENoticeKind.INFECTION: return "INFECTION";
			case RZ_ENoticeKind.EVENT: return "WORLD EVENT";
			case RZ_ENoticeKind.FACTION: return RZ_Factions.GetName(m_iFaction);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	string Icon()
	{
		switch (m_iKind)
		{
			case RZ_ENoticeKind.BROADCAST: return "status_broadcast";
			case RZ_ENoticeKind.SECURE_CHANNEL: return "status_secure_radio";
			case RZ_ENoticeKind.INFECTION: return "status_bitten";
			case RZ_ENoticeKind.EVENT: return "mission_event";
			case RZ_ENoticeKind.OPEN_CHANNEL:
			{
				if (m_iFaction >= 0)
					return RZ_UIText.FactionIcon(m_iFaction);

				return "status_radio_open";
			}
			case RZ_ENoticeKind.FACTION: return RZ_UIText.FactionIcon(m_iFaction);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	Color Accent()
	{
		switch (m_iKind)
		{
			case RZ_ENoticeKind.BROADCAST: return RZ_UIColor.Hazard();
			case RZ_ENoticeKind.INFECTION: return RZ_UIColor.Hazard();
			case RZ_ENoticeKind.EVENT: return RZ_UIColor.Hazard();
		}

		if (m_iFaction >= 0)
			return RZ_UIColor.Faction(m_iFaction);

		return RZ_UIColor.Dim();
	}

	//------------------------------------------------------------------------------------------------
	//! Goes in the radio log.
	bool IsRadio()
	{
		return m_iKind == RZ_ENoticeKind.OPEN_CHANNEL || m_iKind == RZ_ENoticeKind.SECURE_CHANNEL || m_iKind == RZ_ENoticeKind.BROADCAST;
	}
}

//------------------------------------------------------------------------------------------------
//! Every engine UI call in one place (widgets, input, menus, clock), each marked for checking.
class RZ_UIW
{
	//------------------------------------------------------------------------------------------------
	static Color Rgb(int r, int g, int b)
	{
		return Color.FromRGBA(r, g, b, 255);
	}

	//------------------------------------------------------------------------------------------------
	static Widget Find(Widget root, string name)
	{
		if (!root)
			return null;

		return root.FindAnyWidget(name);
	}

	//------------------------------------------------------------------------------------------------
	static void SetText(Widget root, string name, string text)
	{
		TextWidget t = TextWidget.Cast(Find(root, name));
		if (t)
			t.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	static void Show(Widget root, string name, bool show)
	{
		Widget w = Find(root, name);
		if (w)
			w.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	static void Tint(Widget root, string name, Color c)
	{
		Widget w = Find(root, name);
		if (w && c)
			w.SetColor(c);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets an image from the icon imageset (does nothing without one: the layout's image stays).
	static void Icon(Widget root, string name, string icon)
	{
		ImageWidget img = ImageWidget.Cast(Find(root, name));
		ResourceName imageset = RZ_UISettings.Get().m_sIconImageset;
		if (!img || imageset.IsEmpty() || icon.IsEmpty())
			return;

		img.LoadImageFromSet(0, imageset, icon);
	}

	//------------------------------------------------------------------------------------------------
	//! The text typed into an edit box, "" if there's none.
	static string GetEditText(Widget root, string name)
	{
		EditBoxWidget box = EditBoxWidget.Cast(Find(root, name));
		if (!box)
			return "";

		return box.GetText();
	}

	//------------------------------------------------------------------------------------------------
	static void SetEditText(Widget root, string name, string text)
	{
		EditBoxWidget box = EditBoxWidget.Cast(Find(root, name));
		if (box)
			box.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	static void Progress(Widget root, string name, float share)
	{
		ProgressBarWidget p = ProgressBarWidget.Cast(Find(root, name));
		if (!p)
			return;

		p.SetMin(0);
		p.SetMax(1);
		p.SetCurrent(Math.Clamp(share, 0, 1));
	}

	//------------------------------------------------------------------------------------------------
	static Widget Create(ResourceName layout, Widget parent)
	{
		if (layout.IsEmpty() || !parent)
			return null;

		return GetGame().GetWorkspace().CreateWidgets(layout, parent);
	}

	//------------------------------------------------------------------------------------------------
	static void Remove(Widget w)
	{
		if (w)
			w.RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	static void ClearChildren(Widget parent)
	{
		if (!parent)
			return;

		Widget c = parent.GetChildren();
		while (c)
		{
			Widget next = c.GetSibling();
			c.RemoveFromHierarchy();
			c = next;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds on this machine (for the client-side countdowns).
	static float Now()
	{
		return System.GetTickCount() / 1000.0; // milliseconds since the game started
	}


	//------------------------------------------------------------------------------------------------
	static void OpenMenu(int preset)
	{
		MenuManager mm = GetGame().GetMenuManager();
		if (mm)
			mm.OpenMenu(preset);
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: `handler` hears clicks on the named widget, or on root itself if name is "" (make it a
	//! Button in the layout).
	static void OnClick(Widget root, string name, ScriptedWidgetEventHandler handler)
	{
		Widget w = root;
		if (!name.IsEmpty())
			w = Find(root, name);

		if (w && handler)
			w.AddHandler(handler);
	}

	//------------------------------------------------------------------------------------------------
	//! A number stored on a widget (a journal row's index), read back by the click handler.
	static void SetTag(Widget w, int tag)
	{
		if (w)
			w.SetUserID(tag);
	}

	//------------------------------------------------------------------------------------------------
	static int GetTag(Widget w)
	{
		if (!w)
			return -1;

		return w.GetUserID();
	}

	//------------------------------------------------------------------------------------------------
	//! Map: screen position (layout units) of a world spot on the open map. False if there's no map.
	static bool MapToScreen(SCR_MapEntity mapEnt, float x, float z, out float sx, out float sy)
	{
		if (!mapEnt)
			return false;

		int px, py;
		mapEnt.WorldToScreen(x, z, px, py, true);
		WorkspaceWidget ws = GetGame().GetWorkspace();
		sx = ws.DPIUnscale(px);
		sy = ws.DPIUnscale(py);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void SetPos(Widget w, float x, float y)
	{
		if (w)
			FrameSlot.SetPos(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	//! The loadout the local player has picked in the deploy menu, or "".
	static string SelectedLoadoutName()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return "";

		SCR_PlayerLoadoutComponent lc = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		if (!lc)
			return "";

		SCR_BasePlayerLoadout loadout = lc.GetLoadout();
		if (!loadout)
			return "";

		return loadout.GetLoadoutName();
	}
}
