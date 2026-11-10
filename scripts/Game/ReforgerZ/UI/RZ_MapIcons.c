//! The map's icon layer (docs/UI.md, "The map"): the ReforgerZ marker icons on the real map, for the
//! things this player is allowed to know. The server works out the list (RZ_UIFeed.BuildMap: the
//! lab, safehouses unless exiled, strongholds, outposts they know or have seen, the power and radio
//! stations, the sealed checkpoints, world events; never contracts, which give a rough area only)
//! and sends it to this player alone when the map opens, then every 20 s while it's open.
//!
//! Add RZ_MapIconsUI to the components of the map configuration the game uses (the full-screen map
//! config; WORKBENCH_SETUP.md, "Setting up the UI") and set UI > Map icon layout. Without the
//! layout, the layer does nothing and the vanilla text markers stay.
class RZ_MapIconsUI : SCR_MapUIBaseComponent
{
	protected static const float HALF = 14;	// half the icon's size, to centre it on the spot

	protected ref array<Widget> m_aIcons = {};
	protected ref array<float> m_aX = {};
	protected ref array<float> m_aZ = {};
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
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		m_bOpen = false;
		Clear();
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
	}

	//------------------------------------------------------------------------------------------------
	//! P|icon|faction|x|z|label
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
			else if (icon == "marker_event" || icon == "marker_checkpoint")
				c = RZ_UIColor.Hazard();

			RZ_UIW.Icon(w, "Icon", icon);
			RZ_UIW.Tint(w, "Icon", c);
			RZ_UIW.SetText(w, "Label", RZ_UIText.Str(f, 5));
			m_aIcons.Insert(w);
			m_aX.Insert(RZ_UIText.Int(f, 3));
			m_aZ.Insert(RZ_UIText.Int(f, 4));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame while the map is open: the icons follow pan and zoom.
	protected void Place()
	{
		foreach (int i, Widget w : m_aIcons)
		{
			float sx, sy;
			if (RZ_UIW.MapToScreen(m_MapEntity, m_aX[i], m_aZ[i], sx, sy))
				RZ_UIW.SetPos(w, sx - HALF, sy - HALF);
		}
	}
}
