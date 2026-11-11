//! The pause menu's Notebook button (docs/UI.md, "The notebook"): the pad's way into the notebook
//! (Start, then Notebook). No layout override is needed: when vanilla's pause menu opens, a button
//! named RZ_Notebook is used if the layout has one, else one is made from vanilla's own pause
//! menu button layout and put in the same column, just after the Field Manual button. A click (or
//! A) closes the pause menu and opens the notebook. Outside a ReforgerZ game nothing happens.

modded class PauseMenuUI
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		RZ_PauseNotebook.Attach(this, GetRootWidget());
	}
}

//------------------------------------------------------------------------------------------------
class RZ_PauseNotebook
{
	static const string BUTTON = "RZ_Notebook";
	static const ResourceName BUTTON_LAYOUT = "{9ECCD201BCF07E95}UI/layouts/Menus/PauseMenu/PauseMenuButton.layout";

	protected static ref RZ_PauseNotebook s_Instance;
	protected PauseMenuUI m_Menu;

	//------------------------------------------------------------------------------------------------
	static void Attach(PauseMenuUI menu, Widget root)
	{
		if (!root || !RZ_GameModeComponent.GetInstance())
			return;

		SCR_ButtonTextComponent comp = SCR_ButtonTextComponent.GetButtonText(BUTTON, root);
		if (!comp)
			comp = Create(root);

		if (!comp)
			return;

		if (!s_Instance)
			s_Instance = new RZ_PauseNotebook();

		s_Instance.m_Menu = menu;
		comp.m_OnClicked.Insert(s_Instance.OnClicked);
	}

	//------------------------------------------------------------------------------------------------
	//! A button from vanilla's pause menu button layout, after Field Manual (else Settings).
	//! VERIFY: the made button sits in the pad's focus column, labelled Notebook.
	protected static SCR_ButtonTextComponent Create(Widget root)
	{
		Widget anchor = root.FindAnyWidget("FieldManual");
		if (!anchor)
			anchor = root.FindAnyWidget("Settings");

		if (!anchor || !anchor.GetParent())
			return null;

		Widget parent = anchor.GetParent();
		Widget w = GetGame().GetWorkspace().CreateWidgets(BUTTON_LAYOUT, parent);
		if (!w)
			return null;

		w.SetName(BUTTON);
		MoveAfter(w, anchor);
		SCR_ButtonTextComponent comp = SCR_ButtonTextComponent.Cast(w.FindHandler(SCR_ButtonTextComponent));
		if (comp)
			comp.SetText("Notebook");

		return comp;
	}

	//------------------------------------------------------------------------------------------------
	//! Puts `w` (the last child) right after `anchor`: the siblings after the anchor move behind it.
	protected static void MoveAfter(Widget w, Widget anchor)
	{
		Widget parent = anchor.GetParent();
		array<Widget> after = {};
		Widget c = anchor.GetSibling();
		while (c)
		{
			if (c != w)
				after.Insert(c);

			c = c.GetSibling();
		}

		foreach (Widget a : after)
		{
			parent.RemoveChild(a);
			parent.AddChild(a);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClicked()
	{
		if (m_Menu)
			m_Menu.Close();

		// The next frame, once the pause menu has gone
		GetGame().GetCallqueue().CallLater(OpenNotebook, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenNotebook()
	{
		RZ_Notebook.Open(-1);
	}
}
