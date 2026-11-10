//! The background panel for the vanilla deploy menu (docs/UI.md, "Background picker"): the
//! picked loadout's background, its perk and flaw, starting standing, its starting gear, and
//! whether it's locked for this player (with how to unlock it). Locked loadouts themselves are
//! refused by the deploy menu (UI/RZ_LoadoutLock.c).
//!
//! No modded deploy-menu class: put a widget built from the BackgroundPanel layout into the deploy
//! menu's layout and add this component to it (UI/LAYOUTS.md). It polls the picked loadout twice a
//! second while the menu is open. If it isn't added, the spawn notice still names the perk and flaw.
class RZ_BackgroundPanelComponent : ScriptedWidgetComponent // VERIFY: ScriptedWidgetComponent, HandlerAttached/HandlerDeattached
{
	protected static const int POLL_MS = 500;

	protected Widget m_wRoot;
	protected string m_sShown;
	protected int m_iSeen = -1;
	protected bool m_bAsked;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w;
		RZ_UIW.Show(m_wRoot, "PanelBody", false);
		GetGame().GetCallqueue().CallLater(Poll, POLL_MS, true);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(Poll);

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void Poll()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		// Ask once per menu, and again after a death (Legacy and unlocks may have changed)
		if (!m_bAsked)
		{
			m_bAsked = true;
			ui.Ask(RZ_UIFeed.PAYLOAD_BACKGROUNDS);
		}

		string loadout = RZ_UIW.SelectedLoadoutName();
		if (loadout == m_sShown && ui.GetBackgroundsVersion() == m_iSeen)
			return;

		m_sShown = loadout;
		m_iSeen = ui.GetBackgroundsVersion();
		Draw(ui.GetBackgrounds(), loadout);
	}

	//------------------------------------------------------------------------------------------------
	//! G|legacy|Legacy for the Drifter
	//! B|id|name|loadout|perk|flaw|locked|unlock text|Haven|Bastion|Red Hand|Halcyon|gear
	protected void Draw(string payload, string loadout)
	{
		array<string> lines = {};
		RZ_UIText.Split(payload, "\n", lines);
		string foundLine = "";
		int legacy = 0;
		int drifterAt = 10;
		foreach (string ln : lines)
		{
			array<string> f = {};
			RZ_UIText.Split(ln, RZ_UIText.SEP, f);
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "G")
			{
				legacy = RZ_UIText.Int(f, 1);
				drifterAt = RZ_UIText.Int(f, 2);
			}
			else if (tag == "B" && RZ_UIText.Str(f, 3) == loadout)
			{
				foundLine = ln;
			}
		}

		RZ_UIW.Show(m_wRoot, "PanelBody", !foundLine.IsEmpty());
		RZ_UIW.SetText(m_wRoot, "Legacy", string.Format("LEGACY %1 / %2", legacy, drifterAt));
		RZ_UIW.Progress(m_wRoot, "LegacyBar", legacy / Math.Max(1, drifterAt));
		if (foundLine.IsEmpty())
			return;

		array<string> found = {};
		RZ_UIText.Split(foundLine, RZ_UIText.SEP, found);
		string bgName = RZ_UIText.Str(found, 2);
		bgName.ToUpper();
		RZ_UIW.SetText(m_wRoot, "BgName", bgName);
		RZ_UIW.SetText(m_wRoot, "BgPerk", RZ_UIText.Str(found, 4));
		RZ_UIW.SetText(m_wRoot, "BgFlaw", RZ_UIText.Str(found, 5));
		string gear = RZ_UIText.Str(found, 12);
		if (gear.IsEmpty())
			gear = "Whatever the loadout gives you.";

		RZ_UIW.SetText(m_wRoot, "BgGear", gear);

		bool locked = RZ_UIText.Int(found, 6) == 1;
		RZ_UIW.Show(m_wRoot, "LockedBlock", locked);
		RZ_UIW.Icon(m_wRoot, "StateIcon", "status_door_unlocked");
		RZ_UIW.SetText(m_wRoot, "StateText", "AVAILABLE");
		RZ_UIW.Tint(m_wRoot, "StateText", RZ_UIColor.Good());
		if (locked)
		{
			string why = RZ_UIText.Str(found, 7);
			if (why.IsEmpty())
				why = "You haven't unlocked it yet.";

			RZ_UIW.Icon(m_wRoot, "StateIcon", "status_door_locked");
			RZ_UIW.SetText(m_wRoot, "StateText", "LOCKED");
			RZ_UIW.Tint(m_wRoot, "StateText", RZ_UIColor.Dim());
			string after = " Pick it now and you keep its gear, but you play as the default background this life.";
			if (RZ_UISettings.Get().m_bLockLoadouts)
				after = " Until then the deploy menu won't let you take it.";

			RZ_UIW.SetText(m_wRoot, "LockedText", why + after);
		}

		string rep = "";
		for (int i = 0; i < 4; i++)
		{
			int v = RZ_UIText.Int(found, 8 + i);
			if (v > 0)
				rep += string.Format("%1 +%2 Fame\n", RZ_Factions.GetName(i), v);
			else if (v < 0)
				rep += string.Format("%1 +%2 Infamy\n", RZ_Factions.GetName(i), -v);
		}

		if (rep.IsEmpty())
			rep = "Nobody knows you.";

		RZ_UIW.SetText(m_wRoot, "BgStanding", rep);
	}
}
