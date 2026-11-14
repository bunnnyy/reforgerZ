//! The site builder's HUD (docs/SITES.md, "The site builder"; UI/LAYOUTS.md, RZ_SiteHud): while you
//! build a site, a panel top right shows it, its objects (and whether they're saved), the role spots
//! as a checklist with a bar, and the next spot's type; the site builder's screen does the rest. The
//! server sends it every two seconds while you build (World/HandBuilt/RZ_SiteBuilderScreen.c); ""
//! hides it. It takes no input: it can't steal Game Master's or the character's buttons.
class RZ_SiteHud
{
	protected static const ResourceName LAYOUT = "{4382F77E3ACA1D25}UI/layouts/ReforgerZ/RZ_SiteHud.layout";
	protected static const int SPOTS = 16;		// HudSpot0-HudSpot15
	protected static const float BAR = 430;		// HudBarFill's full width

	protected static Widget s_wRoot;

	//------------------------------------------------------------------------------------------------
	static void OnPayload(string payload)
	{
		if (payload.IsEmpty())
		{
			if (s_wRoot)
				s_wRoot.RemoveFromHierarchy();

			s_wRoot = null;
			return;
		}

		if (!s_wRoot)
		{
			WorkspaceWidget ws = GetGame().GetWorkspace();
			if (!ws)
				return;

			s_wRoot = ws.CreateWidgets(LAYOUT);
			if (!s_wRoot)
				return;
		}

		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		array<string> head = null;
		array<ref array<string>> check = {};
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "B")
				head = f;
			else if (tag == "C")
				check.Insert(f);
		}

		if (!head)
			return;

		RZ_UIW.SetText(s_wRoot, "HudHead", RZ_UIText.Str(head, 2));
		RZ_UIW.SetText(s_wRoot, "HudName", RZ_UIText.Clean(RZ_UIText.Str(head, 1), 34));
		string info = string.Format("%1 objects", RZ_UIText.Int(head, 3));
		if (RZ_UIText.Int(head, 4) == 1)
			info += " · unsaved changes";
		else
			info += " · saved";

		if (RZ_UIText.Int(head, 5) > 0)
			info += string.Format(" · %1 outside the ring", RZ_UIText.Int(head, 5));

		RZ_UIW.SetText(s_wRoot, "HudInfo", info);
		int have = RZ_UIText.Int(head, 6);
		int want = Math.Max(1, RZ_UIText.Int(head, 7));
		RZ_UIW.SetText(s_wRoot, "HudSpotsHead", string.Format("ROLE SPOTS · %1 OF %2", have, RZ_UIText.Int(head, 7)));
		Widget fill = RZ_UIW.Find(s_wRoot, "HudBarFill");
		if (fill)
			FrameSlot.SetSizeX(fill, BAR * have / want);

		for (int i = 0; i < SPOTS; i++)
		{
			string idx = i.ToString();
			bool there = i < check.Count();
			RZ_UIW.Show(s_wRoot, "HudSpot" + idx, there);
			if (!there)
				continue;

			array<string> c = check[i];
			bool ok = RZ_UIText.Int(c, 2) == 1;
			string mark = "✗ "; // VERIFY: the HUD's font draws ✓ and ✗ (else they show as boxes)
			Color col = RZ_UIColor.Bad();
			if (ok)
			{
				mark = "✓ ";
				col = RZ_UIColor.Good();
			}

			RZ_UIW.SetText(s_wRoot, "HudSpot" + idx, mark + RZ_UIText.Str(c, 1));
			RZ_UIW.Tint(s_wRoot, "HudSpot" + idx, col);
		}

		RZ_UIW.SetText(s_wRoot, "HudType", "Next spot: " + RZ_UIText.Str(head, 8));
		string hint = "Site builder (/sites): put a spot, save, stop";
		string problem = RZ_UIText.Str(head, 9);
		if (!problem.IsEmpty())
			hint = "Red spot: " + problem;

		RZ_UIW.SetText(s_wRoot, "HudHint", RZ_UIText.Clean(hint, 60));
	}
}
