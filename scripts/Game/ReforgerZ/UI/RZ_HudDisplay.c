//! The ReforgerZ HUD (docs/UI.md, "HUD"): the infection timer with its stages and the golden
//! window, the secure-radio indicator, a subtle noise meter with how visible you are above it
//! (Stealth on), and the styled notice cards.
//!
//! An SCR_InfoDisplay: add it to the HUD manager's info displays on the player controller and
//! give it the layout from UI/LAYOUTS.md ("HUD"). It reads only RZ_PlayerUIComponent's client
//! cache, redraws five times a second, and changes nothing in the game.
//!
//! Notices: RZ_GameModeComponent.QueueHint calls PushNotice first. It returns false when styled
//! notices are off, the layout has no NoticeStack, or no card layout is set; the vanilla hint is
//! used then, exactly as before.

//------------------------------------------------------------------------------------------------
//! A spoken line waiting for the subtitle panel.
class RZ_QueuedSubtitle
{
	string m_sSpeaker;
	int m_iFaction;
	string m_sText;
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
	protected ref array<ref RZ_QueuedSubtitle> m_aSubs = {};
	protected float m_fSubUntil = -1;		// the line showing stays until then; -1 none showing
	protected static const int SUB_WAITING = 4;
	protected static const int SUB_CHARS = 130;	// about two lines at 32 px in 1,044 units

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
		m_fRefresh -= timeSlice;
		if (m_fRefresh > 0)
			return;

		m_fRefresh = REFRESH;
		Refresh();
	}

	// ================================================================ Input
	//------------------------------------------------------------------------------------------------
	//! N: the notebook, on the tab last used (docs/UI.md, "The notebook").
	protected void OpenNotebook(float value = 0, EActionTrigger reason = 0)
	{
		RZ_Notebook.Open(-1);
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
		}
		else
		{
			RZ_UIW.Show(m_wRoot, "StandPanel", false);
			RZ_UIW.Show(m_wRoot, "InfectionPanel", false);
			RZ_UIW.Show(m_wRoot, "RadioPanel", false);
			RZ_UIW.Show(m_wRoot, "NoisePanel", false);
			RZ_UIW.Show(m_wRoot, "SeenPanel", false);
		}

		DrawNotices(settings);
		DrawSubtitle(settings);
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
	//! one at most every Gap seconds; urgent ones skip the gap and may cut the least important card
	//! short; low and normal ones the Radio tab lists drop out after Wait seconds (counted as "+N in
	//! your notebook"); for Calm seconds after a new life only urgent and high ones show.
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

		// Waited too long: it's in the notebook already
		float wait = Math.Max(5, settings.m_fNoticeWait);
		for (int w = m_aQueue.Count() - 1; w >= 0; w--)
		{
			RZ_NoticeStyle st = m_aQueue[w].m_Style;
			if (st.m_iPriority <= RZ_ENoticePriority.NORMAL && st.IsLogged() && now - m_aQueue[w].m_fAt > wait)
			{
				m_aQueue.RemoveOrdered(w);
				m_iToNotebook++;
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

		bool calm = lifeAt >= 0 && now - lifeAt < settings.m_fNoticeCalm;
		while (stack && !m_aQueue.IsEmpty() && m_aCards.Count() < maxCards)
		{
			RZ_QueuedNotice q = m_aQueue[0];
			int qp = q.m_Style.m_iPriority;
			if (calm && qp < RZ_ENoticePriority.HIGH)
				break;

			if (qp < RZ_ENoticePriority.URGENT && now - m_fLastCardAt < settings.m_fNoticeGap)
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
	//! From RZ_GameModeComponent.ShowSay (client). False = no subtitle panel, or Subtitles off:
	//! the line becomes a card instead.
	static bool PushSubtitle(string speaker, int faction, string text)
	{
		if (!s_Instance || !RZ_UISettings.Get().m_bSubtitles || !RZ_UIW.Find(s_Instance.m_wRoot, "SubtitlePanel"))
			return false;

		s_Instance.EnqueueSubtitle(speaker, faction, text);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! One line at a time; a long one is split into parts shown in turn. A newer line from the same
	//! speaker replaces one of theirs still waiting; at most four wait.
	protected void EnqueueSubtitle(string speaker, int faction, string text)
	{
		for (int i = m_aSubs.Count() - 1; i >= 0; i--)
		{
			if (m_aSubs[i].m_sSpeaker == speaker)
				m_aSubs.RemoveOrdered(i);
		}

		array<string> parts = {};
		RZ_UIText.SplitLine(text, SUB_CHARS, parts);
		foreach (string part : parts)
		{
			RZ_QueuedSubtitle q = new RZ_QueuedSubtitle();
			q.m_sSpeaker = speaker;
			q.m_iFaction = faction;
			q.m_sText = part;
			m_aSubs.Insert(q);
		}

		while (m_aSubs.Count() > SUB_WAITING + parts.Count() - 1)
		{
			m_aSubs.RemoveOrdered(0);
		}

		DrawSubtitle(RZ_UISettings.Get());
	}

	//------------------------------------------------------------------------------------------------
	//! On screen for shortest to longest seconds by length; held while the reader is open.
	protected void DrawSubtitle(RZ_UISettings settings)
	{
		float now = RZ_UIW.Now();
		bool held = RZ_ReaderMenu.IsShowing();
		if (m_fSubUntil >= 0 && (now >= m_fSubUntil || held))
		{
			m_fSubUntil = -1;
			RZ_UIW.Show(m_wRoot, "SubtitlePanel", false);
		}

		if (m_fSubUntil >= 0 || held || m_aSubs.IsEmpty())
			return;

		RZ_QueuedSubtitle q = m_aSubs[0];
		m_aSubs.RemoveOrdered(0);
		Color c = RZ_UIColor.Bone();
		if (q.m_iFaction >= 0)
			c = RZ_UIColor.Faction(q.m_iFaction);

		RZ_UIW.SetText(m_wRoot, "SubtitleSpeaker", q.m_sSpeaker);
		RZ_UIW.Tint(m_wRoot, "SubtitleSpeaker", c);
		RZ_UIW.SetText(m_wRoot, "SubtitleText", q.m_sText);
		RZ_UIW.Show(m_wRoot, "SubtitlePanel", true);
		float seconds = q.m_sText.Length() / Math.Max(1, settings.m_fSubtitleCps);
		m_fSubUntil = now + Math.Clamp(seconds, settings.m_fSubtitleMin, Math.Max(settings.m_fSubtitleMin, settings.m_fSubtitleMax));
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
