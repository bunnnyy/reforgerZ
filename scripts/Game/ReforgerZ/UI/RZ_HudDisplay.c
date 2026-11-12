//! The ReforgerZ HUD (docs/UI.md, "HUD"): the infection timer with its stages and the golden
//! window, the secure-radio indicator, a subtle noise meter with how visible you are above it
//! (Stealth on), the quest tracker top left (the tracked job and the others you've taken), the
//! styled notice cards, and the nametags over people near you to talk to (UI/RZ_NametagClient.c).
//!
//! Spoken lines (RZ_SayLine): in person, loudspeakers and notes as subtitles at the bottom centre,
//! each with how it reaches you, the name in their colour, their role and, in person, an arrow and
//! the distance to them; radio channels in their own box at the top left, under the tracker.
//!
//! An SCR_InfoDisplay: add it to the HUD manager's info displays on the player controller and
//! give it the layout from UI/LAYOUTS.md ("HUD"). It reads only RZ_PlayerUIComponent's client
//! cache, redraws five times a second, and changes nothing in the game.
//!
//! Notices: RZ_GameModeComponent.QueueHint calls PushNotice first. It returns false when styled
//! notices are off, the layout has no NoticeStack, or no card layout is set; the vanilla hint is
//! used then, exactly as before.

//------------------------------------------------------------------------------------------------
//! One place spoken lines show (the subtitles, the radio-line box): one line at a time, each for
//! its reading time, a long one cut into parts shown in turn.
class RZ_LineLane
{
	ref array<ref RZ_SayLine> m_aWaiting = {};
	ref RZ_SayLine m_Shown;
	float m_fUntil = -1;		// the line showing stays until then; -1 none showing

	//------------------------------------------------------------------------------------------------
	//! A line's parts (at most `chars` each). replace: a newer line from the same speaker replaces
	//! theirs still waiting. At most `cap` lines' parts wait besides this one's.
	void Add(RZ_SayLine line, int chars, int cap, bool replace)
	{
		if (replace)
		{
			string key = line.Key();
			for (int i = m_aWaiting.Count() - 1; i >= 0; i--)
			{
				if (m_aWaiting[i].Key() == key)
					m_aWaiting.RemoveOrdered(i);
			}
		}

		array<string> parts = {};
		RZ_UIText.SplitLine(line.m_sText, chars, parts);
		foreach (string part : parts)
		{
			m_aWaiting.Insert(line.Part(part));
		}

		while (m_aWaiting.Count() > cap + parts.Count() - 1)
		{
			m_aWaiting.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! True when the line showing is done (its time is up, or `held`: the reader is open).
	bool Expire(float now, bool held)
	{
		if (m_fUntil < 0 || (now < m_fUntil && !held))
			return false;

		m_fUntil = -1;
		m_Shown = null;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The next line to show from now for shortest to longest seconds by length, or null.
	RZ_SayLine Next(float now, bool held, RZ_UISettings settings)
	{
		if (m_fUntil >= 0 || held || m_aWaiting.IsEmpty())
			return null;

		m_Shown = m_aWaiting[0];
		m_aWaiting.RemoveOrdered(0);
		float seconds = m_Shown.m_sText.Length() / Math.Max(1, settings.m_fSubtitleCps);
		m_fUntil = now + Math.Clamp(seconds, settings.m_fSubtitleMin, Math.Max(settings.m_fSubtitleMin, settings.m_fSubtitleMax));
		return m_Shown;
	}

	//------------------------------------------------------------------------------------------------
	//! The line on screen now, or null.
	RZ_SayLine Showing()
	{
		if (m_fUntil < 0)
			return null;

		return m_Shown;
	}
}

//------------------------------------------------------------------------------------------------
//! A notice waiting for a card.
class RZ_QueuedNotice
{
	string m_sTitle;
	string m_sText;
	ref RZ_NoticeStyle m_Style;
	float m_fAt;
}

//------------------------------------------------------------------------------------------------
class RZ_HudDisplay : SCR_InfoDisplayExtended
{
	protected static const float REFRESH = 0.2;

	protected static RZ_HudDisplay s_Instance;

	protected float m_fRefresh;
	protected ref array<ref RZ_QueuedNotice> m_aQueue = {};
	protected ref array<Widget> m_aCards = {};
	protected ref array<float> m_aCardStart = {};
	protected ref array<int> m_aCardPriority = {};
	protected float m_fLastCardAt = -100;
	protected int m_iToNotebook;		// notices that went to the notebook without a card
	protected float m_fLifeSeen = -1;
	protected ref RZ_LineLane m_Subs = new RZ_LineLane();		// bottom centre: in person, loudspeakers, notes
	protected ref RZ_LineLane m_Radio = new RZ_LineLane();	// top left: radio channels
	protected string m_sSubDist;		// the distance last written, so it's set only when it changes
	protected static const int SUB_WAITING = 4;
	protected static const float TRACK_X = 24;		// the tracker's place, tucked into the top-left corner
	protected static const float TRACK_Y = 24;
	protected static const float TRACK_Y_RADIO = 126;	// below the secure-radio panel while it shows
	protected int m_iTrackerDrawn = -1;
	protected static const float TRACK_NEAR = 995;	// under a km (995 m rounds to 1,000): the arrow and metres
	protected bool m_bTrackShown;		// the tracker shows a job
	protected bool m_bTrackAt;			// it points somewhere (x, z sent)
	protected vector m_vTrackAt;		// where: the spot, or the search area's centre
	protected float m_fTrackArrive;		// the spot is reached within this (m)
	protected float m_fTrackSearch;		// the search area's radius (m), 0 for a spot
	protected string m_sTrackWords;		// the server's words ("Camurac, a few km north-west")
	protected string m_sTrackPlace;		// the place alone ("Camurac")
	protected string m_sTrackLine = "-";	// the area line last written, so it's set only when it changes
	protected int m_iTrackArrow = -1;	// the arrow last shown (1) or hidden (0)
	protected static const int SUB_CHARS = 130;	// about two lines at 32 px in 1,044 units
	protected static const int RADIO_WAITING = 6;
	protected static const int RADIO_CHARS = 130;	// about three lines at 26 px in 528 units

	//------------------------------------------------------------------------------------------------
	override protected void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		s_Instance = this;
		GetGame().GetInputManager().AddActionListener("RZ_OpenNotebook", EActionTrigger.DOWN, OpenNotebook);
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayStopDraw(IEntity owner)
	{
		GetGame().GetInputManager().RemoveActionListener("RZ_OpenNotebook", EActionTrigger.DOWN, OpenNotebook);
		foreach (Widget w : m_aCards)
		{
			RZ_UIW.Remove(w);
		}

		m_aCards.Clear();
		m_aCardStart.Clear();
		m_aCardPriority.Clear();
		if (s_Instance == this)
			s_Instance = null;

		super.DisplayStopDraw(owner);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		// Door marks as decals near you (docs/TRACES.md, Doors you can see)
		RZ_DoorMarkClient.Tick(timeSlice);
		RZ_TraceLightsClient.Tick(timeSlice);
		// Tags over the people near you to talk to (docs/UI.md, Nametags): who, four times a second; where, every frame
		RZ_NametagClient.Tick(timeSlice, m_wRoot);
		// The arrow to whoever is speaking turns with you, every frame (one rotation, the text on change)
		DrawSubtitleDir();
		// So does the tracker's arrow to the tracked objective, within a km
		DrawTrackerDir();
		m_fRefresh -= timeSlice;
		if (m_fRefresh > 0)
			return;

		m_fRefresh = REFRESH;
		RZ_NotebookItem.CheckHeld();
		Refresh();
	}

	// ================================================================ Input
	//------------------------------------------------------------------------------------------------
	//! N: the notebook item comes into the hand and the notebook opens, on the tab last used
	//! (docs/UI.md, "The notebook"); without the item, it just opens.
	protected void OpenNotebook(float value = 0, EActionTrigger reason = 0)
	{
		RZ_NotebookItem.OpenByKey();
	}

	// ================================================================ Drawing
	//------------------------------------------------------------------------------------------------
	protected void Refresh()
	{
		if (!m_wRoot)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		RZ_UISettings settings = RZ_UISettings.Get();
		if (ui)
		{
			DrawInfection(ui, settings);
			DrawRadio(ui);
			DrawNoise(ui, settings);
			DrawStand(ui);
			DrawLook(ui);
			DrawTracker(ui);
		}
		else
		{
			RZ_UIW.Show(m_wRoot, "TrackerPanel", false);
			m_bTrackShown = false;
			m_iTrackerDrawn = -1;
			RZ_UIW.Show(m_wRoot, "StandPanel", false);
			RZ_UIW.Show(m_wRoot, "LookPanel", false);
			RZ_UIW.Show(m_wRoot, "InfectionPanel", false);
			RZ_UIW.Show(m_wRoot, "RadioPanel", false);
			RZ_UIW.Show(m_wRoot, "NoisePanel", false);
			RZ_UIW.Show(m_wRoot, "SeenPanel", false);
		}

		DrawNotices(settings);
		DrawSubtitle(settings);
		DrawRadioLine(settings);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawInfection(RZ_PlayerUIComponent ui, RZ_UISettings settings)
	{
		int state = ui.GetInfectionState();
		bool show = settings.m_bInfectionTimer && state != RZ_EInfectionUI.NONE;
		RZ_UIW.Show(m_wRoot, "InfectionPanel", show);
		if (!show)
			return;

		float left = ui.GetInfectionLeft();
		float total = Math.Max(1, ui.GetInfectionTotal());
		Color c = RZ_UIColor.Stage(state);
		RZ_UIW.Tint(m_wRoot, "InfectionAccent", c);
		RZ_UIW.Tint(m_wRoot, "InfectionIcon", c);
		RZ_UIW.Tint(m_wRoot, "InfectionStage", c);
		RZ_UIW.SetText(m_wRoot, "InfectionClock", RZ_UIText.Clock(left));

		bool golden = state == RZ_EInfectionUI.GOLDEN;
		RZ_UIW.Show(m_wRoot, "GoldenRow", golden);
		RZ_UIW.Show(m_wRoot, "StageTrack", !golden);
		if (golden)
		{
			RZ_UIW.Icon(m_wRoot, "InfectionIcon", "status_golden");
			RZ_UIW.SetText(m_wRoot, "InfectionStage", "BITTEN - CLEAN IT NOW");
			RZ_UIW.SetText(m_wRoot, "InfectionNote", "Alcohol or antiseptic might do it. Disinfectant and a bandage are better.");
			RZ_UIW.Progress(m_wRoot, "GoldenBar", left / total);
			RZ_UIW.Show(m_wRoot, "PausedRow", false);
			RZ_UIW.Show(m_wRoot, "FinalGlow", false);
			return;
		}

		// The three stages as segments: Fever is the first half, Final the last few minutes
		float elapsed = total - left;
		float half = total * 0.5;
		float finalStart = total - ui.GetInfectionFinal();
		RZ_UIW.Progress(m_wRoot, "TrackFever", elapsed / Math.Max(1, half));
		RZ_UIW.Progress(m_wRoot, "TrackWeakness", (elapsed - half) / Math.Max(1, finalStart - half));
		RZ_UIW.Progress(m_wRoot, "TrackFinal", (elapsed - finalStart) / Math.Max(1, total - finalStart));

		string stage = "INFECTED - FEVER";
		string icon = "status_fever";
		string note = "";
		if (state == RZ_EInfectionUI.WEAKNESS)
		{
			stage = "INFECTED - WEAKNESS";
			icon = "status_weakness";
			note = "Anyone who looks at you can see it now.";
		}
		else if (state == RZ_EInfectionUI.FINAL)
		{
			stage = "THE END OF IT";
			icon = "status_final";
			note = "If there's an antidote anywhere near, this is the time.";
		}

		RZ_UIW.Icon(m_wRoot, "InfectionIcon", icon);
		RZ_UIW.SetText(m_wRoot, "InfectionStage", stage);
		RZ_UIW.SetText(m_wRoot, "InfectionNote", note);
		RZ_UIW.Show(m_wRoot, "FinalGlow", state == RZ_EInfectionUI.FINAL);

		float pause = ui.GetPauseLeft();
		RZ_UIW.Show(m_wRoot, "PausedRow", pause > 0);
		if (pause > 0)
			RZ_UIW.SetText(m_wRoot, "PausedText", "PAUSED - " + RZ_UIText.Clock(pause));
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawRadio(RZ_PlayerUIComponent ui)
	{
		int f = ui.GetListenFaction();
		RZ_UIW.Show(m_wRoot, "RadioPanel", f >= 0);
		if (f < 0)
			return;

		Color c = RZ_UIColor.Faction(f);
		RZ_UIW.Tint(m_wRoot, "RadioAccent", c);
		RZ_UIW.Tint(m_wRoot, "RadioIcon", c);
		RZ_UIW.Tint(m_wRoot, "RadioChannel", c);
		string channel = RZ_SecureRadio.ChannelName(f);
		channel.ToUpper();
		RZ_UIW.SetText(m_wRoot, "RadioChannel", channel);
		int minutes = Math.Max(1, RZ_Util.RoundToInt(ui.GetListenLeft() / 60));
		RZ_UIW.SetText(m_wRoot, "RadioSub", string.Format("Cracked - codes change in %1 min", minutes));
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawNoise(RZ_PlayerUIComponent ui, RZ_UISettings settings)
	{
		RZ_UIW.Show(m_wRoot, "NoisePanel", settings.m_bNoiseMeter);
		if (!settings.m_bNoiseMeter)
		{
			RZ_UIW.Show(m_wRoot, "SeenPanel", false);
			return;
		}

		int level = ui.GetNoise();
		for (int i = 1; i <= 6; i++)
		{
			Color c = RZ_UIColor.Bone();
			if (i > level)
				c = RZ_UIW.Rgb(70, 68, 62);
			else if (i >= 5)
				c = RZ_UIColor.Stage(RZ_EInfectionUI.WEAKNESS);

			RZ_UIW.Tint(m_wRoot, "Noise" + i.ToString(), c);
		}

		string word = "QUIET";
		if (level >= 5)
			word = "VERY LOUD";
		else if (level >= 4)
			word = "LOUD";
		else if (level >= 2)
			word = "HEARD";

		RZ_UIW.SetText(m_wRoot, "NoiseText", word);
		DrawSeen(ui);
	}

	//------------------------------------------------------------------------------------------------
	//! The stand's line under the noise meter (docs/FINALE.md, "The stand"): routes, never players.
	//! Hidden unless the server sent one in the last few seconds; a layout without StandPanel shows nothing.
	protected void DrawStand(RZ_PlayerUIComponent ui)
	{
		string line = ui.GetStandLine();
		RZ_UIW.Show(m_wRoot, "StandPanel", !line.IsEmpty());
		if (!line.IsEmpty())
			RZ_UIW.SetText(m_wRoot, "StandText", line);
	}

	//------------------------------------------------------------------------------------------------
	//! Borrowed kit's look coming, RZ_Hud_Look (docs/UI.md, the HUD): the Watched eye and "PASS AS
	//! BASTION: 2 OF 2" above the stand's line, while a look is coming. A layout without LookPanel
	//! shows nothing (the notices still carry it).
	protected void DrawLook(RZ_PlayerUIComponent ui)
	{
		string line = ui.GetLookLine();
		RZ_UIW.Show(m_wRoot, "LookPanel", !line.IsEmpty());
		if (line.IsEmpty())
			return;

		RZ_UIW.Icon(m_wRoot, "LookIcon", "tier_watched");
		RZ_UIW.SetText(m_wRoot, "LookText", line);
	}

	//------------------------------------------------------------------------------------------------
	//! The quest tracker (docs/UI.md, "The quest tracker"), top left, small, owner-only: the tracked
	//! job alone, its type icon and name, its current objective (a filled diamond), the known next
	//! one (an open diamond, dim) and the area line in the accent colour (DrawTrackerDir: words, or
	//! within a km an arrow and metres). The other jobs taken are the Quests tab's, never here.
	//! Hidden while nothing is tracked. Redrawn only when the server sends a new copy; a layout
	//! without TrackerPanel shows nothing.
	protected void DrawTracker(RZ_PlayerUIComponent ui)
	{
		Widget panel = RZ_UIW.Find(m_wRoot, "TrackerPanel");
		if (!panel)
			return;

		// Below the secure-radio panel while it shows, so they never overlap
		float y = TRACK_Y;
		if (ui.GetListenFaction() >= 0)
			y = TRACK_Y_RADIO;

		RZ_UIW.SetPos(panel, TRACK_X, y);

		int version = ui.GetTrackerVersion();
		if (version == m_iTrackerDrawn)
			return;

		m_iTrackerDrawn = version;
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(ui.GetTracker(), recs);
		array<string> head = null;
		string now = "";
		string next = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "T")
				head = f;
			else if (tag == "O" && RZ_UIText.Int(f, 1) == RZ_EObjState.CURRENT && now.IsEmpty())
				now = RZ_UIText.Str(f, 2);
			else if (tag == "O" && RZ_UIText.Int(f, 1) == RZ_EObjState.NEXT && next.IsEmpty())
				next = RZ_UIText.Str(f, 2);
		}

		RZ_UIW.Show(m_wRoot, "TrackerPanel", head != null);
		m_bTrackShown = head != null;
		if (!head)
			return;

		int giver = RZ_UIText.Int(head, 3);
		Color accent = RZ_UIColor.Hazard();
		if (giver >= 0)
			accent = RZ_UIColor.Faction(giver);

		RZ_UIW.Icon(m_wRoot, "TrackerIcon", RZ_UIText.MissionIcon(RZ_UIText.Int(head, 2)));
		RZ_UIW.Tint(m_wRoot, "TrackerIcon", accent);
		RZ_UIW.SetText(m_wRoot, "TrackerTitle", RZ_UIText.Str(head, 4));
		RZ_UIW.SetText(m_wRoot, "TrackerNow", now);
		RZ_UIW.Show(m_wRoot, "TrackerNextRow", !next.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "TrackerNext", next);

		// The area on its own line, in the accent colour: where it points, and how to say it far off
		string area = RZ_UIText.Str(head, 6);
		m_sTrackWords = RZ_QuestText.Cap(area);
		m_sTrackPlace = RZ_UIText.Str(head, 7);
		m_bTrackAt = !RZ_UIText.Str(head, 8).IsEmpty() && !RZ_UIText.Str(head, 9).IsEmpty();
		m_vTrackAt = Vector(RZ_UIText.Float(head, 8), 0, RZ_UIText.Float(head, 9));
		m_fTrackArrive = RZ_UIText.Float(head, 10);
		m_fTrackSearch = RZ_UIText.Float(head, 11);
		bool areaShown = !area.IsEmpty() || m_bTrackAt;
		RZ_UIW.Show(m_wRoot, "TrackerAreaRow", areaShown);
		RZ_UIW.Show(m_wRoot, "TrackerArea", areaShown);
		m_sTrackLine = "-";
		m_iTrackArrow = -1;
		DrawTrackerDir();
	}

	//------------------------------------------------------------------------------------------------
	//! The tracker's area line, every frame (docs/UI.md, "The quest tracker"). Within a km of a known
	//! spot: an arrow turned to it from where you face and the metres ("Camurac, 120 m"); inside its
	//! arrival radius the arrow goes and the words come back ("Camurac, close by"). A search area:
	//! the arrow to its centre and the metres to its edge ("Search area, 240 m"), then "In the search
	//! area" once you're inside it. Farther off, dead, or a job with no place: the server's words
	//! alone. Client-side from the position in the tracker's record; one rotation a frame, the
	//! text and the arrow's visibility only when they change.
	protected void DrawTrackerDir()
	{
		if (!m_wRoot || !m_bTrackShown)
			return;

		string line = m_sTrackWords;
		bool arrow = false;
		IEntity me = RZ_UIW.LocalEntity();
		if (me && m_bTrackAt)
		{
			vector here = me.GetOrigin();
			float dist = RZ_Util.DistXZ(here, m_vTrackAt);
			if (m_fTrackSearch > 0)
			{
				float edge = dist - m_fTrackSearch;
				if (edge <= 0)
				{
					line = "In the search area";
				}
				else if (edge < TRACK_NEAR)
				{
					arrow = true;
					line = "Search area, " + RZ_QuestText.Metres(edge);
				}
			}
			else if (dist < TRACK_NEAR)
			{
				// Down to 0 m; the arrow goes in the last 2 m, where it would only spin
				arrow = dist >= 2;
				line = RZ_QuestText.Metres(dist);
				if (!m_sTrackPlace.IsEmpty())
					line = m_sTrackPlace + ", " + line;
			}

			if (arrow)
				RZ_UIW.Rotate(m_wRoot, "TrackerArrow", RZ_UIW.Bearing(here, m_vTrackAt) - RZ_UIW.Heading(me));
		}

		int shown = 0;
		if (arrow)
			shown = 1;

		if (shown != m_iTrackArrow)
		{
			m_iTrackArrow = shown;
			RZ_UIW.Show(m_wRoot, "TrackerArrow", arrow);
		}

		if (line != m_sTrackLine)
		{
			m_sTrackLine = line;
			RZ_UIW.SetText(m_wRoot, "TrackerArea", line);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! How visible you are (Stealth, docs/DESIGN.md), above the noise meter: five segments and a
	//! word. Not shown with Stealth off (the server never sends it).
	protected void DrawSeen(RZ_PlayerUIComponent ui)
	{
		int seen = ui.GetSeen();
		RZ_UIW.Show(m_wRoot, "SeenPanel", seen >= 0);
		if (seen < 0)
			return;

		bool hidden = ui.IsSeenHidden();
		for (int s = 1; s <= 5; s++)
		{
			Color lit = RZ_UIColor.Bone();
			if (s > seen)
				lit = RZ_UIW.Rgb(70, 68, 62);
			else if (hidden)
				lit = RZ_UIColor.Good();
			else if (s >= 5)
				lit = RZ_UIColor.Stage(RZ_EInfectionUI.WEAKNESS);

			RZ_UIW.Tint(m_wRoot, "Seen" + s.ToString(), lit);
		}

		string state = "SEEN";
		if (hidden)
			state = "HIDDEN";
		else if (seen >= 5)
			state = "IN PLAIN SIGHT";
		else if (seen <= 2)
			state = "LOW";

		RZ_UIW.SetText(m_wRoot, "SeenText", state);
	}

	// ================================================================ Notice cards
	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.QueueHint (client). False = use the vanilla hint instead.
	static bool PushNotice(string title, string text, RZ_NoticeStyle style)
	{
		if (!s_Instance || !s_Instance.CanShowNotices())
			return false;

		s_Instance.Enqueue(title, text, style);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanShowNotices()
	{
		RZ_UISettings settings = RZ_UISettings.Get();
		if (!settings.m_bStyledNotices || settings.m_sNoticeLayout.IsEmpty())
			return false;

		return RZ_UIW.Find(m_wRoot, "NoticeStack") != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A quiet notice went into the notebook without a card: the line under the cards counts it.
	static void NoteQuiet()
	{
		if (s_Instance)
			s_Instance.m_iToNotebook++;
	}

	//------------------------------------------------------------------------------------------------
	//! The notebook was opened: what went there unseen has been seen.
	static void ClearNotebookCount()
	{
		if (s_Instance)
			s_Instance.m_iToNotebook = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Waiting notices are kept most important first, then oldest first.
	protected void Enqueue(string title, string text, RZ_NoticeStyle style)
	{
		RZ_QueuedNotice q = new RZ_QueuedNotice();
		q.m_sTitle = title;
		q.m_sText = text;
		q.m_Style = style;
		q.m_fAt = RZ_UIW.Now();
		int idx = m_aQueue.Count();
		foreach (int i, RZ_QueuedNotice other : m_aQueue)
		{
			if (other.m_Style.m_iPriority < style.m_iPriority)
			{
				idx = i;
				break;
			}
		}

		m_aQueue.InsertAt(q, idx);

		// Past the cap the least important goes: one the Radio tab lists, if there is one
		int cap = Math.Max(1, RZ_UISettings.Get().m_iNoticeQueue);
		while (m_aQueue.Count() > cap)
		{
			int drop = m_aQueue.Count() - 1;
			for (int d = m_aQueue.Count() - 1; d >= 0; d--)
			{
				if (m_aQueue[d].m_Style.IsLogged() && m_aQueue[d].m_Style.m_iPriority < RZ_ENoticePriority.URGENT)
				{
					drop = d;
					break;
				}
			}

			if (m_aQueue[drop].m_Style.IsLogged())
				m_iToNotebook++;

			m_aQueue.RemoveOrdered(drop);
		}

		DrawNotices(RZ_UISettings.Get());
	}

	//------------------------------------------------------------------------------------------------
	//! A few at a time, by importance (docs/UI.md, "Notices"): at most Notice cards at once, a new
	//! one at most every Gap seconds (First minutes gap in a life's first minutes); urgent ones skip
	//! the gap and may cut the least important card short; one that isn't urgent and waits past Wait
	//! seconds is stale and goes to the notebook only ("+N in your notebook") or is dropped. The quiet
	//! start is RZ_GameModeComponent.QueueHint's: those notices never reach the queue.
	protected void DrawNotices(RZ_UISettings settings)
	{
		float now = RZ_UIW.Now();
		float life = Math.Max(2, settings.m_fNoticeSeconds);
		for (int i = m_aCards.Count() - 1; i >= 0; i--)
		{
			float share = (now - m_aCardStart[i]) / life;
			if (share >= 1)
			{
				RemoveCard(i);
				continue;
			}

			RZ_UIW.Progress(m_aCards[i], "CardLife", 1 - share);
		}

		// A new life: last life's count belongs to last life's notebook
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		float lifeAt = -1;
		if (ui)
			lifeAt = ui.GetNewLifeAt();

		if (lifeAt != m_fLifeSeen)
		{
			m_fLifeSeen = lifeAt;
			m_iToNotebook = 0;
		}

		// Waited too long: stale, so no card. A notice the notebook lists is there already; the
		// rest are dropped (docs/UI.md, "Notices")
		float wait = Math.Max(5, settings.m_fNoticeWait);
		for (int w = m_aQueue.Count() - 1; w >= 0; w--)
		{
			RZ_NoticeStyle st = m_aQueue[w].m_Style;
			if (st.m_iPriority < RZ_ENoticePriority.URGENT && now - m_aQueue[w].m_fAt > wait)
			{
				if (st.IsLogged())
					m_iToNotebook++;

				m_aQueue.RemoveOrdered(w);
			}
		}

		Widget stack = RZ_UIW.Find(m_wRoot, "NoticeStack");
		int maxCards = Math.Max(1, settings.m_iNoticeStack);

		// An urgent one doesn't wait behind a full stack: the least important card makes room
		if (!m_aQueue.IsEmpty() && m_aQueue[0].m_Style.m_iPriority == RZ_ENoticePriority.URGENT && m_aCards.Count() >= maxCards)
		{
			int lowest = -1;
			foreach (int c, int prio : m_aCardPriority)
			{
				if (prio < RZ_ENoticePriority.URGENT && (lowest < 0 || prio < m_aCardPriority[lowest]))
					lowest = c;
			}

			if (lowest >= 0)
				RemoveCard(lowest);
		}

		// The first minutes of a life: one card at most every First minutes gap (urgent ones don't wait)
		float gap = settings.m_fNoticeGap;
		if (lifeAt >= 0 && now - lifeAt < settings.m_fNoticeEarly)
			gap = Math.Max(gap, settings.m_fNoticeEarlyGap);

		while (stack && !m_aQueue.IsEmpty() && m_aCards.Count() < maxCards)
		{
			RZ_QueuedNotice q = m_aQueue[0];
			int qp = q.m_Style.m_iPriority;
			if (qp < RZ_ENoticePriority.URGENT && !q.m_Style.m_bSpawn && now - m_fLastCardAt < gap)
				break;

			m_aQueue.RemoveOrdered(0);
			Widget card = RZ_UIW.Create(settings.m_sNoticeLayout, stack);
			if (!card)
				break;

			FillCard(card, q.m_sTitle, q.m_sText, q.m_Style);
			m_aCards.Insert(card);
			m_aCardStart.Insert(now);
			m_aCardPriority.Insert(qp);
			m_fLastCardAt = now;
			CardSound();
		}

		RZ_UIW.Show(m_wRoot, "NoticeQueued", m_iToNotebook > 0);
		RZ_UIW.SetText(m_wRoot, "NoticeQueued", string.Format("+%1 in your notebook", m_iToNotebook));
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveCard(int i)
	{
		RZ_UIW.Remove(m_aCards[i]);
		m_aCards.RemoveOrdered(i);
		m_aCardStart.RemoveOrdered(i);
		m_aCardPriority.RemoveOrdered(i);
	}

	// ================================================================ Subtitles
	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.ShowSayLine and QueueHint (client). A radio line goes to the
	//! radio-line box at the top left when the layout has one; everything else, and a radio line
	//! without the box, to the subtitles at the bottom centre. radioOnly: a radio notice, which
	//! stays a card without the box. False = Subtitles off or no place for it: a card instead.
	static bool PushLine(RZ_SayLine line, bool radioOnly = false)
	{
		if (!s_Instance || !line || !RZ_UISettings.Get().m_bSubtitles)
			return false;

		Widget root = s_Instance.m_wRoot;
		if (line.m_iSource == RZ_ESaySource.RADIO && RZ_UIW.Find(root, "RadioLinePanel"))
		{
			s_Instance.m_Radio.Add(line, RADIO_CHARS, RADIO_WAITING, false);
			s_Instance.DrawRadioLine(RZ_UISettings.Get());
			return true;
		}

		if (radioOnly || !RZ_UIW.Find(root, "SubtitlePanel"))
			return false;

		// A newer line from the same speaker replaces theirs still waiting; at most four wait
		s_Instance.m_Subs.Add(line, SUB_CHARS, SUB_WAITING, true);
		s_Instance.DrawSubtitle(RZ_UISettings.Get());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Bottom centre, one line at a time for shortest to longest seconds by length; held while the
	//! reader is open. The source icon, the name in their colour, the role, then for someone in
	//! person the arrow and the distance (DrawSubtitleDir), and the line under them.
	protected void DrawSubtitle(RZ_UISettings settings)
	{
		// Never under the talk screen or the reader: the line showing goes, the rest wait till it closes
		float now = RZ_UIW.Now();
		bool held = RZ_ReaderMenu.IsShowing() || RZ_TalkMenu.IsShowing();
		if (m_Subs.Expire(now, held))
			RZ_UIW.Show(m_wRoot, "SubtitlePanel", false);

		RZ_SayLine q = m_Subs.Next(now, held, settings);
		if (!q)
			return;

		Color c = q.NameColor();
		string name = q.m_sName;
		if (name.IsEmpty())
			name = q.m_sChannel;

		RZ_UIW.Icon(m_wRoot, "SubtitleSource", q.SourceIcon());
		RZ_UIW.Tint(m_wRoot, "SubtitleSource", c);
		RZ_UIW.SetText(m_wRoot, "SubtitleSpeaker", name);
		RZ_UIW.Tint(m_wRoot, "SubtitleSpeaker", c);
		RZ_UIW.Show(m_wRoot, "SubtitleRole", !q.m_sRole.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "SubtitleRole", "· " + q.m_sRole);
		RZ_UIW.SetText(m_wRoot, "SubtitleText", q.m_sText);
		m_sSubDist = "";
		DrawSubtitleDir();
		RZ_UIW.Show(m_wRoot, "SubtitlePanel", true);
	}

	//------------------------------------------------------------------------------------------------
	//! The arrow to an in-person speaker, turned from where you face, and how far they are ("12 m"),
	//! from their body while it streams in, else where they stood. Hidden for radio, loudspeakers and
	//! notes, and when the server had no place.
	protected void DrawSubtitleDir()
	{
		if (!m_wRoot)
			return;

		RZ_SayLine q = m_Subs.Showing();
		if (!q)
			return;

		vector there;
		IEntity me = RZ_UIW.LocalEntity();
		bool show = false;
		if (me)
			show = q.Where(there);

		RZ_UIW.Show(m_wRoot, "SubtitleDir", show);
		if (!show)
			return;

		vector here = me.GetOrigin();
		RZ_UIW.Rotate(m_wRoot, "SubtitleArrow", RZ_UIW.Bearing(here, there) - RZ_UIW.Heading(me));
		string dist = RZ_SayLine.Distance(RZ_Util.DistXZ(here, there));
		if (dist != m_sSubDist)
		{
			m_sSubDist = dist;
			RZ_UIW.SetText(m_wRoot, "SubtitleDist", dist);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Top left, under the tracker: a radio channel's voice, so it never looks like the person in
	//! front of you. The radio in the channel's colour, the channel's name in dim italics, the
	//! voice's name when it has one, and the line. Paced like the subtitles.
	protected void DrawRadioLine(RZ_UISettings settings)
	{
		float now = RZ_UIW.Now();
		bool held = RZ_ReaderMenu.IsShowing() || RZ_TalkMenu.IsShowing();
		if (m_Radio.Expire(now, held))
			RZ_UIW.Show(m_wRoot, "RadioLinePanel", false);

		RZ_SayLine q = m_Radio.Next(now, held, settings);
		if (!q)
			return;

		Color c = q.NameColor();
		RZ_UIW.Tint(m_wRoot, "RadioLineAccent", c);
		RZ_UIW.Icon(m_wRoot, "RadioLineIcon", q.SourceIcon());
		RZ_UIW.Tint(m_wRoot, "RadioLineIcon", c);
		RZ_UIW.SetText(m_wRoot, "RadioLineChannel", q.m_sChannel);
		RZ_UIW.Show(m_wRoot, "RadioLineSpeaker", !q.m_sName.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "RadioLineSpeaker", q.m_sName);
		RZ_UIW.Tint(m_wRoot, "RadioLineSpeaker", c);
		RZ_UIW.SetText(m_wRoot, "RadioLineText", q.m_sText);
		RZ_UIW.Show(m_wRoot, "RadioLinePanel", true);
	}

	//------------------------------------------------------------------------------------------------
	//! A new card's sound: vanilla's own hint sound (SCR_HintUIComponent plays the same event), so a
	//! card sounds like the hint it replaces. Cards are paced, so this never stacks.
	protected void CardSound()
	{
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.HINT);
	}

	//------------------------------------------------------------------------------------------------
	protected void FillCard(Widget card, string title, string text, RZ_NoticeStyle style)
	{
		Color c = style.Accent();
		RZ_UIW.Tint(card, "CardAccent", c);
		RZ_UIW.Tint(card, "CardIcon", c);
		RZ_UIW.Tint(card, "CardKind", c);
		RZ_UIW.Tint(card, "CardLife", c);
		string icon = style.Icon();
		RZ_UIW.Show(card, "CardIcon", !icon.IsEmpty());
		RZ_UIW.Icon(card, "CardIcon", icon);
		RZ_UIW.SetText(card, "CardKind", style.KindText());
		RZ_UIW.SetText(card, "CardTime", RZ_UIText.TimeOfDay());
		RZ_UIW.SetText(card, "CardSpeaker", title);
		RZ_UIW.SetText(card, "CardText", text);
		RZ_UIW.Show(card, "CardHazard", style.m_iKind == RZ_ENoticeKind.BROADCAST);
	}
}
