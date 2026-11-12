//! The status strip (docs/UI.md, "The status strip and locked rows"), client side: up to four
//! readout tiles under a screen's header, shared by the object menu and the cooking screen. Each
//! of their layouts has a StatusStrip frame with Tile0 to Tile3, and each tile TileNLabel,
//! TileNLamp, TileNValue and TileNBar (UI/LAYOUTS.md). The server sends one S record per tile
//! (Interactions/RZ_ObjectStatus.c): S|label|value|state|left.
//!
//! Also the footer's A prompt both screens relabel and dim (a locked row, a recipe you're short for).

//------------------------------------------------------------------------------------------------
class RZ_StatusStrip
{
	//------------------------------------------------------------------------------------------------
	//! Draws the S records into the tiles and hides the ones left over; the strip hides with none.
	//! `elapsed`: seconds since the data came, for a value that counts down ("BOILED WATER · 12 S").
	static void Draw(Widget root, notnull array<ref array<string>> tiles, float elapsed)
	{
		RZ_UIW.Show(root, "StatusStrip", !tiles.IsEmpty());
		for (int i = 0; i < RZ_ObjectStatus.MAX_TILES; i++)
		{
			string n = "Tile" + i.ToString();
			if (i >= tiles.Count())
			{
				RZ_UIW.Show(root, n, false);
				continue;
			}

			array<string> f = tiles[i];
			int state = RZ_UIText.Int(f, 3);
			int left = RZ_UIText.Int(f, 4);
			string value = RZ_UIText.Str(f, 2);
			if (left > 0)
				value += string.Format(" · %1 S", Math.Max(0, RZ_Util.RoundToInt(Math.Ceil(left - elapsed))));

			RZ_UIW.Show(root, n, true);
			RZ_UIW.SetText(root, n + "Label", RZ_UIText.Str(f, 1));
			RZ_UIW.SetText(root, n + "Value", value);
			RZ_UIW.Tint(root, n + "Value", ValueColour(state));
			RZ_UIW.Tint(root, n + "Lamp", StateColour(state));
			RZ_UIW.Tint(root, n + "Bar", StateColour(state));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A tile counts down: the screen redraws once a second.
	static bool Counting(notnull array<ref array<string>> tiles)
	{
		foreach (array<string> f : tiles)
		{
			if (RZ_UIText.Int(f, 4) > 0)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The lamp and the bar: olive on, red off or blocked, amber partly or under way, faint unknown.
	static Color StateColour(int state)
	{
		switch (state)
		{
			case RZ_ObjectStatus.STATE_OK: return RZ_UIW.Rgb(154, 163, 94);		// #9AA35E
			case RZ_ObjectStatus.STATE_OFF: return RZ_UIW.Rgb(217, 87, 76);		// #D9574C
			case RZ_ObjectStatus.STATE_PART: return RZ_UIW.Rgb(232, 178, 58);	// #E8B23A
		}

		return RZ_UIW.Rgb(110, 106, 96);	// #6E6A60
	}

	//------------------------------------------------------------------------------------------------
	//! The value's text, a shade lighter than its lamp.
	static Color ValueColour(int state)
	{
		switch (state)
		{
			case RZ_ObjectStatus.STATE_OK: return RZ_UIW.Rgb(196, 204, 138);		// #C4CC8A
			case RZ_ObjectStatus.STATE_OFF: return RZ_UIW.Rgb(240, 138, 128);	// #F08A80
			case RZ_ObjectStatus.STATE_PART: return RZ_UIW.Rgb(232, 178, 58);	// #E8B23A
		}

		return RZ_UIColor.Dim();
	}

	//------------------------------------------------------------------------------------------------
	//! A footer prompt, as RZ_UIW.AddPrompt makes it, kept so the screen can relabel and dim it.
	//! Null when it can't be made (the static footer shows instead).
	static Widget MakePrompt(Widget root, string parentName, string action, string label)
	{
		Widget parent = RZ_UIW.Find(root, parentName);
		if (!parent)
			return null;

		Widget w = GetGame().GetWorkspace().CreateWidgets(RZ_UIW.PROMPT_LAYOUT, parent);
		if (!w)
			return null;

		SCR_InputButtonComponent btn = SCR_InputButtonComponent.FindComponent(w);
		if (!btn)
		{
			w.RemoveFromHierarchy();
			return null;
		}

		btn.SetAction(action);
		btn.SetLabel(label);
		return w;
	}

	//------------------------------------------------------------------------------------------------
	//! Relabels a prompt from MakePrompt ("Boil", "Do it") and dims it when the pick can't go.
	static void SetPrompt(Widget prompt, string label, bool enabled)
	{
		if (!prompt)
			return;

		SCR_InputButtonComponent btn = SCR_InputButtonComponent.FindComponent(prompt);
		if (btn && !label.IsEmpty())
			btn.SetLabel(label);

		float alpha = 1;
		if (!enabled)
			alpha = 0.4;

		prompt.SetOpacity(alpha);
	}
}
