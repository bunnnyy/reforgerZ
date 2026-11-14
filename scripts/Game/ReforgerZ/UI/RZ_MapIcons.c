//! The map's icon layer (docs/UI.md, "The map"): the ReforgerZ marker icons on the real map, for the
//! things this player is allowed to know. The server works out the list (RZ_UIFeed.BuildMap: the
//! lab, safehouses unless exiled, strongholds, outposts they know or have seen, the power and radio
//! stations, the sealed checkpoints, the packs they know of, and of the jobs only the tracked one's
//! current objective) and sends it to this player alone when the map opens, then every 20 s while
//! it's open.
//!
//! Each place is a marker (design A): a solid disc in its owner's colour with the faction's emblem on it
//! in bone, a thin near-black ring round it with a small gap, and at the bottom right a small badge for
//! what the place is (stronghold, outpost, safehouse, the lab). Places without a faction (the stations,
//! checkpoints) get the same disc and ring in their own colour with their glyph, no badge.
//! A pack's range ring stays a ring sized to its ground.
//!
//! The tracked job's current objective: on a known spot, an amber diamond with a near-black outline
//! and a dark centre dot, labelled with the objective; while they know only the rough area, the
//! search area, a soft amber fill with a thin solid amber edge, drawn on a canvas so the edge stays
//! thin at every zoom, labelled under it.
//!
//! You are here: the local player's own marker, an arrow on a dark disc turned to where they face,
//! drawn above the other icons and moved every frame while the map is open. It's client-side only,
//! from the controlled entity; nothing is sent. It shows with or without vanilla's markers.
//!
//! The map key sits on the map too: a compact legend in the bottom-left (UI > Map legend layout),
//! shown while the map is zoomed out and faded as you zoom in. RZ_MapLegend (I, pad Y) shows or
//! hides it; the choice lasts the session.
//!
//! The map's fog (docs/UI.md, "The map's fog") is drawn first, under everything, by RZ_MapFogDraw;
//! it needs no layout, only this component on the map.
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
	protected static const float AREA_MIN = 24;	// the search area is at least 48 across on screen
	protected static const int AREA_SIDES = 48;	// the search area's circle, as a polygon
	protected static const float AREA_EDGE = 2;	// its edge's width
	protected static const string OBJ_SPOT = "objective_spot";	// the feed's names for the objective
	protected static const string OBJ_AREA = "objective_area";
	protected static const string LEGEND_ACTION = "RZ_MapLegend";

	protected static bool s_bLegendHidden;	// the player hid the legend (this session)

	protected Widget m_wLegend;
	protected Widget m_wSelfBack;	// you are here: the dark disc
	protected Widget m_wSelf;		// and the arrow on it
	protected CanvasWidget m_wCanvas;	// the search area is drawn here
	protected ref array<ref CanvasWidgetCommand> m_aDraw = {};
	protected bool m_bDrawn;		// the canvas has something on it
	protected int m_iSpot = -1;		// the objective's diamond among m_aIcons, -1 none
	protected int m_iArea = -1;		// the search area's label among m_aIcons, -1 none

	protected ref array<Widget> m_aIcons = {};
	protected ref array<float> m_aX = {};
	protected ref array<float> m_aZ = {};
	protected ref array<float> m_aRadius = {};	// a range ring's radius in metres (0: a fixed-size icon)
	protected ref array<string> m_aLabels = {};
	protected int m_iSeen = -1;
	protected float m_fAsk;
	protected bool m_bOpen;
	protected ref RZ_MapFogDraw m_Fog = new RZ_MapFogDraw();	// the map's fog, under everything else (UI/RZ_MapFogDraw.c)

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		m_bOpen = true;
		m_iSeen = -1;
		m_fAsk = 0;
		// The fog first, so the markers draw above it; it doesn't need the icon layout
		if (m_RootWidget)
		{
			m_Fog.Make(m_RootWidget);
			RZ_PlayerUIComponent fogUi = RZ_PlayerUIComponent.GetLocal();
			if (fogUi)
				fogUi.Ask(RZ_UIFeed.PAYLOAD_FOG);
		}

		MakeCanvas();
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
		RemoveCanvas();
		m_Fog.Remove();
		super.OnMapClose(config);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		if (m_bOpen)
			m_Fog.Update(m_MapEntity);

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
		m_iSpot = -1;
		m_iArea = -1;
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
			if (icon == OBJ_SPOT)
			{
				m_iSpot = m_aIcons.Count();
				PaintObjective(w);
			}
			else if (icon == OBJ_AREA)
			{
				// The search area: the canvas draws it (DrawArea); this widget holds its label only
				m_iArea = m_aIcons.Count();
				Bare(w);
			}
			else if (RZ_UIText.Int(f, 6) > 0)
			{
				// A pack's range ring: the ring alone, in hazard yellow
				Solo(w, icon, RZ_UIColor.Hazard(), RING_MIN * 2);
			}
			else
			{
				Paint(w, icon, faction);
			}

			RZ_UIW.SetText(w, "Label", RZ_UIText.Str(f, 5));
			RZ_UIW.Tint(w, "Label", Color.FromRGBA(0, 0, 0, 255));	// black reads on the light map
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
		Bare(root);
		RZ_UIW.Show(root, "Icon", true);
		Widget img = RZ_UIW.Find(root, "Icon");
		RZ_UIW.SetPos(img, 0, 0);
		RZ_UIW.SetSize(img, size, size);
		RZ_UIW.SetSize(root, size, size);
		RZ_UIW.Icon(root, "Icon", icon);
		RZ_UIW.Tint(root, "Icon", c);
	}

	//------------------------------------------------------------------------------------------------
	//! Every image in `root` hidden: the label alone (the search area's).
	protected static void Bare(Widget root)
	{
		RZ_UIW.Show(root, "Ring", false);
		RZ_UIW.Show(root, "Disc", false);
		RZ_UIW.Show(root, "Icon", false);
		RZ_UIW.Show(root, "BadgeBack", false);
		RZ_UIW.Show(root, "Badge", false);
	}

	//------------------------------------------------------------------------------------------------
	//! The objective's diamond in `root` (a map icon or the key's): Ring a near-black diamond
	//! (track_current, 44 box) as the outline, Disc the amber diamond on it (36 box), Icon a dark
	//! centre dot (marker_disc, 8); the badge hidden. Centred on the 34 frame like every marker.
	protected static void PaintObjective(Widget root)
	{
		RZ_UIW.Show(root, "BadgeBack", false);
		RZ_UIW.Show(root, "Badge", false);
		Fit(root, "Ring", -5, 44);
		RZ_UIW.Icon(root, "Ring", "track_current");
		RZ_UIW.Tint(root, "Ring", RingColor());
		Fit(root, "Disc", -1, 36);
		RZ_UIW.Icon(root, "Disc", "track_current");
		RZ_UIW.Tint(root, "Disc", RZ_UIColor.Objective());
		Fit(root, "Icon", 13, 8);
		RZ_UIW.Icon(root, "Icon", "marker_disc");
		RZ_UIW.Tint(root, "Icon", RingColor());
	}

	//------------------------------------------------------------------------------------------------
	//! The search area's swatch in the key: Fill (marker_disc) in the area's soft amber, Edge
	//! (marker_ring) in solid amber.
	protected static void PaintArea(Widget root)
	{
		RZ_UIW.Icon(root, "Fill", "marker_disc");
		RZ_UIW.Tint(root, "Fill", AreaFill());
		RZ_UIW.Icon(root, "Edge", "marker_ring");
		RZ_UIW.Tint(root, "Edge", RZ_UIColor.Objective());
	}

	//------------------------------------------------------------------------------------------------
	//! A named image in `root` moved to (at, at) and sized `size` square.
	protected static void Fit(Widget root, string name, float at, float size)
	{
		Widget img = RZ_UIW.Find(root, name);
		RZ_UIW.SetPos(img, at, at);
		RZ_UIW.SetSize(img, size, size);
	}

	//------------------------------------------------------------------------------------------------
	//! The search area's fill: the objective's amber at about 14%.
	protected static Color AreaFill()
	{
		return Color.FromRGBA(232, 178, 58, 36);
	}

	//------------------------------------------------------------------------------------------------
	//! The ring round every marker and the badges' chip: near-black (#121416).
	protected static Color RingColor()
	{
		return RZ_UIW.Rgb(18, 20, 22);
	}

	//------------------------------------------------------------------------------------------------
	//! Whose it is: the faction's colour; hazard yellow for the checkpoints; steel grey for the
	//! stations (dark enough for the bone glyph to read on it).
	protected static Color DiscColor(string icon, int faction)
	{
		if (faction >= 0)
			return RZ_UIColor.Faction(faction);

		if (icon == "marker_checkpoint")
			return RZ_UIColor.Hazard();

		return RZ_UIW.Rgb(78, 84, 89);
	}

	//------------------------------------------------------------------------------------------------
	//! Bone on the disc, except near-black on hazard yellow, where bone wouldn't read.
	protected static Color GlyphColor(string icon, int faction)
	{
		if (faction < 0 && icon == "marker_checkpoint")
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
	//! labels on top of each other and none could be read. A place at the objective's diamond joins
	//! its label instead, under the objective's line, wherever the diamond is in the list (it comes
	//! last, to be drawn on top). Range rings and the search area keep their own.
	protected void MergeLabels()
	{
		array<string> merged = {};
		array<int> owner = {};
		foreach (int n, string t : m_aLabels)
		{
			merged.Insert(t);
			owner.Insert(n);
		}

		for (int i = 0; i < m_aIcons.Count(); i++)
		{
			if (i == m_iSpot || m_aRadius[i] > 0)
				continue;

			int into = -1;
			if (m_iSpot >= 0 && Near(i, m_iSpot))
			{
				into = m_iSpot;
			}
			else
			{
				for (int j = 0; j < i; j++)
				{
					if (j != m_iSpot && m_aRadius[j] <= 0 && Near(i, j))
					{
						into = owner[j];
						break;
					}
				}
			}

			if (into < 0)
				continue;

			owner[i] = into;
			if (!m_aLabels[i].IsEmpty() && !merged[into].Contains(m_aLabels[i]))
			{
				if (merged[into].IsEmpty())
					merged[into] = m_aLabels[i];
				else
					merged[into] = merged[into] + "\n" + m_aLabels[i];
			}

			merged[i] = "";
		}

		foreach (int k, Widget w : m_aIcons)
		{
			RZ_UIW.SetText(w, "Label", merged[k]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Two markers within LABEL_MERGE m of each other.
	protected bool Near(int a, int b)
	{
		float dx = m_aX[a] - m_aX[b];
		float dz = m_aZ[a] - m_aZ[b];
		return dx * dx + dz * dz <= LABEL_MERGE * LABEL_MERGE;
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame while the map is open: the markers follow pan and zoom; a range ring is sized to its
	//! radius on the map (48 to 320 units across), its label kept centred under it; the search area
	//! is its true size on the map (at least 48 across), drawn on the canvas, its label under it.
	protected void Place()
	{
		bool drew = false;
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
					half = Math.AbsFloat(ex - sx);

				if (i == m_iArea)
				{
					half = Math.Max(half, AREA_MIN);
					DrawArea(sx, sy, half);
					drew = true;
				}
				else
				{
					half = Math.Clamp(half, RING_MIN, RING_MAX);
					RZ_UIW.SetSize(RZ_UIW.Find(w, "Icon"), half * 2, half * 2);
				}

				RZ_UIW.SetSize(w, half * 2, half * 2);
				RZ_UIW.SetPos(RZ_UIW.Find(w, "Label"), half - LABEL_W / 2, half * 2 + 4);
			}

			RZ_UIW.SetPos(w, sx - half, sy - half);
		}

		if (!drew && m_bDrawn)
		{
			m_aDraw.Clear();
			RZ_UIW.Draw(m_wCanvas, m_aDraw);
			m_bDrawn = false;
		}
	}

	// ================================================================ The search area
	//------------------------------------------------------------------------------------------------
	//! The canvas under the markers (made before them), filling the map. Nothing without the icon
	//! layout.
	protected void MakeCanvas()
	{
		RemoveCanvas();
		if (m_RootWidget && RZ_UISettings.Get().UsesMapIcons())
			m_wCanvas = RZ_UIW.MakeCanvas(m_RootWidget);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveCanvas()
	{
		RZ_UIW.Remove(m_wCanvas);
		m_wCanvas = null;
		m_aDraw.Clear();
		m_bDrawn = false;
	}

	//------------------------------------------------------------------------------------------------
	//! The search area round (sx, sy), `radius` across in layout units: a polygon filled in the soft
	//! amber, and its edge a solid amber line AREA_EDGE wide at every zoom.
	protected void DrawArea(float sx, float sy, float radius)
	{
		if (!m_wCanvas)
			return;

		float cx = RZ_UIW.Native(sx);
		float cy = RZ_UIW.Native(sy);
		float r = RZ_UIW.Native(radius);
		array<float> points = {};
		for (int p = 0; p < AREA_SIDES; p++)
		{
			float a = Math.PI2 * p / AREA_SIDES;
			points.Insert(cx + r * Math.Cos(a));
			points.Insert(cy + r * Math.Sin(a));
		}

		PolygonDrawCommand fill = new PolygonDrawCommand();
		fill.m_iColor = AreaFill().PackToInt();
		fill.m_Vertices = points;

		LineDrawCommand edge = new LineDrawCommand();
		edge.m_iColor = RZ_UIColor.Objective().PackToInt();
		edge.m_fWidth = RZ_UIW.Native(AREA_EDGE);
		edge.m_Vertices = points;
		edge.m_bShouldEnclose = true;

		m_aDraw.Clear();
		m_aDraw.Insert(fill);
		m_aDraw.Insert(edge);
		RZ_UIW.Draw(m_wCanvas, m_aDraw);
		m_bDrawn = true;
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

		// Drawn as the map draws them: the four factions' discs, the four place badges, the world, then
		// your objective and a search area
		Paint(RZ_UIW.Find(m_wLegend, "Key_haven"), "", RZ_EFaction.SURVIVORS);
		Paint(RZ_UIW.Find(m_wLegend, "Key_bastion"), "", RZ_EFaction.MILITARY);
		Paint(RZ_UIW.Find(m_wLegend, "Key_redhand"), "", RZ_EFaction.BANDITS);
		Paint(RZ_UIW.Find(m_wLegend, "Key_halcyon"), "", RZ_EFaction.INSTITUTE);
		PaintBadge(RZ_UIW.Find(m_wLegend, "Badge_stronghold"), "badge_stronghold");
		PaintBadge(RZ_UIW.Find(m_wLegend, "Badge_outpost"), "badge_outpost");
		PaintBadge(RZ_UIW.Find(m_wLegend, "Badge_safehouse"), "badge_safehouse");
		PaintBadge(RZ_UIW.Find(m_wLegend, "Badge_lab"), "badge_lab");
		Paint(RZ_UIW.Find(m_wLegend, "Key_checkpoint"), "marker_checkpoint", -1);
		Paint(RZ_UIW.Find(m_wLegend, "Key_power"), "marker_power", -1);
		Paint(RZ_UIW.Find(m_wLegend, "Key_radio"), "marker_radio", -1);
		Widget rangeKey = RZ_UIW.Find(m_wLegend, "Key_range");
		RZ_UIW.Icon(rangeKey, "Icon", "marker_range");
		RZ_UIW.Tint(rangeKey, "Icon", RZ_UIColor.Hazard());
		PaintObjective(RZ_UIW.Find(m_wLegend, "Key_objective"));
		PaintArea(RZ_UIW.Find(m_wLegend, "Key_area"));
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
