//! The spawn screen (docs/UI.md, "The spawn screen"): our own screen in place of vanilla's deploy
//! menu and role selection. The 13 backgrounds as a list; the picked one's name, perk, flaw,
//! starting gear, starting standing, whether it's locked (and how to unlock it) and Legacy toward
//! the Drifter; the kind of place that background starts at; Deploy. Nobody picks where they
//! start: the screen asks for one of the open spawn points at random, and the server's opening
//! rolls the real place (Guidance/RZ_OpeningPlaces.c). The faction is forced to SURV
//! (SCR_MenuSpawnLogic's Forced faction), so there's no faction or group step.
//!
//! It spawns through the vanilla requests: the loadout (SCR_PlayerLoadoutComponent.RequestLoadout),
//! then the spawn (SCR_RespawnComponent.RequestSpawn with the spawn point), so the server's checks,
//! RZ_LoadoutLock.c among them, apply as they did to the vanilla menu.
//!
//! Pad first: up/down pick a background, A deploys, B opens the pause menu. Keys: the arrows,
//! Enter, Esc. Mouse: click a row, Deploy.
//!
//! Hook: SCR_PlayerDeployMenuHandlerComponent (modded, below) opens this screen where vanilla would
//! open its deploy menu or role selection. With UI > Spawn row layout empty, or if the screen can't
//! open on a client (no preset or layout), vanilla's deploy menu opens as before.

//------------------------------------------------------------------------------------------------
modded class SCR_PlayerDeployMenuHandlerComponent // VERIFY: 1.9's DeployMenuSystem calls Update every frame on the local player's handler while it's ready, and its CanOpenMenu is true exactly when vanilla would show the deploy menu
{
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		// The welcome screen, and everything when our screen isn't used, stay vanilla's
		if (!RZ_SpawnMenu.Used() || CanOpenWelcomeScreen() || IsWelcomeScreenOpen())
		{
			super.Update(timeSlice);
			return;
		}

		if (CanOpenMenu())
		{
			if (SCR_DeployMenuMain.GetDeployMenu())
				SCR_DeployMenuMain.CloseDeployMenu();

			if (SCR_RoleSelectionMenu.GetRoleSelectionMenu())
				SCR_RoleSelectionMenu.CloseRoleSelectionMenu();

			if (!RZ_SpawnMenu.IsShowing())
				RZ_SpawnMenu.Open();

			return;
		}

		RZ_SpawnMenu.CloseIfOpen();
	}
}

//------------------------------------------------------------------------------------------------
class RZ_SpawnMenu : RZ_MenuBase
{
	protected static RZ_SpawnMenu s_Open;
	protected static bool s_bFailed;		// the menu couldn't open here: vanilla's deploy menu from now on
	protected static string s_sLastPicked;	// the loadout picked last (this session)

	protected int m_iLocal;					// bumps when something on the screen changes
	protected int m_iParsed = -1;
	protected int m_iSelected = -1;
	protected int m_iLegacy;
	protected int m_iDrifterAt = 10;
	protected ref array<ref array<string>> m_aBackgrounds = {};
	protected ref array<Widget> m_aRows = {};
	protected ref array<SCR_SpawnPoint> m_aPlaces = {};
	protected RplId m_PlaceId = RplId.Invalid();
	protected float m_fPlacesAge = 99;
	protected int m_iWait = -1;				// seconds before the respawn timer lets you in, as last drawn
	protected bool m_bBusy;					// a deploy request is on its way
	protected string m_sStatus;
	protected SCR_BasePlayerLoadout m_Pending;
	protected SCR_PlayerLoadoutComponent m_LoadoutComp;
	protected SCR_RespawnComponent m_Respawn;

	// ================================================================ Opening
	//------------------------------------------------------------------------------------------------
	//! The screen is used on this machine: UI > Spawn row layout set and it hasn't failed to open here.
	static bool Used()
	{
		return !s_bFailed && !RZ_UISettings.Get().m_sSpawnRowLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	static void Open()
	{
		if (s_Open || RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_SpawnMenu))
			return;

		// No preset or layout here: vanilla's deploy menu opens on the next frame
		s_bFailed = true;
		RZ_Log.Warn("Spawn screen: the RZ_SpawnMenu preset or its layout is missing, using the vanilla deploy menu");
	}

	//------------------------------------------------------------------------------------------------
	static void CloseIfOpen()
	{
		if (s_Open)
			s_Open.Close();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		RZ_UIW.OnClick(m_wRoot, "DeployButton", m_Click);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuDown", "Background");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Deploy") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Menu") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);

		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			m_LoadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
			m_Respawn = SCR_RespawnComponent.Cast(pc.FindComponent(SCR_RespawnComponent));
		}

		if (m_LoadoutComp)
			m_LoadoutComp.GetOnPlayerLoadoutResponseInvoker_O().Insert(OnLoadoutResponse);

		if (m_Respawn)
			m_Respawn.GetOnRespawnResponseInvoker_O().Insert(OnSpawnResponse);

		// Ask every time it opens: Legacy and unlocks change between lives
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.Ask(RZ_UIFeed.PAYLOAD_BACKGROUNDS);

		RefreshPlaces();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDown);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		if (m_LoadoutComp)
			m_LoadoutComp.GetOnPlayerLoadoutResponseInvoker_O().Remove(OnLoadoutResponse);

		if (m_Respawn)
			m_Respawn.GetOnRespawnResponseInvoker_O().Remove(OnSpawnResponse);

		if (s_Open == this)
			s_Open = null;

		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second: the spawn points open to you (they move every 20 minutes) and the respawn
	//! timer's wait.
	override void OnMenuUpdate(float tDelta)
	{
		m_fPlacesAge += tDelta;
		if (m_fPlacesAge >= 1)
		{
			RefreshPlaces();
			int wait = WaitSeconds();
			if (wait != m_iWait)
			{
				m_iWait = wait;
				m_iLocal++;
			}
		}

		super.OnMenuUpdate(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		int bg = 0;
		if (ui)
			bg = ui.GetBackgroundsVersion();

		return bg * 100000 + m_iLocal;
	}

	// ================================================================ Input
	//------------------------------------------------------------------------------------------------
	//! Only while nothing is open on top (the pause menu listens to the same actions).
	protected bool IsTop()
	{
		MenuManager mm = GetGame().GetMenuManager();
		return mm && mm.GetTopMenu() == this;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUp(float value = 0, EActionTrigger reason = 0)
	{
		if (IsTop())
			Pick(m_iSelected - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDown(float value = 0, EActionTrigger reason = 0)
	{
		if (IsTop())
			Pick(m_iSelected + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		if (IsTop())
			TryDeploy();
	}

	//------------------------------------------------------------------------------------------------
	//! Esc or B: there's nothing to go back to, so the pause menu opens (to leave, or for settings),
	//! as from vanilla's deploy menu.
	override protected void Back()
	{
		if (!IsTop())
			return;

		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PauseMenu, 0, true, false);
		PauseMenuUI pause = PauseMenuUI.Cast(menu);
		if (pause)
			pause.FadeBackground(true, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse: a row picks it, Deploy deploys.
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "DeployButton")
			TryDeploy();
		else if (name == "SpawnRow")
			Pick(RZ_UIW.GetTag(w));
	}

	//------------------------------------------------------------------------------------------------
	protected void Pick(int index)
	{
		if (m_aBackgrounds.IsEmpty() || m_bBusy)
			return;

		int idx = Math.Clamp(index, 0, m_aBackgrounds.Count() - 1);
		if (idx == m_iSelected)
			return;

		m_iSelected = idx;
		s_sLastPicked = RZ_UIText.Str(m_aBackgrounds[idx], 3);
		m_sStatus = "";
		m_iLocal++;
	}

	// ================================================================ The spawn point
	//------------------------------------------------------------------------------------------------
	//! The player faction's spawn points that are on; the one drawn at random is kept while it's
	//! still there. Nobody chooses it: it's only what the spawn request names, and the server's
	//! opening moves the new character to the place it rolls.
	protected void RefreshPlaces()
	{
		m_fPlacesAge = 0;
		string key = RZ_Factions.GetPlayerKey();
		if (key.IsEmpty())
			key = "SURV";

		int pid = 0;
		if (GetGame().GetPlayerController())
			pid = GetGame().GetPlayerController().GetPlayerId();

		array<SCR_SpawnPoint> found = SCR_SpawnPoint.GetSpawnPointsForFaction(key);
		m_aPlaces.Clear();
		bool kept = false;
		foreach (SCR_SpawnPoint sp : found)
		{
			if (!sp || !sp.IsSpawnPointEnabled() || !sp.IsSpawnPointVisibleForPlayer(pid))
				continue;

			m_aPlaces.Insert(sp);
			if (sp.GetRplId() == m_PlaceId)
				kept = true;
		}

		if (kept || m_aPlaces.IsEmpty())
		{
			if (m_aPlaces.IsEmpty() && m_PlaceId.IsValid())
			{
				m_PlaceId = RplId.Invalid();
				m_iLocal++;
			}

			return;
		}

		m_PlaceId = m_aPlaces.GetRandomElement().GetRplId();
		m_iLocal++;
	}

	//------------------------------------------------------------------------------------------------
	protected int PlaceIndex()
	{
		foreach (int i, SCR_SpawnPoint sp : m_aPlaces)
		{
			if (sp && sp.GetRplId() == m_PlaceId)
				return i;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SpawnPoint Place()
	{
		if (m_aPlaces.IsEmpty())
			return null;

		return m_aPlaces[PlaceIndex()];
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PlayerDeployMenuHandlerComponent Handler()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;

		return SCR_PlayerDeployMenuHandlerComponent.Cast(pc.FindComponent(SCR_PlayerDeployMenuHandlerComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds the respawn timer still holds you (0: go), as vanilla's deploy menu counts them.
	protected int WaitSeconds()
	{
		BaseGameMode gm = GetGame().GetGameMode();
		PlayerController pc = GetGame().GetPlayerController();
		if (!gm || !pc)
			return 0;

		SCR_RespawnTimerComponent timer = SCR_RespawnTimerComponent.Cast(gm.FindComponent(SCR_RespawnTimerComponent));
		if (!timer || !timer.IsPlayerEnqueued(pc.GetPlayerId()))
			return 0;

		float extra = 0;
		SCR_SpawnPoint sp = Place();
		if (sp)
			extra = sp.GetRespawnTime();

		return Math.Max(0, timer.GetPlayerRemainingTime(pc.GetPlayerId(), extra));
	}

	// ================================================================ Deploy
	//------------------------------------------------------------------------------------------------
	//! The loadout behind a background, by its name in the loadout manager, or null.
	protected SCR_BasePlayerLoadout FindLoadout(string loadoutName)
	{
		SCR_LoadoutManager manager = GetGame().GetLoadoutManager();
		if (!manager || !manager.GetPlayerLoadouts())
			return null;

		foreach (SCR_BasePlayerLoadout loadout : manager.GetPlayerLoadouts())
		{
			if (loadout && loadout.GetLoadoutName() == loadoutName)
				return loadout;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Asks for the picked background's loadout; once the server gives it, the spawn. The client's
	//! checks only spare a round trip: the server checks everything again (RZ_LoadoutLock.c).
	protected void TryDeploy()
	{
		if (m_bBusy || m_iSelected < 0 || m_iSelected >= m_aBackgrounds.Count())
			return;

		array<string> f = m_aBackgrounds[m_iSelected];
		if (RZ_UIText.Int(f, 6) == 1 && RZ_UISettings.Get().m_bLockLoadouts)
		{
			Say(LockedWhy(f) + " Until then you can't start as one.");
			return;
		}

		SCR_BasePlayerLoadout loadout = FindLoadout(RZ_UIText.Str(f, 3));
		if (!loadout || !m_LoadoutComp || !m_Respawn)
		{
			Say("This server has no loadout for that background. Pick another.");
			return;
		}

		if (!Place())
		{
			Say("Nowhere to start yet. Give it a moment.");
			return;
		}

		if (WaitSeconds() > 0)
			return;

		m_bBusy = true;
		m_Pending = loadout;
		Say("On your way.");
		if (!m_LoadoutComp.RequestLoadout(loadout))
		{
			m_bBusy = false;
			Say("That didn't go through. Try again.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The loadout came back: on yes, the spawn at the chosen place.
	protected void OnLoadoutResponse(SCR_PlayerLoadoutComponent component, int loadoutIndex, bool response)
	{
		if (!m_bBusy || !m_Pending)
			return;

		if (!response)
		{
			m_bBusy = false;
			Say("The server wouldn't give you that one. Pick again.");
			return;
		}

		SCR_SpawnPoint sp = Place();
		if (!sp)
		{
			m_bBusy = false;
			Say("Nowhere to start yet. Give it a moment.");
			return;
		}

		SCR_SpawnPointSpawnData data = new SCR_SpawnPointSpawnData(m_Pending.GetLoadoutResource(), sp.GetRplId());
		if (!m_Respawn.RequestSpawn(data))
		{
			m_bBusy = false;
			Say("That didn't go through. Try again.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The spawn came back: on OK the screen closes (the handler closes it too once you have a body).
	protected void OnSpawnResponse(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		if (!m_bBusy)
			return;

		m_bBusy = false;
		if (response == SCR_ESpawnResult.OK)
		{
			Close();
			return;
		}

		Say("You can't go in just yet. Try again in a moment.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Say(string status)
	{
		m_sStatus = status;
		m_iLocal++;
	}

	//------------------------------------------------------------------------------------------------
	protected string LockedWhy(array<string> f)
	{
		string why = RZ_UIText.Str(f, 7);
		if (why.IsEmpty())
			why = "You haven't unlocked it yet.";

		return why;
	}

	// ================================================================ Drawing
	//------------------------------------------------------------------------------------------------
	//! G|legacy|Legacy for the Drifter, then B|id|name|loadout|perk|flaw|locked|unlock text|Haven|
	//! Bastion|Red Hand|Halcyon|gear|where you start per background, in the game mode's order.
	protected void Parse(string payload)
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		m_aBackgrounds.Clear();
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "G")
			{
				m_iLegacy = RZ_UIText.Int(f, 1);
				m_iDrifterAt = Math.Max(1, RZ_UIText.Int(f, 2));
			}
			else if (tag == "B")
			{
				m_aBackgrounds.Insert(f);
			}
		}

		if (m_iSelected >= 0 && m_iSelected < m_aBackgrounds.Count())
			return;

		// The first time: the one picked last, else the first that isn't locked
		m_iSelected = -1;
		foreach (int i, array<string> b : m_aBackgrounds)
		{
			if (RZ_UIText.Str(b, 3) == s_sLastPicked)
				m_iSelected = i;
		}

		if (m_iSelected < 0)
		{
			foreach (int j, array<string> c : m_aBackgrounds)
			{
				if (m_iSelected < 0 && RZ_UIText.Int(c, 6) != 1)
					m_iSelected = j;
			}
		}

		m_iSelected = Math.Max(0, m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && ui.GetBackgroundsVersion() != m_iParsed)
		{
			m_iParsed = ui.GetBackgroundsVersion();
			Parse(ui.GetBackgrounds());
		}

		RZ_UIW.Show(m_wRoot, "ListEmpty", m_aBackgrounds.IsEmpty());
		DrawRows();
		DrawPanel();
		DrawFoot();
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawRows()
	{
		Widget list = RZ_UIW.Find(m_wRoot, "Rows");
		RZ_UIW.ClearChildren(list);
		m_aRows.Clear();
		ResourceName rowLayout = RZ_UISettings.Get().m_sSpawnRowLayout;
		foreach (int i, array<string> f : m_aBackgrounds)
		{
			Widget row = RZ_UIW.Create(rowLayout, list);
			if (!row)
				return;

			bool on = i == m_iSelected;
			bool locked = RZ_UIText.Int(f, 6) == 1;
			RZ_UIW.SetText(row, "RowName", RZ_UIText.Str(f, 2));
			RZ_UIW.Show(row, "RowBg", on);
			RZ_UIW.Show(row, "RowBar", on);
			RZ_UIW.Show(row, "RowLock", locked);
			RZ_UIW.Icon(row, "RowLock", "status_door_locked");
			string state = "";
			if (locked)
				state = "LOCKED";

			RZ_UIW.SetText(row, "RowState", state);
			Color fg = RZ_UIColor.Bone();
			if (locked)
				fg = RZ_UIW.Rgb(110, 106, 96);
			else if (on)
				fg = RZ_UIW.Rgb(244, 240, 230);

			RZ_UIW.Tint(row, "RowName", fg);
			RZ_UIW.SetTag(row, i);
			RZ_UIW.OnClick(row, "", m_Click);
			m_aRows.Insert(row);
		}

		if (m_iSelected >= 0 && m_iSelected < m_aRows.Count())
			RZ_UIW.FocusWidget(m_aRows[m_iSelected]);

		// Keep the picked row in view on a pad
		ScrollLayoutWidget sc = ScrollLayoutWidget.Cast(RZ_UIW.Find(m_wRoot, "RowScroll"));
		if (sc && m_aRows.Count() > 1)
			sc.SetSliderPos(0, Math.Max(0, m_iSelected) / (m_aRows.Count() - 1.0));
	}

	//------------------------------------------------------------------------------------------------
	//! The picked background: name, state, perk, flaw, gear, standing, how to unlock it, Legacy.
	protected void DrawPanel()
	{
		RZ_UIW.SetText(m_wRoot, "Legacy", string.Format("LEGACY %1 / %2 - THE DRIFTER AT %2", m_iLegacy, m_iDrifterAt));
		RZ_UIW.Progress(m_wRoot, "LegacyBar", m_iLegacy / Math.Max(1, m_iDrifterAt));
		bool has = m_iSelected >= 0 && m_iSelected < m_aBackgrounds.Count();
		RZ_UIW.Show(m_wRoot, "PanelBody", has);
		if (!has)
			return;

		array<string> f = m_aBackgrounds[m_iSelected];
		string bgName = RZ_UIText.Str(f, 2);
		bgName.ToUpper();
		RZ_UIW.SetText(m_wRoot, "BgName", bgName);
		RZ_UIW.SetText(m_wRoot, "BgPerk", RZ_UIText.Str(f, 4));
		string flaw = RZ_UIText.Str(f, 5);
		if (flaw.IsEmpty())
			flaw = "None to speak of.";

		RZ_UIW.SetText(m_wRoot, "BgFlaw", flaw);
		string gear = RZ_UIText.Str(f, 12);
		if (gear.IsEmpty())
			gear = "Whatever the loadout gives you.";

		RZ_UIW.SetText(m_wRoot, "BgGear", gear);
		string rep = "";
		for (int i = 0; i < 4; i++)
		{
			int v = RZ_UIText.Int(f, 8 + i);
			if (v > 0)
				rep += string.Format("%1 +%2 Fame\n", RZ_Factions.GetName(i), v);
			else if (v < 0)
				rep += string.Format("%1 +%2 Infamy\n", RZ_Factions.GetName(i), -v);
		}

		if (rep.IsEmpty())
			rep = "Nobody knows you.";

		RZ_UIW.SetText(m_wRoot, "BgStanding", rep);

		bool locked = RZ_UIText.Int(f, 6) == 1;
		bool noLoadout = !FindLoadout(RZ_UIText.Str(f, 3));
		RZ_UIW.Show(m_wRoot, "LockedBlock", locked);
		RZ_UIW.Icon(m_wRoot, "StateIcon", "status_door_unlocked");
		RZ_UIW.SetText(m_wRoot, "StateText", "AVAILABLE");
		RZ_UIW.Tint(m_wRoot, "StateText", RZ_UIColor.Good());
		if (locked)
		{
			RZ_UIW.Icon(m_wRoot, "StateIcon", "status_door_locked");
			RZ_UIW.SetText(m_wRoot, "StateText", "LOCKED");
			RZ_UIW.Tint(m_wRoot, "StateText", RZ_UIColor.Dim());
			string after = " Pick it now and you keep its gear, but you play as the default background this life.";
			if (RZ_UISettings.Get().m_bLockLoadouts)
				after = " Until then you can't start as one.";

			RZ_UIW.SetText(m_wRoot, "LockedText", LockedWhy(f) + after);
		}
		else if (noLoadout)
		{
			RZ_UIW.SetText(m_wRoot, "StateText", "NOT ON THIS SERVER");
			RZ_UIW.Tint(m_wRoot, "StateText", RZ_UIColor.Dim());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The kind of place the picked background starts at (never where: the server rolls it), what's
	//! holding you, and Deploy.
	protected void DrawFoot()
	{
		SCR_SpawnPoint sp = Place();
		string place = "Somewhere in the Zone";
		if (m_iSelected >= 0 && m_iSelected < m_aBackgrounds.Count() && !RZ_UIText.Str(m_aBackgrounds[m_iSelected], 13).IsEmpty())
			place = RZ_UIText.Str(m_aBackgrounds[m_iSelected], 13);

		RZ_UIW.SetText(m_wRoot, "PlaceName", place);

		bool locked = false;
		if (m_iSelected >= 0 && m_iSelected < m_aBackgrounds.Count())
			locked = RZ_UIText.Int(m_aBackgrounds[m_iSelected], 6) == 1 && RZ_UISettings.Get().m_bLockLoadouts;

		int wait = Math.Max(0, m_iWait);
		string word = "DEPLOY";
		if (m_bBusy)
			word = "ON YOUR WAY";
		else if (wait > 0)
			word = "DEPLOY IN " + RZ_UIText.Clock(wait);

		RZ_UIW.SetText(m_wRoot, "DeployText", word);
		bool ready = !m_bBusy && wait <= 0 && !locked && sp != null;
		Color bg = RZ_UIColor.Bone();
		Color fg = RZ_UIW.Rgb(21, 23, 26);
		if (!ready)
		{
			bg = RZ_UIW.Rgb(27, 31, 34);
			fg = RZ_UIColor.Dim();
		}

		RZ_UIW.Tint(m_wRoot, "DeployBg", bg);
		RZ_UIW.Tint(m_wRoot, "DeployText", fg);
		RZ_UIW.SetText(m_wRoot, "SpawnStatus", m_sStatus);
		RZ_UIW.Show(m_wRoot, "SpawnStatus", !m_sStatus.IsEmpty());
	}
}
