//! The map's icon layer (docs/UI.md, "The map"): the ReforgerZ marker icons on the real map, for the
//! things this player is allowed to know. The server works out the list (RZ_UIFeed.BuildMap: the
//! lab, safehouses unless exiled, strongholds, outposts they know or have seen, the power and radio
//! stations, the sealed checkpoints, world events; never contracts, which give a rough area only)
//! and sends it to this player alone when the map opens, then every 20 s while it's open.
//!
//! The map key sits on the map too: a compact legend in the bottom-left (UI > Map legend layout),
//! shown while the map is zoomed out and faded as you zoom in. RZ_MapLegend (I, pad Y) shows or
//! hides it; the choice lasts the session.
//!
//! Add RZ_MapIconsUI to the components of the map configuration the game uses (the full-screen map
//! config; WORKBENCH_SETUP.md, "Setting up the UI") and set UI > Map icon layout. Without the
//! layout, the layer does nothing and the vanilla text markers stay (and there's no legend).
class RZ_MapIconsUI : SCR_MapUIBaseComponent
{
	protected static const float HALF = 24;	// half the icon's size (48), to centre it on the spot
	protected static const string LEGEND_ACTION = "RZ_MapLegend";

	protected static bool s_bLegendHidden;	// the player hid the legend (this session)

	protected Widget m_wLegend;

	protected ref array<Widget> m_aIcons = {};
	protected ref array<float> m_aX = {};
	protected ref array<float> m_aZ = {};
	protected ref array<float> m_aRadius = {};	// a range ring's radius in metres (0: a fixed-size icon)
	protected int m_iSeen = -1;
	protected float m_fAsk;
	protected bool m_bOpen;

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		m_bOpen = true;
		m_iSeen = -1;
		m_fAsk = 0;
		Ask();
		MakeLegend();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		m_bOpen = false;
		Clear();
		RemoveLegend();
		super.OnMapClose(config);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		RZ_UISettings settings = RZ_UISettings.Get();
		if (!m_bOpen || !settings.UsesMapIcons())
			return;

		m_fAsk += timeSlice;
		if (m_fAsk >= Math.Max(5, settings.m_fMapRefreshSeconds))
		{
			m_fAsk = 0;
			Ask();
		}

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && ui.GetMapVersion() != m_iSeen)
		{
			m_iSeen = ui.GetMapVersion();
			Build(ui.GetMap(), settings.m_sMapIconLayout);
		}

		Place();
		FadeLegend(settings);
	}

	//------------------------------------------------------------------------------------------------
	protected void Ask()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && RZ_UISettings.Get().UsesMapIcons())
			ui.Ask(RZ_UIFeed.PAYLOAD_MAP);
	}

	//------------------------------------------------------------------------------------------------
	protected void Clear()
	{
		foreach (Widget w : m_aIcons)
		{
			RZ_UIW.Remove(w);
		}

		m_aIcons.Clear();
		m_aX.Clear();
		m_aZ.Clear();
		m_aRadius.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! P|icon|faction|x|z|label[|radius]
	protected void Build(string payload, ResourceName layout)
	{
		Clear();
		if (!m_RootWidget)
			return;

		array<string> lines = {};
		RZ_UIText.Split(payload, "\n", lines);
		foreach (string ln : lines)
		{
			array<string> f = {};
			RZ_UIText.Split(ln, RZ_UIText.SEP, f);
			if (RZ_UIText.Str(f, 0) != "P")
				continue;

			Widget w = RZ_UIW.Create(layout, m_RootWidget);
			if (!w)
				return;

			int faction = RZ_UIText.Int(f, 2);
			Color c = RZ_UIColor.Bone();
			string icon = RZ_UIText.Str(f, 1);
			if (faction >= 0)
				c = RZ_UIColor.Faction(faction);
			else if (icon == "marker_event" || icon == "marker_checkpoint" || icon == "marker_range")
				c = RZ_UIColor.Hazard();

			RZ_UIW.Icon(w, "Icon", icon);
			RZ_UIW.Tint(w, "Icon", c);
			RZ_UIW.SetText(w, "Label", RZ_UIText.Str(f, 5));
			m_aIcons.Insert(w);
			m_aX.Insert(RZ_UIText.Int(f, 3));
			m_aZ.Insert(RZ_UIText.Int(f, 4));
			m_aRadius.Insert(RZ_UIText.Int(f, 6));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame while the map is open: the icons follow pan and zoom; a range ring is sized to its
	//! radius on the map (48 to 320 units across).
	protected void Place()
	{
		foreach (int i, Widget w : m_aIcons)
		{
			float sx, sy;
			if (!RZ_UIW.MapToScreen(m_MapEntity, m_aX[i], m_aZ[i], sx, sy))
				continue;

			float half = HALF;
			float ex, ey;
			if (m_aRadius[i] > 0 && RZ_UIW.MapToScreen(m_MapEntity, m_aX[i] + m_aRadius[i], m_aZ[i], ex, ey))
			{
				half = Math.Clamp(Math.AbsFloat(ex - sx), HALF, 160);
				RZ_UIW.SetSize(w, half * 2, half * 2);
				RZ_UIW.SetSize(RZ_UIW.Find(w, "Icon"), half * 2, half * 2);
			}

			RZ_UIW.SetPos(w, sx - half, sy - half);
		}
	}

	// ================================================================ The legend
	//------------------------------------------------------------------------------------------------
	//! The map key in the bottom-left, with its show/hide input. Nothing without both layouts.
	protected void MakeLegend()
	{
		RemoveLegend();
		RZ_UISettings settings = RZ_UISettings.Get();
		if (!m_RootWidget || !settings.UsesMapIcons())
			return;

		m_wLegend = RZ_UIW.Create(settings.m_sMapLegendLayout, m_RootWidget);
		if (!m_wLegend)
			return;

		// Its icons, in the colours the map draws them in (a stronghold and an outpost in two
		// factions' colours, as examples: the colour says whose it is)
		LegendIcon("marker_stronghold", RZ_UIColor.Faction(RZ_EFaction.MILITARY));
		LegendIcon("marker_outpost", RZ_UIColor.Faction(RZ_EFaction.BANDITS));
		LegendIcon("marker_safehouse", RZ_UIColor.Faction(RZ_EFaction.SURVIVORS));
		LegendIcon("marker_lab", RZ_UIColor.Faction(RZ_EFaction.INSTITUTE));
		LegendIcon("marker_event", RZ_UIColor.Hazard());
		LegendIcon("marker_checkpoint", RZ_UIColor.Hazard());
		LegendIcon("marker_power", RZ_UIColor.Bone());
		LegendIcon("marker_radio", RZ_UIColor.Bone());
		LegendIcon("marker_range", RZ_UIColor.Hazard());
		bool prompt = RZ_UIW.AddPrompt(m_wLegend, "LegendPrompt", LEGEND_ACTION, "Hide the key");
		RZ_UIW.Show(m_wLegend, "LegendPromptText", !prompt);
		// VERIFY: RZ_MapLegend (MapContext) fires while the full-screen map is open, and pad Y there
		// does nothing else (vanilla binds Y in the map only with D-pad left, held, to dismiss a hint)
		GetGame().GetInputManager().AddActionListener(LEGEND_ACTION, EActionTrigger.DOWN, OnToggleLegend);
		FadeLegend(settings);
	}

	//------------------------------------------------------------------------------------------------
	//! One of the legend's icons (Key_<icon> in the layout): its image from the imageset, and its colour.
	protected void LegendIcon(string icon, Color c)
	{
		RZ_UIW.Icon(m_wLegend, "Key_" + icon, icon);
		RZ_UIW.Tint(m_wLegend, "Key_" + icon, c);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveLegend()
	{
		if (m_wLegend)
			GetGame().GetInputManager().RemoveActionListener(LEGEND_ACTION, EActionTrigger.DOWN, OnToggleLegend);

		RZ_UIW.Remove(m_wLegend);
		m_wLegend = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnToggleLegend(float value = 0, EActionTrigger reason = 0)
	{
		s_bLegendHidden = !s_bLegendHidden;
		FadeLegend(RZ_UISettings.Get());
	}

	//------------------------------------------------------------------------------------------------
	//! Shown in full while the map is zoomed out, fading as you zoom in, gone when zoomed in close
	//! (UI > Map legend full and gone zooms: how wide a kilometre of map is on screen).
	protected void FadeLegend(RZ_UISettings settings)
	{
		if (!m_wLegend)
			return;

		float alpha = 0;
		if (!s_bLegendHidden)
		{
			float sx, sy, ex, ey;
			float km = 0;
			if (RZ_UIW.MapToScreen(m_MapEntity, 0, 0, sx, sy) && RZ_UIW.MapToScreen(m_MapEntity, 1000, 0, ex, ey))
				km = Math.AbsFloat(ex - sx);

			float full = Math.Max(1, settings.m_fLegendFullZoom);
			float gone = Math.Max(full + 1, settings.m_fLegendGoneZoom);
			alpha = Math.Clamp((gone - km) / (gone - full), 0, 1);
		}

		RZ_UIW.Fade(m_wLegend, alpha);
	}
}
