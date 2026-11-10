//! The ReforgerZ HUD (docs/UI.md, "HUD"): the infection timer with its stages and the golden
//! window, the secure-radio indicator, a subtle noise meter, and the styled notice cards.
//!
//! An SCR_InfoDisplay: add it to the HUD manager's info displays on the player controller and
//! give it the layout from UI/LAYOUTS.md ("HUD"). It reads only RZ_PlayerUIComponent's client
//! cache, redraws five times a second, and changes nothing in the game.
//!
//! Notices: RZ_GameModeComponent.QueueHint calls PushNotice first. It returns false when styled
//! notices are off, the layout has no NoticeStack, or no card layout is set; the vanilla hint is
//! used then, exactly as before.
class RZ_HudDisplay : SCR_InfoDisplay // VERIFY: SCR_InfoDisplay, its m_wRoot and the DisplayStartDraw/DisplayStopDraw/DisplayUpdate overrides
{
	protected static const float REFRESH = 0.2;

	protected static RZ_HudDisplay s_Instance;

	protected float m_fRefresh;
	protected ref array<string> m_aQTitles = {};
	protected ref array<string> m_aQTexts = {};
	protected ref array<ref RZ_NoticeStyle> m_aQStyles = {};
	protected ref array<Widget> m_aCards = {};
	protected ref array<float> m_aCardStart = {};

	//------------------------------------------------------------------------------------------------
	override protected void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		s_Instance = this;
		RZ_UIW.Listen("RZ_OpenStanding", OpenStanding);
		RZ_UIW.Listen("RZ_OpenJournal", OpenJournal);
		RZ_UIW.Listen("RZ_OpenRadio", OpenRadio);
		RZ_UIW.Listen("RZ_OpenHall", OpenHall);
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayStopDraw(IEntity owner)
	{
		RZ_UIW.Unlisten("RZ_OpenStanding", OpenStanding);
		RZ_UIW.Unlisten("RZ_OpenJournal", OpenJournal);
		RZ_UIW.Unlisten("RZ_OpenRadio", OpenRadio);
		RZ_UIW.Unlisten("RZ_OpenHall", OpenHall);
		foreach (Widget w : m_aCards)
		{
			RZ_UIW.Remove(w);
		}

		m_aCards.Clear();
		m_aCardStart.Clear();
		if (s_Instance == this)
			s_Instance = null;

		super.DisplayStopDraw(owner);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		m_fRefresh -= timeSlice;
		if (m_fRefresh > 0)
			return;

		m_fRefresh = REFRESH;
		Refresh();
	}

	// ================================================================ Input
	//------------------------------------------------------------------------------------------------
	protected void OpenStanding(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_StandingMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenJournal(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_JournalMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenRadio(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_RadioLogMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenHall(float value = 0, EActionTrigger reason = 0)
	{
		RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_HallOfFameMenu);
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
		}
		else
		{
			RZ_UIW.Show(m_wRoot, "InfectionPanel", false);
			RZ_UIW.Show(m_wRoot, "RadioPanel", false);
			RZ_UIW.Show(m_wRoot, "NoisePanel", false);
		}

		DrawNotices(settings);
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
		channel.ToUpper(); // VERIFY: ToUpper() changes the string in place
		RZ_UIW.SetText(m_wRoot, "RadioChannel", channel);
		int minutes = Math.Max(1, RZ_Util.RoundToInt(ui.GetListenLeft() / 60));
		RZ_UIW.SetText(m_wRoot, "RadioSub", string.Format("Cracked - codes change in %1 min", minutes));
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawNoise(RZ_PlayerUIComponent ui, RZ_UISettings settings)
	{
		RZ_UIW.Show(m_wRoot, "NoisePanel", settings.m_bNoiseMeter);
		if (!settings.m_bNoiseMeter)
			return;

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
	protected void Enqueue(string title, string text, RZ_NoticeStyle style)
	{
		// Under heavy traffic, drop the oldest waiting notices rather than lag minutes behind
		int cap = Math.Max(1, RZ_UISettings.Get().m_iNoticeQueue);
		while (m_aQTitles.Count() >= cap)
		{
			m_aQTitles.RemoveOrdered(0);
			m_aQTexts.RemoveOrdered(0);
			m_aQStyles.RemoveOrdered(0);
		}

		m_aQTitles.Insert(title);
		m_aQTexts.Insert(text);
		m_aQStyles.Insert(style);
		DrawNotices(RZ_UISettings.Get());
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawNotices(RZ_UISettings settings)
	{
		float now = RZ_UIW.Now();
		float life = Math.Max(2, settings.m_fNoticeSeconds);
		for (int i = m_aCards.Count() - 1; i >= 0; i--)
		{
			float share = (now - m_aCardStart[i]) / life;
			if (share >= 1)
			{
				RZ_UIW.Remove(m_aCards[i]);
				m_aCards.Remove(i);
				m_aCardStart.Remove(i);
				continue;
			}

			RZ_UIW.Progress(m_aCards[i], "CardLife", 1 - share);
		}

		Widget stack = RZ_UIW.Find(m_wRoot, "NoticeStack");
		int maxCards = Math.Max(1, settings.m_iNoticeStack);
		while (stack && !m_aQTitles.IsEmpty() && m_aCards.Count() < maxCards)
		{
			Widget card = RZ_UIW.Create(settings.m_sNoticeLayout, stack);
			if (card)
			{
				FillCard(card, m_aQTitles[0], m_aQTexts[0], m_aQStyles[0]);
				m_aCards.Insert(card);
				m_aCardStart.Insert(now);
			}

			m_aQTitles.RemoveOrdered(0);
			m_aQTexts.RemoveOrdered(0);
			m_aQStyles.RemoveOrdered(0);
			if (!card)
				break;
		}

		RZ_UIW.Show(m_wRoot, "NoticeQueued", !m_aQTitles.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "NoticeQueued", string.Format("+%1 QUEUED", m_aQTitles.Count()));
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
