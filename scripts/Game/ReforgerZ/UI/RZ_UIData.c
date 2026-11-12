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
	ERRAND,		// an errand you hold (Missions/Errands): mission_errand
	PACK,		// a pack you know of (docs/INFECTED.md, Knowing): the range ring, the level as its time
	DOORS,		// Read on doors (docs/TRACES.md): the last door lines you read, the puzzle icon
	FINALE		// the cure's finale page (docs/FINALE.md): the readout, the worth, your route; the lab icon
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

	[Attribute("", UIWidgets.ResourceNamePicker, "Map legend: the map key in the full-screen map's bottom-left (UI/LAYOUTS.md, MapLegend), shown zoomed out and faded as you zoom in; the RZ_MapLegend input (I, pad Y) shows or hides it. Needs the map icon layout. Empty = no legend", "layout")]
	ResourceName m_sMapLegendLayout;

	[Attribute("150", desc: "Map legend: fully shown while a kilometre of map is at most this many units across on screen (zoomed out)")]
	float m_fLegendFullZoom;

	[Attribute("450", desc: "Map legend: gone once a kilometre of map is this many units across on screen (zoomed in); it fades in between")]
	float m_fLegendGoneZoom;

	[Attribute("1", desc: "The server refuses background loadouts the player hasn't unlocked (UI/RZ_LoadoutLock.c), whatever screen asks. Off: picking one plays as the default background with its gear")]
	bool m_bLockLoadouts;

	[Attribute("", UIWidgets.ResourceNamePicker, "Spawn screen: the layout of one background row (UI/LAYOUTS.md, SpawnRow). Set, our spawn screen (RZ_SpawnMenu, the background picker) opens instead of vanilla's deploy menu; it needs the RZ_SpawnMenu preset in chimeraMenus.conf. Empty = vanilla's deploy menu", "layout")]
	ResourceName m_sSpawnRowLayout;

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

	[Attribute("", UIWidgets.ResourceNamePicker, "Crafting screen: the layout of one recipe row (UI/LAYOUTS.md, CraftRow). Empty = no crafting screen: the workbench's Make actions and notices, as before", "layout")]
	ResourceName m_sCraftRowLayout;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Standing page layout (UI/LAYOUTS.md). Empty = the tab says Nothing here yet", "layout")]
	ResourceName m_sStandingPage;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Journal page layout (UI/LAYOUTS.md)", "layout")]
	ResourceName m_sJournalPage;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Radio page layout (UI/LAYOUTS.md)", "layout")]
	ResourceName m_sRadioPage;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: the Hall of fame page layout (UI/LAYOUTS.md)", "layout")]
	ResourceName m_sHallPage;

	[Attribute("", UIWidgets.ResourceNamePicker, "Notebook: The season page layout (UI/LAYOUTS.md): the majors, and in the epilogue the season as the Zone tells it", "layout")]
	ResourceName m_sSeasonPage;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The notebook item every new life is given (Prefabs/ReforgerZ/UI/RZ_Notebook.et): taken into the hand it opens the notebook, the pad's way in besides the pause menu. Empty = none", "et")]
	ResourceName m_sNotebookPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The compass every new life is given (vanilla Compass_Adrianov.et), unless the loadout already has one. Empty = none", "et")]
	ResourceName m_sCompassPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The paper map every new life is given (vanilla PaperMap_01_folded.et), unless the loadout already has one. Empty = none", "et")]
	ResourceName m_sMapPrefab;

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
			case RZ_EMissionKindUI.ERRAND: return "mission_errand";
			case RZ_EMissionKindUI.PACK: return "marker_range";
			case RZ_EMissionKindUI.DOORS: return "mission_puzzle";
			case RZ_EMissionKindUI.FINALE: return "marker_lab";
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
	//! A widget's opacity, 0 to 1; hidden at 0 (the map legend's fade).
	static void Fade(Widget w, float alpha)
	{
		if (!w)
			return;

		w.SetOpacity(Math.Clamp(alpha, 0, 1));
		w.SetVisible(alpha > 0.01);
	}

	//------------------------------------------------------------------------------------------------
	//! A widget's size in its frame (the map's range rings).
	static void SetSize(Widget w, float width, float height)
	{
		if (w)
			FrameSlot.SetSize(w, width, height);
	}

	//------------------------------------------------------------------------------------------------
	//! Turns the named image, degrees clockwise from up (the map's you-are-here, the tracker's arrow).
	//! VERIFY: ImageWidget.SetRotation turns the image about its centre, clockwise in degrees.
	static void Rotate(Widget root, string name, float degrees)
	{
		ImageWidget img = ImageWidget.Cast(Find(root, name));
		if (img)
			img.SetRotation(degrees);
	}

	//------------------------------------------------------------------------------------------------
	//! The entity this machine's player controls, null when there's none (dead, spectating).
	static IEntity LocalEntity()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;

		return pc.GetControlledEntity();
	}

	//------------------------------------------------------------------------------------------------
	//! Where an entity faces, degrees clockwise from north (its forward axis on the ground).
	static float Heading(IEntity ent)
	{
		if (!ent)
			return 0;

		vector mat[4];
		ent.GetWorldTransform(mat);
		vector fwd = mat[2];
		float deg = Math.Atan2(fwd[0], fwd[2]) * Math.RAD2DEG;
		if (deg < 0)
			deg += 360;

		return deg;
	}

	//------------------------------------------------------------------------------------------------
	//! The bearing from one spot to another, degrees clockwise from north.
	static float Bearing(vector from, vector to)
	{
		float deg = Math.Atan2(to[0] - from[0], to[2] - from[2]) * Math.RAD2DEG;
		if (deg < 0)
			deg += 360;

		return deg;
	}
}

//------------------------------------------------------------------------------------------------
//! How a spoken line reaches you (docs/UI.md, "Subtitles"): the icon before the name, and where on
//! the screen it shows.
enum RZ_ESaySource
{
	IN_PERSON,		// someone near you: the speech bubble, an arrow to them and the distance
	RADIO,			// a radio channel: the radio, in the radio-line box at the top left
	LOUDSPEAKER,	// Bastion's loudspeakers, the Perimeter line, a speaker on a post, a loudhailer
	NOTE			// a sign, a board or a note, read out
}

//------------------------------------------------------------------------------------------------
//! One spoken line as the HUD shows it (RZ_GameModeComponent.Say, RZ_HudDisplay): who says it (a
//! name, a short role, their faction), how it reaches you, a radio line's channel, and where an
//! in-person speaker stands. Worked out from the title the line comes under ("Ines Calder, council
//! head in Morton", "Loudspeaker", "Radio: Bastion net") and the line itself, so the callers keep
//! their signature. The server sends it (RZ_PlayerUIComponent.SendSayLine); the client builds one
//! the same way from a plain spoken line or a radio notice.
class RZ_SayLine
{
	static const float NEAR = 15;		// server: a line without a body is pinned on the nearest person this close to the player
	static const int ROLE_CHARS = 48;

	string m_sSpeaker;		// the title as given ("Name, role"), for the card it falls back to
	string m_sName;
	string m_sRole;
	string m_sChannel;		// a radio line's channel ("Bastion net")
	string m_sText;
	int m_iFaction = -1;
	int m_iSource;
	bool m_bSecure;			// a cracked secure channel (client, from the notice's title)
	vector m_vAt;			// where the speaker stood when it was said; vector.Zero: nowhere to point
	RplId m_Who;			// their body, followed on the client while it streams in
	IEntity m_Body;			// client: that body, once found

	//------------------------------------------------------------------------------------------------
	void RZ_SayLine()
	{
		m_Who = RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	//! A line from the title it comes under. faction: RZ_EFaction or -1.
	static RZ_SayLine Make(string speaker, int faction, string text)
	{
		RZ_SayLine line = new RZ_SayLine();
		line.m_sSpeaker = speaker;
		line.m_iFaction = faction;
		line.m_sText = text;
		line.m_iSource = SourceFor(speaker, text);
		if (line.m_iSource == RZ_ESaySource.RADIO)
		{
			line.m_sChannel = ChannelOf(speaker);
			if (line.m_sChannel.IsEmpty())
				line.m_sChannel = speaker;

			return line;
		}

		int comma = speaker.IndexOf(", ");
		if (comma > 0)
		{
			line.m_sName = speaker.Substring(0, comma);
			line.m_sRole = speaker.Substring(comma + 2, speaker.Length() - comma - 2).Trim();
		}
		else
		{
			line.m_sName = speaker;
		}

		line.m_sRole = RoleWithFaction(line.m_sName, line.m_sRole, faction);
		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! A radio channel's voice (a notice titled "Radio: Bastion net", "Cracked: ..."), for the radio-line box.
	static RZ_SayLine Radio(string title, string channel, int faction, string text)
	{
		RZ_SayLine line = new RZ_SayLine();
		line.m_sSpeaker = title;
		line.m_sChannel = channel;
		line.m_iFaction = faction;
		line.m_sText = text;
		line.m_iSource = RZ_ESaySource.RADIO;
		string t = title;
		t.ToLower();
		line.m_bSecure = t.IndexOf("cracked") == 0;
		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! The role says whose they are: "Haven council head in Morton", not "council head in Morton".
	//! Left alone when the name or role already names the faction; cut to ROLE_CHARS.
	protected static string RoleWithFaction(string name, string role, int faction)
	{
		string result = role;
		if (RZ_Factions.IsValid(faction))
		{
			string shortName = RZ_Factions.GetShortName(faction);
			string both = name + " " + role;
			both.ToLower();
			string shortLower = shortName;
			shortLower.ToLower();
			if (!both.Contains(shortLower))
			{
				string bare = role;
				string bareLower = role;
				bareLower.ToLower();
				if (bareLower.IndexOf("the ") == 0)
					bare = role.Substring(4, role.Length() - 4);

				result = (shortName + " " + bare).Trim();
			}
		}

		if (result.Length() > ROLE_CHARS)
			result = result.Substring(0, ROLE_CHARS - 3) + "...";

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! How it reaches you, from the title and the line (RZ_ESaySource).
	static int SourceFor(string speaker, string text)
	{
		string s = speaker;
		s.ToLower();
		string t = text;
		t.ToLower();
		if (!ChannelOf(speaker).IsEmpty() || s == "a radio" || s == "the radio")
			return RZ_ESaySource.RADIO;

		if (s.Contains("loudspeaker") || s.Contains("loudhailer") || t.IndexOf("a loudhailer") == 0 || t.IndexOf("a speaker on a post") == 0)
			return RZ_ESaySource.LOUDSPEAKER;

		if (t.IndexOf("a sign ") == 0 || t.IndexOf("a sign,") == 0 || t.IndexOf("a new sign") == 0 || t.IndexOf("the board ") == 0 || t.IndexOf("the roster board") == 0 || t.IndexOf("under the toll sign") == 0 || t.IndexOf("a note") == 0)
			return RZ_ESaySource.NOTE;

		return RZ_ESaySource.IN_PERSON;
	}

	//------------------------------------------------------------------------------------------------
	//! The channel a radio voice's title names ("Radio: Bastion net" is "Bastion net"; also
	//! "Cracked: ...", "Wind-up radio: ...", "Radio chatter", "The open channel"), else "": a plain
	//! "Radio", "Radio station" or anything else is not a channel's voice.
	static string ChannelOf(string title)
	{
		string t = title;
		t.ToLower();
		if (t == "radio chatter")
			return "Open channel";

		if (t == "the open channel")
			return "The open channel";

		int colon = title.IndexOf(": ");
		if (colon <= 0)
			return "";

		string before = t.Substring(0, colon);
		if (before != "radio" && before != "cracked" && before != "wind-up radio")
			return "";

		string channel = title.Substring(colon + 2, title.Length() - colon - 2).Trim();
		if (channel.IsEmpty())
			return "";

		string first = channel.Substring(0, 1);
		first.ToUpper();
		return first + channel.Substring(1, channel.Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: where an in-person speaker stands, from their body (its place and its RplId), else `at`.
	void Place(IEntity who, vector at)
	{
		if (who)
		{
			m_vAt = who.GetOrigin();
			RplComponent rpl = RplComponent.Cast(who.FindComponent(RplComponent));
			if (rpl)
				m_Who = rpl.Id();

			return;
		}

		m_vAt = at;
	}

	//------------------------------------------------------------------------------------------------
	//! What the wire carries besides the numbers: "name|role|channel".
	string Head()
	{
		return RZ_UIText.Clean(m_sName, 80) + RZ_UIText.SEP + RZ_UIText.Clean(m_sRole, 80) + RZ_UIText.SEP + RZ_UIText.Clean(m_sChannel, 80);
	}

	//------------------------------------------------------------------------------------------------
	//! Client: the line as it came (RZ_PlayerUIComponent.RpcDo_SayLine).
	static RZ_SayLine FromWire(string head, int faction, int source, vector at, RplId who, string text)
	{
		array<string> f = {};
		RZ_UIText.Split(head, RZ_UIText.SEP, f);
		RZ_SayLine line = new RZ_SayLine();
		line.m_sName = RZ_UIText.Str(f, 0);
		line.m_sRole = RZ_UIText.Str(f, 1);
		line.m_sChannel = RZ_UIText.Str(f, 2);
		line.m_sSpeaker = line.m_sName;
		if (!line.m_sRole.IsEmpty())
			line.m_sSpeaker = line.m_sName + ", " + line.m_sRole;
		if (line.m_sSpeaker.IsEmpty())
			line.m_sSpeaker = line.m_sChannel;

		line.m_iFaction = faction;
		line.m_iSource = source;
		line.m_vAt = at;
		line.m_Who = who;
		line.m_sText = text;
		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! One part of a long line: everything but the text is the same.
	RZ_SayLine Part(string text)
	{
		RZ_SayLine p = new RZ_SayLine();
		p.m_sSpeaker = m_sSpeaker;
		p.m_sName = m_sName;
		p.m_sRole = m_sRole;
		p.m_sChannel = m_sChannel;
		p.m_iFaction = m_iFaction;
		p.m_iSource = m_iSource;
		p.m_bSecure = m_bSecure;
		p.m_vAt = m_vAt;
		p.m_Who = m_Who;
		p.m_sText = text;
		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! Who it is, for a newer line replacing one of theirs still waiting.
	string Key()
	{
		return m_sName + "|" + m_sRole + "|" + m_sChannel;
	}

	//------------------------------------------------------------------------------------------------
	//! The source icon (UI/Icons): speech bubble, radio, loudspeaker, paper.
	string SourceIcon()
	{
		switch (m_iSource)
		{
			case RZ_ESaySource.RADIO:
			{
				if (m_bSecure)
					return "status_secure_radio";

				return "status_radio_open";
			}
			case RZ_ESaySource.LOUDSPEAKER: return "status_broadcast";
			case RZ_ESaySource.NOTE: return "voice_note";
		}

		return "standing_word";
	}

	//------------------------------------------------------------------------------------------------
	//! The name tag's colour: the faction's, grey for people with none.
	Color NameColor()
	{
		if (RZ_Factions.IsValid(m_iFaction))
			return RZ_UIColor.Faction(m_iFaction);

		return RZ_UIColor.Dim();
	}

	//------------------------------------------------------------------------------------------------
	//! Client: where the speaker is now: their body while it's streamed in, else where they stood.
	//! False: nowhere to point (not in person, or the server had no place).
	bool Where(out vector at)
	{
		if (m_iSource != RZ_ESaySource.IN_PERSON)
			return false;

		if (!m_Body && m_Who.IsValid())
		{
			RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_Who));
			if (rpl)
				m_Body = rpl.GetEntity();
		}

		if (m_Body)
		{
			at = m_Body.GetOrigin();
			return true;
		}

		at = m_vAt;
		return m_vAt != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! "12 m", "1.4 km".
	static string Distance(float metres)
	{
		int m = RZ_Util.RoundToInt(metres);
		if (m < 1000)
			return string.Format("%1 m", m);

		int tenths = RZ_Util.RoundToInt(metres / 100);
		return string.Format("%1.%2 km", tenths / 10, tenths % 10);
	}
}
