//! The epilogue week's settings, saved form and names policy (docs/EPILOGUE.md). The controller is
//! Director/Epilogue/RZ_Epilogue.c.

//------------------------------------------------------------------------------------------------
//! Epilogue > Names: how players are named in the world's text.
enum RZ_EEpilogueNames
{
	FULL,		// the name they played under when they did it (frozen, filtered)
	TITLES,		// their title only ("the Haven's Champion")
	OFF			// nobody is named
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Director > Epilogue. Attribute defaults only.
[BaseContainerProps()]
class RZ_EpilogueSettings
{
	[Attribute("1", desc: "The epilogue week on (docs/EPILOGUE.md): after the cure the world tells the season for a week (Season > Epilogue hours): the record, beats at places, the dose outlets, the queue at the north road, leaders' season topics, the grey choices and the closing work. Off: the built stand-down only")]
	bool m_bEnabled;

	[Attribute("0", UIWidgets.ComboBox, "Names: Full (the name played under at the deed, filtered), Titles only, or Off", "", ParamEnumArray.FromEnum(RZ_EEpilogueNames))]
	RZ_EEpilogueNames m_eNames;

	[Attribute("12", UIWidgets.Slider, "Notables: at most this many players named in the world (half the identities with deeds on a small server)", "0 30 1")]
	int m_iNotables;

	[Attribute("8", UIWidgets.Slider, "Towns: at most this many towns carry an epilogue mark (beats and candles together)", "0 20 1")]
	int m_iTowns;

	[Attribute("20", UIWidgets.Slider, "Since-the-cure beats: at most this many new beats in the week", "0 40 1")]
	int m_iSinceBeats;

	[Attribute("1", desc: "The reel: the radio station plays one player broadcast from before the cure every 20 minutes in the week")]
	bool m_bReel;

	[Attribute("1", desc: "The world settles: in the epilogue no towns are taken or dropped, no outpost changes hands and the outpost contracts aren't offered (HoldingsFrozen)")]
	bool m_bWorldSettles;

	[Attribute("1", desc: "The closing work: Loose Ends, Bring Them Home, Take Back a Town, the Long Table's last week, the Walk Out, Settle Accounts, the Last Broadcast, Personal Goals and the Last Entry. Off below 24 epilogue hours either way")]
	bool m_bClosingWork;

	[Attribute("1", desc: "The Walk Out: in processing (the last 24 hours) an unarmed character can walk out at the north road barrier for good")]
	bool m_bWalkOut;

	[Attribute("1", desc: "Empty server ends early: nobody online for 2/7 of the epilogue in its middle phase starts processing at the next login")]
	bool m_bEmptyEndsEarly;

	[Attribute("0", desc: "Archive last entries: an admin may pick up to 5 Names board lines for the hall of fame (/epilogue lastnotes)")]
	bool m_bArchiveLastEntries;

	[Attribute("6", UIWidgets.Slider, "People per scene (the queue, the lab gate), inside the one body budget", "1 6 1")]
	int m_iScenePeople;

	[Attribute("1", desc: "Test scale: multiplies every real-time clock of the epilogue and its length (0.008 runs a 168-hour week in about 81 minutes; 1 for play)")]
	float m_fTestScale;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a free-standing board for the epilogue's marks (stand board, Names board, price board, notices). Empty: the marks are text (approach lines)", "et")]
	ResourceName m_sBoardPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a candle in a jar (a town taken back, the safehouses' candles). Empty: text", "et")]
	ResourceName m_sCandlePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: the table, crate or board players use at an epilogue place (needs the generic puzzle object's components and actions). Empty: Puzzles > Object", "et")]
	ResourceName m_sTablePrefab;

	[Attribute("8", UIWidgets.Slider, "Marks per cluster: at most this many epilogue marks stand as props around one group of players (150 m)", "1 20 1")]
	int m_iMarksPerCluster;

	[Attribute("2", UIWidgets.Slider, "Scenes with bodies that last (the queue, the lab gate): at most this many at once; the rest are boards", "0 4 1")]
	int m_iScenesLasting;

	[Attribute("1", UIWidgets.Slider, "Evening scenes with bodies (the supper reading): at most this many at once", "0 2 1")]
	int m_iScenesEvening;

	// The marks' own props (each optional; an empty slot uses the board above, else text)
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a sheet between two posts (a rescue town's mark)", "et")]
	ResourceName m_sSheetPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a stencilled board (Bastion's marks: a shelled town, a roster column)", "et")]
	ResourceName m_sStencilPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a price board (Rudi's crossroads)", "et")]
	ResourceName m_sPriceBoardPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a notice on a post (a lost post's last notice, the house rule's notice, the transformer's notice)", "et")]
	ResourceName m_sNoticePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a grave marker (graves, a leader's grave)", "et")]
	ResourceName m_sGravePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a wanted poster (the wanted at holdings)", "et")]
	ResourceName m_sPosterPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a trestle table (the queue's table, the case on the table)", "et")]
	ResourceName m_sTrestlePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a crate (the open crate, a post's stores)", "et")]
	ResourceName m_sCratePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: bunting (a house that grew)", "et")]
	ResourceName m_sBuntingPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a ribbon on a post (a name brought home)", "et")]
	ResourceName m_sRibbonPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a truck wreck on the verge (the roads' last lost trucks)", "et")]
	ResourceName m_sWreckPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: tape between two posts (a Quarantine Line town)", "et")]
	ResourceName m_sTapePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: a handcart (the farm's basket, the queue's bags)", "et")]
	ResourceName m_sHandcartPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional: the Names board at the bend on the north road", "et")]
	ResourceName m_sNamesBoardPrefab;
}

//------------------------------------------------------------------------------------------------
//! One beat: one fact about the season, told by the world at its place (docs/EPILOGUE.md, Beats).
class RZ_EpiBeat
{
	string key;			// "lab", "queue", "house.3", "town.7", "pack.12", "post.5", "since.town.7"
	string slot;		// cure, faction, hub, place, people, side, since
	string place;		// the place's name, for echoes ("the lab gate", "Levie")
	float x;
	float z;
	ref array<string> texts = {};	// the four versions: phase A, a third, two thirds, processing ("" = the one before)
	string since;		// the since version ("" = dropped when its subject changes)
	string echo;		// the echo: the place and one detail
	string subj;		// what it depends on: leader, wanderer, owner, house, crawlers, table, pack, wanted, ""
	int subjId = -1;
	int subjVal;
	bool sinceOn;
	bool dropped;
	float weight;
	int faction = -1;
	string extra;		// a fixed detail (a grave's "MECHANIC. DAY 23")
	ref array<string> idents = {};	// players it names (frozen names below)
	ref array<string> names = {};
	string more;		// the places in full: a detail added to the text (RZ_EpiRecord.Places)
	bool changed;	// the place ended up different (the change factor)
}

//------------------------------------------------------------------------------------------------
//! One piece of closing work that can close on its own (docs/EPILOGUE.md, When something closes).
class RZ_EpiItem
{
	string key;			// "loose.door", "broadcast", "table", "coda.book", "ridges"
	string title;
	string giver;		// whose voice the last call comes in
	string lastCall;	// the last call's line
	string place;
	float x;
	float z;
	float need;			// real hours of play it takes (x the test scale)
	int state;			// 0 open, 1 last call given, 2 closed live, 3 closed as data
	float grace;		// seconds of play since the last call
	int touchUnix;		// the last time someone took part
	string endLine;
	int faction = -1;
}

//------------------------------------------------------------------------------------------------
//! A named dead person whose keepsake can be brought home (Bring Them Home).
class RZ_EpiName
{
	string name;		// "Pavel Novak"
	string what;		// "his tags", "her ID card", "the notebook"
	int faction = -1;	// whose dead: home is that faction's (-1: the Haven's houses)
	float x;			// where the keepsake lies
	float z;
	string lies;		// "on the Morton road", "at the quarry"
	int state;			// 0 lies there, 1 carried, 2 home, 3 not brought home
	string carrier;		// identity carrying it
	string home;		// the place it went home to
	string by;			// who brought it (frozen name, "" unnamed)
	string killedBy;	// identity of whoever killed them, if a player (refused)
	string ident;		// a player's character: their identity (their own or their group's: no pay)
	string role;		// "Mechanic", "the trader"
	int day;			// the season day they died
}

//------------------------------------------------------------------------------------------------
//! A line on the Names board (the Last Entry).
class RZ_EpiEntry
{
	string ident;
	string name;
	string text;
	int unix;
}

//------------------------------------------------------------------------------------------------
class RZ_EpilogueSave
{
	bool built;
	bool lastDay;			// built for the Unsigned last day (no cure)
	int cureUnix;
	int commitUnix;
	int aEndUnix;			// the end of phase A (0: still in it)
	bool cForced;			// processing started early (an admin, or an empty server)
	int cStartUnix;			// when processing began (0: not yet)
	int lastOnlineUnix;
	bool emptyPending;
	int step;				// the evolving step: 0, 1 (a third), 2 (two thirds), 3 (processing)
	bool frozen;			// the hall of fame froze

	// The record (docs/EPILOGUE.md, The season's story)
	int route;				// finale.route (0 nobody, 1 Haven ... 5 Independent)
	int how;
	int terms;
	int condition;
	int faction = -1;		// whose rules: the controller's, or the season's Fame leader
	int cureDay;
	bool slow;
	int fameLeader = -1;
	int fameLeaderFame;
	ref array<string> deciders = {};		// names, frozen
	ref array<string> deciderIdents = {};	// and their identities (the notables' score)
	ref array<string> standNames = {};
	ref array<string> majorLines = {};
	ref array<int> housePop = {};
	ref array<int> houseTier = {};
	ref array<int> holdIds = {};
	ref array<int> holdOwner = {};
	ref array<int> townIdx = {};
	ref array<int> townCrawlers = {};
	ref array<int> townTaken = {};			// 0 not yet, else the season day it was taken back
	ref array<string> townBy = {};
	ref array<int> packIds = {};
	ref array<string> packPlaces = {};
	ref array<string> playerIdents = {};
	ref array<string> playerNames = {};
	ref array<string> playerLines = {};	// up to three lines, "\n" between
	ref array<string> told = {};			// identities who had their first line
	ref array<string> wantIdents = {};
	ref array<string> wantNames = {};
	ref array<int> wantFaction = {};
	ref array<int> wantState = {};			// 0 wanted, 1 settled, 2 paid, 3 not paid, 4 walked out

	ref array<ref RZ_EpiBeat> beats = {};
	ref array<ref RZ_EpiItem> items = {};
	ref array<ref RZ_EpiName> names = {};
	ref array<ref RZ_EpiEntry> entries = {};
	ref array<string> met = {};				// "ident|beat key": beats each player has met
	ref array<string> walkDown = {};		// "ident|name": names put down for the walk
	ref array<string> walked = {};			// "ident|name|day"
	ref array<string> struck = {};			// names and beats an admin struck

	// The dose outlets
	ref array<string> outletKeys = {};
	ref array<int> outletStock = {};
	ref array<int> outletUnix = {};
	ref array<string> dosed = {};			// "ident|life": one outlet dose per life
	ref array<string> crate = {};			// identities who had the open crate's one
	ref array<string> owed = {};			// identities with a line in the book from a dose
	ref array<string> bookPaid = {};

	// The four grey choices: 0 open, 1 the first answer, 2 the other
	ref array<int> choices = {0, 0, 0, 0};
	// The Last Broadcast: -1 open, else the version (0 names, 1 the three weeks, 2 the truth, 3 the plain line)
	int broadcast = -1;
	ref array<string> sayIdent = {};
	ref array<int> sayVersion = {};
	int airedHour = -1;

	// Personal goals and deeds
	ref array<string> goals = {};			// "ident|background" finished in the week
	ref array<string> deedKeys = {};		// identity
	ref array<int> deedCounts = {};
	ref array<string> payKeys = {};			// "ident|real day": paid deliveries
	ref array<int> payCounts = {};
	ref array<string> square = {};			// identities square with all four at processing
	ref array<string> hallLines = {};		// the week's lines for the season block
	int ridgesDay = -1;
	string ridgesLast;
	int tableState;							// 0 nothing, 1 packing, 2 gone to the queue, 3 found out
	int sinceCount;
	ref array<string> reelIdents = {};
	ref array<string> reelLines = {};
	ref array<int> reelDays = {};

	// The evening calendar (RZ_EpilogueCalendar.c)
	int supperDay = -1;						// the game day of the last supper reading
	int watchDay = -1;						// the game day of the last 20:30 watch
	int watchStrokes;						// strokes on the roster board
	ref array<string> codaKeys = {};		// codas seen run (major keys) and when
	ref array<int> codaUnix = {};
	ref array<string> aired = {};			// second airings done
	int airedDay = -1;

	// The rest of the week (RZ_EpilogueRest.c); null in a save from before it
	ref RZ_EpiRestSave rest;
	ref RZ_EpiRecordSave record;
}

//------------------------------------------------------------------------------------------------
//! The names policy (docs/EPILOGUE.md, Notables, and keeping names safe): frozen at the deed,
//! filtered, opt-out. Server only.
class RZ_EpiNames
{
	//------------------------------------------------------------------------------------------------
	//! The name to write now for a player doing something (frozen by the caller), "" if they're
	//! anonymous or names are off.
	static string Now(int playerId)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || playerId <= 0)
			return "";

		string ident = gm.GetProfile(playerId).m_sIdentity;
		return Show(ident, RZ_Util.PlayerName(playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! A frozen name as the world's text shows it: "a survivor" for the anonymous, the title (or "a
	//! stranger") when the filter would change it or an admin struck it, nothing with names off.
	static string Show(string ident, string frozen)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return frozen;

		RZ_HallOfFame hall = gm.GetHall();
		if (hall && !ident.IsEmpty() && hall.IsAnonymous(ident))
			return "a survivor";

		RZ_Epilogue epi = RZ_Epilogue.Get();
		RZ_EEpilogueNames mode = RZ_EEpilogueNames.FULL;
		if (epi)
			mode = epi.GetSettings().m_eNames;

		if (mode == RZ_EEpilogueNames.OFF)
			return "someone";

		if (frozen.IsEmpty())
			return "someone";

		string title = TitleOf(ident);
		if (mode == RZ_EEpilogueNames.TITLES || (epi && epi.IsStruck(frozen)))
			return title;

		if (Filtered(frozen) != frozen)
			return title;

		return frozen;
	}

	//------------------------------------------------------------------------------------------------
	//! The note filter (Haven > Note filter) over a text.
	static string Filtered(string text)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetSafehouses() || !gm.GetSafehouses().GetBoard())
			return text;

		return gm.GetSafehouses().GetBoard().FilterText(text);
	}

	//------------------------------------------------------------------------------------------------
	//! The title people use for an identity: the tier of their best standing now ("the Haven's
	//! Champion"), else "a stranger". Simplified: the current life's, not the season's best life.
	static string TitleOf(string ident)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || ident.IsEmpty())
			return "a stranger";

		RZ_PlayerProfile p = gm.FindProfileByIdentity(ident);
		if (!p)
			return "a stranger";

		int best = -1;
		int bestFame = 0;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (f < p.m_aFame.Count() && p.m_aFame[f] > bestFame)
			{
				bestFame = p.m_aFame[f];
				best = f;
			}
		}

		if (best < 0)
			return "a stranger";

		RZ_ETier tier = p.GetTier(best);
		if (!RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN))
			return "a stranger";

		return string.Format("%1's %2", RZ_Factions.GetNameInText(best), RZ_Reputation.GetTierName(tier));
	}
}
