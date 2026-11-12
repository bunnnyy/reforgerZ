//! The map's icon layer (docs/UI.md, "The map"): the ReforgerZ marker icons on the real map, for the
//! things this player is allowed to know. The server works out the list (RZ_UIFeed.BuildMap: the
//! lab, safehouses unless exiled, strongholds, outposts they know or have seen, the power and radio
//! stations, the sealed checkpoints, world events; never contracts, which give a rough area only)
//! and sends it to this player alone when the map opens, then every 20 s while it's open.
//!
//! Each place is a marker (design A): a solid disc in its owner's colour with the faction's emblem on it
//! in bone, a thin near-black ring round it with a small gap, and at the bottom right a small badge for
//! what the place is (stronghold, outpost, safehouse, the lab). Places without a faction (the stations,
//! world events, checkpoints) get the same disc and ring in their own colour with their glyph, no badge.
//! A pack's range ring stays a ring sized to its ground.
//!
//! You are here: the local player's own marker, an arrow on a dark disc turned to where they face,
//! drawn above the other icons and moved every frame while the map is open. It's client-side only,
//! from the controlled entity; nothing is sent. It shows with or without vanilla's markers.
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
	protected static const float HALF = 17;	// half the marker's size (34), to centre it on the spot
	protected static const float RING_MIN = 24;	// a range ring is 48 to 320 across
	protected static const float RING_MAX = 160;
	protected static const float LABEL_W = 280;	// the label's width, centred under the marker
	protected static const float LABEL_MERGE = 40;	// markers this close (m) share one label
	protected static const float SELF_HALF = 22;	// half the you-are-here marker's size (44)
	protected static const string LEGEND_ACTION = "RZ_MapLegend";

	protected static bool s_bLegendHidden;	// the player hid the legend (this session)

	protected Widget m_wLegend;
	protected Widget m_wSelfBack;	// you are here: the dark disc
	protected Widget m_wSelf;		// and the arrow on it

	protected ref array<Widget> m_aIcons = {};
	protected ref array<float> m_aX = {};
	protected ref array<float> m_aZ = {};
	protected ref array<float> m_aRadius = {};	// a range ring's radius in metres (0: a fixed-size icon)
	protected ref array<string> m_aLabels = {};
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
		MakeSelf();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		m_bOpen = false;
		Clear();
		RemoveSelf();
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
		PlaceSelf();
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
		m_aLabels.Clear();
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
			string icon = RZ_UIText.Str(f, 1);
			if (RZ_UIText.Int(f, 6) > 0)
			{
				// A range ring: the ring alone, in the faction's colour (the tracked job's giver) or hazard
				Color ringColor = RZ_UIColor.Hazard();
				if (faction >= 0)
					ringColor = RZ_UIColor.Faction(faction);

				Solo(w, icon, ringColor, RING_MIN * 2);
			}
			else
			{
				Paint(w, icon, faction);
			}

			RZ_UIW.SetText(w, "Label", RZ_UIText.Str(f, 5));
			m_aIcons.Insert(w);
			m_aX.Insert(RZ_UIText.Int(f, 3));
			m_aZ.Insert(RZ_UIText.Int(f, 4));
			m_aRadius.Insert(RZ_UIText.Int(f, 6));
			m_aLabels.Insert(RZ_UIText.Str(f, 5));
		}

		MergeLabels();
		MakeSelf();	// made again after the icons, so it's drawn above them
	}

	// ================================================================ The marker
	//------------------------------------------------------------------------------------------------
	//! Paints one marker in `root` (a map icon, or one of the key's): Ring near-black, Disc in the
	//! owner's colour, Icon the faction's emblem (or the place's glyph) on it, and the badge, if the
	//! root has one, shown only for a faction's place.
	protected static void Paint(Widget root, string icon, int faction)
	{
		RZ_UIW.Icon(root, "Ring", "marker_ring");
		RZ_UIW.Tint(root, "Ring", RingColor());
		RZ_UIW.Icon(root, "Disc", "marker_disc");
		RZ_UIW.Tint(root, "Disc", DiscColor(icon, faction));
		RZ_UIW.Icon(root, "Icon", Glyph(icon, faction));
		RZ_UIW.Tint(root, "Icon", GlyphColor(icon, faction));

		string badge = "";
		if (faction >= 0)
			badge = Badge(icon);

		PaintBadge(root, badge);
	}

	//------------------------------------------------------------------------------------------------
	//! The badge in `root` (BadgeBack and Badge): a dark chip with the bone glyph; hidden when "".
	protected static void PaintBadge(Widget root, string badge)
	{
		bool shown = !badge.IsEmpty();
		RZ_UIW.Show(root, "BadgeBack", shown);
		RZ_UIW.Show(root, "Badge", shown);
		if (!shown)
			return;

		RZ_UIW.Icon(root, "BadgeBack", "marker_disc");
		RZ_UIW.Tint(root, "BadgeBack", RingColor());
		RZ_UIW.Icon(root, "Badge", badge);
		RZ_UIW.Tint(root, "Badge", RZ_UIColor.Bone());
	}

	//------------------------------------------------------------------------------------------------
	//! One image alone, `size` across from the root's top left (a range ring, you-are-here): the
	//! disc, ring and badge hidden.
	protected static void Solo(Widget root, string icon, Color c, float size)
	{
		RZ_UIW.Show(root, "Ring", false);
		RZ_UIW.Show(root, "Disc", false);
		RZ_UIW.Show(root, "BadgeBack", false);
		RZ_UIW.Show(root, "Badge", false);
		Widget img = RZ_UIW.Find(root, "Icon");
		RZ_UIW.SetPos(img, 0, 0);
		RZ_UIW.SetSize(img, size, size);
		RZ_UIW.SetSize(root, size, size);
		RZ_UIW.Icon(root, "Icon", icon);
		RZ_UIW.Tint(root, "Icon", c);
	}

	//------------------------------------------------------------------------------------------------
	//! The ring round every marker and the badges' chip: near-black (#121416).
	protected static Color RingColor()
	{
		return RZ_UIW.Rgb(18, 20, 22);
	}

	//------------------------------------------------------------------------------------------------
	//! Whose it is: the faction's colour; hazard yellow for events and checkpoints; steel grey for the
	//! stations (dark enough for the bone glyph to read on it).
	protected static Color DiscColor(string icon, int faction)
	{
		if (faction >= 0)
			return RZ_UIColor.Faction(faction);

		if (icon == "marker_event" || icon == "marker_checkpoint")
			return RZ_UIColor.Hazard();

		return RZ_UIW.Rgb(78, 84, 89);
	}

	//------------------------------------------------------------------------------------------------
	//! Bone on the disc, except near-black on hazard yellow, where bone wouldn't read.
	protected static Color GlyphColor(string icon, int faction)
	{
		if (faction < 0 && (icon == "marker_event" || icon == "marker_checkpoint"))
			return RingColor();

		return RZ_UIColor.Bone();
	}

	//------------------------------------------------------------------------------------------------
	//! On the disc: a faction's emblem; for a place without one, its glyph (no plate).
	protected static string Glyph(string icon, int faction)
	{
		if (faction >= 0)
			return RZ_UIText.FactionIcon(faction);

		switch (icon)
		{
			case "marker_power": return "glyph_power";
			case "marker_radio": return "glyph_radio";
			case "marker_event": return "glyph_event";
			case "marker_checkpoint": return "glyph_checkpoint";
		}

		return icon;
	}

	//------------------------------------------------------------------------------------------------
	//! What a faction's place is, "" for none.
	protected static string Badge(string icon)
	{
		switch (icon)
		{
			case "marker_stronghold": return "badge_stronghold";
			case "marker_outpost": return "badge_outpost";
			case "marker_safehouse": return "badge_safehouse";
			case "marker_lab": return "badge_lab";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Markers within LABEL_MERGE m of an earlier one (a stronghold, its quartermaster and a job at the
	//! same house) share one label, one line each, on the first marker: drawn apart they lay their
	//! labels on top of each other and none could be read.
	protected void MergeLabels()
	{
		array<string> merged = {};
		foreach (string t : m_aLabels)
		{
			merged.Insert(t);
		}

		for (int i = 1; i < m_aIcons.Count(); i++)
		{
			for (int j = 0; j < i; j++)
			{
				float dx = m_aX[i] - m_aX[j];
				float dz = m_aZ[i] - m_aZ[j];
				if (dx * dx + dz * dz > LABEL_MERGE * LABEL_MERGE || m_aRadius[i] > 0 || m_aRadius[j] > 0)
					continue;

				if (!m_aLabels[i].IsEmpty() && !merged[j].Contains(m_aLabels[i]))
				{
					if (merged[j].IsEmpty())
						merged[j] = m_aLabels[i];
					else
						merged[j] = merged[j] + "\n" + m_aLabels[i];
				}

				merged[i] = "";
				break;
			}
		}

		foreach (int k, Widget w : m_aIcons)
		{
			RZ_UIW.SetText(w, "Label", merged[k]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame while the map is open: the markers follow pan and zoom; a range ring is sized to its
	//! radius on the map (48 to 320 units across), its label kept centred under it.
	protected void Place()
	{
		foreach (int i, Widget w : m_aIcons)
		{
			float sx, sy;
			if (!RZ_UIW.MapToScreen(m_MapEntity, m_aX[i], m_aZ[i], sx, sy))
				continue;

			float half = HALF;
			if (m_aRadius[i] > 0)
			{
				half = RING_MIN;
				float ex, ey;
				if (RZ_UIW.MapToScreen(m_MapEntity, m_aX[i] + m_aRadius[i], m_aZ[i], ex, ey))
					half = Math.Clamp(Math.AbsFloat(ex - sx), RING_MIN, RING_MAX);

				RZ_UIW.SetSize(w, half * 2, half * 2);
				RZ_UIW.SetSize(RZ_UIW.Find(w, "Icon"), half * 2, half * 2);
				RZ_UIW.SetPos(RZ_UIW.Find(w, "Label"), half - LABEL_W / 2, half * 2 + 4);
			}

			RZ_UIW.SetPos(w, sx - half, sy - half);
		}
	}

	// ================================================================ You are here
	//------------------------------------------------------------------------------------------------
	//! Two map icons from UI > Map icon layout, 44 across: marker_self_back tinted dark, and the
	//! marker_self arrow in hazard yellow on it. Nothing without the layout.
	protected void MakeSelf()
	{
		RemoveSelf();
		RZ_UISettings settings = RZ_UISettings.Get();
		if (!m_RootWidget || !settings.UsesMapIcons())
			return;

		m_wSelfBack = SelfIcon(settings.m_sMapIconLayout, "marker_self_back", Color.FromRGBA(22, 24, 27, 225));
		m_wSelf = SelfIcon(settings.m_sMapIconLayout, "marker_self", RZ_UIColor.Hazard());
		PlaceSelf();
	}

	//------------------------------------------------------------------------------------------------
	protected Widget SelfIcon(ResourceName layout, string icon, Color c)
	{
		Widget w = RZ_UIW.Create(layout, m_RootWidget);
		if (!w)
			return null;

		Solo(w, icon, c, SELF_HALF * 2);
		RZ_UIW.SetText(w, "Label", "");
		return w;
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveSelf()
	{
		RZ_UIW.Remove(m_wSelfBack);
		RZ_UIW.Remove(m_wSelf);
		m_wSelfBack = null;
		m_wSelf = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame: on the player's own spot, the arrow turned to where they face. Hidden while
	//! there's no controlled entity (dead, spectating).
	protected void PlaceSelf()
	{
		if (!m_wSelf)
			return;

		IEntity me = RZ_UIW.LocalEntity();
		float sx, sy;
		bool shown = false;
		if (me)
		{
			vector at = me.GetOrigin();
			shown = RZ_UIW.MapToScreen(m_MapEntity, at[0], at[2], sx, sy);
		}

		m_wSelf.SetVisible(shown);
		if (m_wSelfBack)
			m_wSelfBack.SetVisible(shown);

		if (!shown)
			return;

		RZ_UIW.SetPos(m_wSelfBack, sx - SELF_HALF, sy - SELF_HALF);
		RZ_UIW.SetPos(m_wSelf, sx - SELF_HALF, sy - SELF_HALF);
		RZ_UIW.Rotate(m_wSelf, "Icon", RZ_UIW.Heading(me));
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

		// Drawn as the map draws them: the four factions' discs, the four place badges, then the world
		Paint(RZ_UIW.Find(m_wLegend, "Key_haven"), "", RZ_EFaction.SURVIVORS);
		Paint(RZ_UIW.Find(m_wLegend, "Key_bastion"), "", RZ_EFaction.MILITARY);
		Paint(RZ_UIW.Find(m_wLegend, "Key_redhand"), "", RZ_EFaction.BANDITS);
		Paint(RZ_UIW.Find(m_wLegend, "Key_halcyon"), "", RZ_EFaction.INSTITUTE);
		PaintBadge(RZ_UIW.Find(m_wLegend, "Badge_stronghold"), "badge_stronghold");
		PaintBadge(RZ_UIW.Find(m_wLegend, "Badge_outpost"), "badge_outpost");
		PaintBadge(RZ_UIW.Find(m_wLegend, "Badge_safehouse"), "badge_safehouse");
		PaintBadge(RZ_UIW.Find(m_wLegend, "Badge_lab"), "badge_lab");
		Paint(RZ_UIW.Find(m_wLegend, "Key_event"), "marker_event", -1);
		Paint(RZ_UIW.Find(m_wLegend, "Key_checkpoint"), "marker_checkpoint", -1);
		Paint(RZ_UIW.Find(m_wLegend, "Key_power"), "marker_power", -1);
		Paint(RZ_UIW.Find(m_wLegend, "Key_radio"), "marker_radio", -1);
		Widget rangeKey = RZ_UIW.Find(m_wLegend, "Key_range");
		RZ_UIW.Icon(rangeKey, "Icon", "marker_range");
		RZ_UIW.Tint(rangeKey, "Icon", RZ_UIColor.Hazard());
		bool prompt = RZ_UIW.AddPrompt(m_wLegend, "LegendPrompt", LEGEND_ACTION, "Hide the key");
		RZ_UIW.Show(m_wLegend, "LegendPromptText", !prompt);
		// VERIFY: RZ_MapLegend (MapContext) fires while the full-screen map is open, and pad Y there
		// does nothing else (vanilla binds Y in the map only with D-pad left, held, to dismiss a hint)
		GetGame().GetInputManager().AddActionListener(LEGEND_ACTION, EActionTrigger.DOWN, OnToggleLegend);
		FadeLegend(settings);
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
