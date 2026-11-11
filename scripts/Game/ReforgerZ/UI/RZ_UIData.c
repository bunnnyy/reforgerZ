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
	EVENT,			// world events
	MISSION,		// contract offers, "this is the place", done, failed, refused (tagged by the server)
	WORLD			// the state of the world at spawn: the outbreak, the news, the season, the hall
}

//------------------------------------------------------------------------------------------------
//! How important a notice is: the HUD shows the most important first (docs/UI.md, "Notices").
enum RZ_ENoticePriority
{
	LOW,		// news and welcome
	NORMAL,		// radio and faction voices
	HIGH,		// about you, now
	URGENT		// danger to you, now: always first, and may cut a low card short
}

//------------------------------------------------------------------------------------------------
//! What the server says a notice is (RZ_GameModeComponent.NotifyPlayerAs). AUTO: the client
//! works it out from the title, as for every plain NotifyPlayer. MISSION, EVENT and WORLD also say
//! where it's listed in the notebook's Radio tab; URGENT, HIGH and LOW only set its importance.
enum RZ_ENoticeTag
{
	AUTO,
	MISSION,	// offers, "this is the place", done, failed, refused
	EVENT,		// world events
	WORLD,		// the state of the world at spawn: the outbreak, the news, the season, the hall
	URGENT,
	HIGH,
	LOW
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
	ARC,
	QUESTLINE,	// a questline's entry and its stage missions (the arc icon)
	ERRAND,		// an errand you hold (Missions/Errands): the unarmed icon until mission_errand is drawn
	PACK		// a pack you know of (docs/INFECTED.md, Knowing): the range ring, the level as its time
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > UI. Read on every machine (the attribute values are on the
//! game mode prefab), so clients use them too.
[BaseContainerProps()]
class RZ_UISettings
{
	[Attribute("1", desc: "Styled notice cards (faction-coloured header, speaker) instead of the vanilla hint. Off, or with no HUD layout, every message uses the vanilla hint as before")]
	bool m_bStyledNotices;

	[Attribute("9", desc: "Seconds a notice card stays on screen")]
	float m_fNoticeSeconds;

	[Attribute("2", desc: "Notice cards at once; the rest wait their turn, the most important first")]
	int m_iNoticeStack;

	[Attribute("2", desc: "Seconds between one card and the next (urgent cards don't wait)")]
	float m_fNoticeGap;

	[Attribute("45", desc: "Seconds a low or normal notice that's in the notebook's Radio tab waits for a card before it goes to the notebook only")]
	float m_fNoticeWait;

	[Attribute("4", desc: "Calm after spawn: for this many seconds after a new life only urgent and high notices show")]
	float m_fNoticeCalm;

	[Attribute("12", desc: "Notices waiting at most; past it the least important goes to the notebook only (or is dropped if it isn't listed there)")]
	int m_iNoticeQueue;

	[Attribute("60", desc: "Radio tab: radio, mission and world notices kept on the client for this life")]
	int m_iRadioLogSize;

	[Attribute("1", desc: "Show the infection timer (golden window, stages)")]
	bool m_bInfectionTimer;

	[Attribute("1", desc: "Show the noise meter (subtle: how much attention you draw where you stand), and above it how visible you are while Stealth is on")]
	bool m_bNoiseMeter;

	[Attribute("60", desc: "Seconds between infection timer resyncs (the client counts down between them)")]
	float m_fResyncSeconds;

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
	float m_fMapRefreshSeconds;

	[Attribute("1", desc: "The deploy menu refuses background loadouts the player hasn't unlocked (UI/RZ_LoadoutLock.c). Off: picking one plays as the default background with its gear")]
	bool m_bLockLoadouts;

	[Attribute("1", desc: "The safehouse notice board opens as a menu with a box to write in and ready-made notes a pad can pin up without typing (RZ_NoticeBoardMenu; needs its layout and chimeraMenus entry, UI/LAYOUTS.md). Off = notices, and notes are written with /note in chat (no pad route)")]
	bool m_bBoardMenus;

	[Attribute("1", desc: "Subtitles: what people say outside the talk screen shows at the bottom centre (needs SubtitlePanel in the HUD layout). Off = notice cards, as before")]
	bool m_bSubtitles;

	[Attribute("2.5", desc: "Subtitles: shortest time a line stays (s)")]
	float m_fSubtitleMin;

	[Attribute("8", desc: "Subtitles: longest time a line stays (s); longer lines are split into parts shown in turn")]
	float m_fSubtitleMax;

	[Attribute("18", desc: "Subtitles: reading speed (characters per second) that sets how long a line stays")]
	float m_fSubtitleCps;

	[Attribute("1", desc: "The reader: notes, letters, orders and flyers open on a sheet of paper (RZ_ReaderMenu). Off = notice cards, as before")]
	bool m_bReader;

	[Attribute("700", desc: "Reader: about this many characters to a page; pages break at the end of a paragraph")]
	int m_iReaderPageChars;

	[Attribute("", UIWidgets.ResourceNamePicker, "Talk screen: the layout of one reply row (UI/LAYOUTS.md, TalkReply). Empty = no talk screen: a person's actions and notices, as before", "layout")]
	ResourceName m_sTalkReplyLayout;

	[Attribute("", UIWidgets.ResourceNamePicker, "Car repair screen: the layout of one job row (UI/LAYOUTS.md, CarJob). Empty = no repair screen: the car's actions and notices, as before", "layout")]
	ResourceName m_sCarJobLayout;

	[Attribute("", UIWidgets.ResourceNamePicker, "Car repair screen: the side view of the car, a white line drawing (UI/Textures/ReforgerZ/RZ_CarSide.png, imported). Empty = the job markers on a plain panel", "edds")]
	ResourceName m_sCarPicture;

	[Attribute("", UIWidgets.ResourceNamePicker, "Shop screen: the layout of one item row (UI/LAYOUTS.md, ShopItem). Empty = no shop screen: a quartermaster's Buy, Requisition and service actions, as before", "layout")]
	ResourceName m_sShopItemLayout;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Standing page layout (UI/LAYOUTS.md). Empty = the tab says Nothing here yet", "layout")]
	ResourceName m_sStandingPage;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Journal page layout (UI/LAYOUTS.md)", "layout")]
	ResourceName m_sJournalPage;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Radio page layout (UI/LAYOUTS.md)", "layout")]
	ResourceName m_sRadioPage;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Map key page layout (UI/LAYOUTS.md)", "layout")]
	ResourceName m_sMapKeyPage;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Hall of fame page layout (UI/LAYOUTS.md)", "layout")]
	ResourceName m_sHallPage;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The notebook item every new life is given (Prefabs/ReforgerZ/UI/RZ_Notebook.et): taken into the hand it opens the notebook, the pad's way in besides the pause menu. Empty = none", "et")]
	ResourceName m_sNotebookPrefab;

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
			s_Defaults = RZ_UISettings.Cast(RZ_Util.NewWithDefaults("RZ_UISettings"));

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
	//! Where to cut `text` at or before `limit` characters: after a sentence end, else at a space,
	//! else at the limit.
	static int CutAt(string text, int limit)
	{
		if (text.Length() <= limit)
			return text.Length();

		int space = -1;
		for (int i = limit; i > limit / 2; i--)
		{
			string ch = text.Substring(i, 1);
			if (ch == "\n")
				return i + 1;

			if (ch == " ")
			{
				string before = text.Substring(i - 1, 1);
				if (before == "." || before == "?" || before == "!" || before == "\"")
					return i + 1;

				if (space < 0)
					space = i + 1;
			}
		}

		if (space > 0)
			return space;

		return limit;
	}

	//------------------------------------------------------------------------------------------------
	//! A subtitle's parts: at most `limit` characters each (two lines), cut at sentence ends.
	static void SplitLine(string text, int limit, notnull array<string> parts)
	{
		parts.Clear();
		string rest = text.Trim();
		int guard = 0;
		while (!rest.IsEmpty() && guard < 20)
		{
			guard++;
			int cut = CutAt(rest, limit);
			string part = rest.Substring(0, cut);
			parts.Insert(part.Trim());
			rest = rest.Substring(cut, rest.Length() - cut);
			rest = rest.Trim();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The reader's pages: whole paragraphs (split by line breaks) up to about `chars` characters a
	//! page; a paragraph longer than a page is cut at a sentence end.
	static void Paginate(string text, int chars, notnull array<string> pages)
	{
		pages.Clear();
		int size = Math.Max(200, chars);
		array<string> paras = {};
		Split(text, "\n", paras);
		string page = "";
		foreach (string para : paras)
		{
			string p = para.Trim();
			if (p.IsEmpty())
				continue;

			while (p.Length() > size)
			{
				if (!page.IsEmpty())
				{
					pages.Insert(page);
					page = "";
				}

				int cut = CutAt(p, size);
				string piece = p.Substring(0, cut);
				pages.Insert(piece.Trim());
				p = p.Substring(cut, p.Length() - cut);
				p = p.Trim();
			}

			if (p.IsEmpty())
				continue;

			if (!page.IsEmpty() && page.Length() + p.Length() > size)
			{
				pages.Insert(page);
				page = "";
			}

			if (page.IsEmpty())
				page = p;
			else
				page += "\n\n" + p;
		}

		if (!page.IsEmpty() || pages.IsEmpty())
			pages.Insert(page);
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
			case RZ_EMissionKindUI.QUESTLINE: return "mission_arc";
			case RZ_EMissionKindUI.ERRAND: return "mission_unarmed";
			case RZ_EMissionKindUI.PACK: return "marker_range";
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
	int m_iPriority = RZ_ENoticePriority.NORMAL;

	//------------------------------------------------------------------------------------------------
	//! The kind from the title, then the server's tag (RZ_ENoticeTag): a mission, event or world tag
	//! sets the kind (radio messages keep theirs), and the priority follows from both.
	static RZ_NoticeStyle Classify(string title, int tag = RZ_ENoticeTag.AUTO)
	{
		RZ_NoticeStyle st = FromTitle(title);
		bool radio = st.IsRadio();
		if (tag == RZ_ENoticeTag.MISSION && !radio)
			st.m_iKind = RZ_ENoticeKind.MISSION;
		else if (tag == RZ_ENoticeTag.EVENT && !radio)
			st.m_iKind = RZ_ENoticeKind.EVENT;
		else if (tag == RZ_ENoticeTag.WORLD && !radio)
			st.m_iKind = RZ_ENoticeKind.WORLD;

		st.m_iPriority = PriorityFor(title, st.m_iKind, tag);
		return st;
	}

	//------------------------------------------------------------------------------------------------
	//! docs/UI.md, "Notices": urgent is danger to you now, high is about you now, normal the radio
	//! and faction voices, low the news and welcome. A tag decides; else the title.
	static int PriorityFor(string title, int kind, int tag)
	{
		switch (tag)
		{
			case RZ_ENoticeTag.URGENT: return RZ_ENoticePriority.URGENT;
			case RZ_ENoticeTag.HIGH: return RZ_ENoticePriority.HIGH;
			case RZ_ENoticeTag.LOW: return RZ_ENoticePriority.LOW;
			case RZ_ENoticeTag.MISSION: return RZ_ENoticePriority.HIGH;
			case RZ_ENoticeTag.WORLD: return RZ_ENoticePriority.LOW;
		}

		if (kind == RZ_ENoticeKind.INFECTION)
			return RZ_ENoticePriority.URGENT;

		string t = title;
		t.ToLower();
		if (t == "horde" || t == "the final stand")
			return RZ_ENoticePriority.URGENT;

		if (t == "background unlocked" || t == "mission complete" || t == "mission failed" || Starts(t, "contract:") || t.EndsWith(" is locked"))
			return RZ_ENoticePriority.HIGH;

		if (Starts(t, "outbreak:") || t == "word on the roads" || t == "this season" || t == "hall of fame" || t == "radio")
			return RZ_ENoticePriority.LOW;

		return RZ_ENoticePriority.NORMAL;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_NoticeStyle FromTitle(string title)
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
			case RZ_ENoticeKind.WORLD: return "THE ZONE";
			case RZ_ENoticeKind.MISSION:
			{
				if (m_iFaction >= 0)
					return "MISSION - " + RZ_Factions.GetShortName(m_iFaction);

				return "MISSION";
			}
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
			case RZ_ENoticeKind.MISSION: return "mission_contract";
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
	bool IsRadio()
	{
		return m_iKind == RZ_ENoticeKind.OPEN_CHANNEL || m_iKind == RZ_ENoticeKind.SECURE_CHANNEL || m_iKind == RZ_ENoticeKind.BROADCAST;
	}

	//------------------------------------------------------------------------------------------------
	//! Listed in the notebook's Radio tab: radio messages, missions, world events and the world at spawn.
	bool IsLogged()
	{
		return IsRadio() || m_iKind == RZ_ENoticeKind.MISSION || m_iKind == RZ_ENoticeKind.EVENT || m_iKind == RZ_ENoticeKind.WORLD;
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
	//! Sets an image from an imported texture (.edds), as vanilla's credits do. Nothing if empty.
	static void Texture(Widget root, string name, ResourceName texture)
	{
		ImageWidget img = ImageWidget.Cast(Find(root, name));
		if (img && !texture.IsEmpty())
			img.LoadImageTexture(0, texture);
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
	//! Gamepad: scroll the named ScrollLayout a step up (-1) or down (1).
	static void Scroll(Widget root, string name, int dir)
	{
		ScrollLayoutWidget sc = ScrollLayoutWidget.Cast(Find(root, name));
		if (!sc)
			return;

		float x, y;
		sc.GetSliderPos(x, y);
		sc.SetSliderPos(x, Math.Clamp(y + dir * 0.12, 0, 1));
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds on this machine (for the client-side countdowns).
	static float Now()
	{
		return System.GetTickCount() / 1000.0; // milliseconds since the game started
	}


	//------------------------------------------------------------------------------------------------
	//! The opened menu, or null (no preset in chimeraMenus.conf, or its layout is missing).
	static MenuBase OpenMenu(int preset)
	{
		MenuManager mm = GetGame().GetMenuManager();
		if (!mm)
			return null;

		return mm.OpenMenu(preset);
	}

	//------------------------------------------------------------------------------------------------
	//! Gamepad: give the named widget the focus, so the D-pad has something to move from.
	//! VERIFY: WorkspaceWidget.SetFocusedWidget focuses a Button in a ChimeraMenuBase (vanilla's
	//! pause menu does the same).
	static void Focus(Widget root, string name)
	{
		Widget w = root;
		if (!name.IsEmpty())
			w = Find(root, name);

		FocusWidget(w);
	}

	//------------------------------------------------------------------------------------------------
	static void FocusWidget(Widget w)
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (ws && w)
			ws.SetFocusedWidget(w);
	}

	//------------------------------------------------------------------------------------------------
	//! Footer prompts: vanilla's input button (WLib_InputNavigationButton), which shows the key or
	//! the pad button for the action by the device in use. Created into the HorizontalLayout
	//! named `parentName`; true if it was made (the layout's static PromptsText can then hide).
	//! VERIFY: the prompt switches glyphs between keyboard and pad in our layouts.
	static bool AddPrompt(Widget root, string parentName, string action, string label)
	{
		Widget parent = Find(root, parentName);
		if (!parent)
			return false;

		Widget w = GetGame().GetWorkspace().CreateWidgets(PROMPT_LAYOUT, parent);
		if (!w)
			return false;

		SCR_InputButtonComponent btn = SCR_InputButtonComponent.FindComponent(w);
		if (!btn)
		{
			w.RemoveFromHierarchy();
			return false;
		}

		btn.SetAction(action);
		btn.SetLabel(label);
		return true;
	}

	static const ResourceName PROMPT_LAYOUT = "{0EC7676429DDCBC6}UI/layouts/WidgetLibrary/Buttons/WLib_InputNavigationButton.layout";

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
	//! A widget's size in its frame (the map's range rings).
	static void SetSize(Widget w, float width, float height)
	{
		if (w)
			FrameSlot.SetSize(w, width, height);
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
