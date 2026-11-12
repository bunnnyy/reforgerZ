//! A picture of an item on a screen: vanilla's item preview of its prefab (as the inventory draws
//! items), made at run time inside a frame of the layout, and the item's own description and
//! weight from the same preview copy. Client only. Used by the crafting screen (RZ_CraftMenu.c).

//------------------------------------------------------------------------------------------------
class RZ_ItemPreview
{
	protected ItemPreviewWidget m_wPreview;
	protected ResourceName m_sShown;

	//------------------------------------------------------------------------------------------------
	//! Makes the preview widget inside `slot`, filling it. False when it can't (no slot).
	bool Make(Widget slot)
	{
		if (!slot)
			return false;

		Widget w = GetGame().GetWorkspace().CreateWidget(GameWidgetType.ItemPreviewWidgetTypeID, WidgetFlags.VISIBLE, Color.FromRGBA(255, 255, 255, 255), 0, slot);
		m_wPreview = ItemPreviewWidget.Cast(w);
		if (!m_wPreview)
			return false;

		FrameSlot.SetAnchorMin(m_wPreview, 0, 0);
		FrameSlot.SetAnchorMax(m_wPreview, 1, 1);
		FrameSlot.SetOffsets(m_wPreview, 0, 0, 0, 0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the prefab's picture ("" hides it). The same prefab again changes nothing.
	void Show(ResourceName prefab)
	{
		if (!m_wPreview)
			return;

		m_wPreview.SetVisible(!prefab.IsEmpty());
		if (prefab.IsEmpty() || prefab == m_sShown)
			return;

		ItemPreviewManagerEntity manager = Manager();
		if (!manager)
			return;

		m_sShown = prefab;
		PreviewRenderAttributes attrs;
		InventoryItemComponent item = Item(prefab);
		if (item)
			attrs = PreviewRenderAttributes.Cast(item.FindAttribute(PreviewRenderAttributes));

		manager.SetPreviewItemFromPrefab(m_wPreview, prefab, attrs, true);
	}

	//------------------------------------------------------------------------------------------------
	static ItemPreviewManagerEntity Manager()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return null;

		return world.GetItemPreviewManager();
	}

	//------------------------------------------------------------------------------------------------
	//! The item component of the prefab's preview copy, or null.
	//! VERIFY: ResolvePreviewEntityForPrefab gives the crafting screen a copy of the output carrying
	//! the prefab's InventoryItemComponent, with its UI info, weight and preview attributes (as the
	//! shop screen's RZ_ShopMenu.PreviewItem).
	static InventoryItemComponent Item(ResourceName prefab)
	{
		ItemPreviewManagerEntity manager = Manager();
		if (!manager || prefab.IsEmpty())
			return null;

		IEntity e = manager.ResolvePreviewEntityForPrefab(prefab);
		if (!e)
			return null;

		return InventoryItemComponent.Cast(e.FindComponent(InventoryItemComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! The item's own description and its weight ("" when it has neither).
	static string Describe(ResourceName prefab)
	{
		InventoryItemComponent item = Item(prefab);
		if (!item)
			return "";

		string text = "";
		UIInfo info = item.GetUIInfo();
		if (info)
			text = info.GetDescription();

		int tenths = RZ_Util.RoundToInt(item.GetTotalWeight() * 10);
		if (tenths > 0)
		{
			if (!text.IsEmpty())
				text += "\n";

			text += string.Format("%1.%2 kg", tenths / 10, tenths % 10);
		}

		return text;
	}
}
