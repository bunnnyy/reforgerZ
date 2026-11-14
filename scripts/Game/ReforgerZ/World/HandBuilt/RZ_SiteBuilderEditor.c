//! The site builder in Game Master (docs/SITES.md, "Hand-built sites"): a row in Game Master's own
//! context menu (right click on the ground; on a pad, Game Master's action menu), "ReforgerZ: Site
//! builder", that opens the menu; and the camera sent to look at a site when the builder takes you
//! there. The row is added in code to every context-action list (no vanilla config is overridden),
//! last, so every vanilla row keeps its index on the client and the server alike.

//------------------------------------------------------------------------------------------------
//! The context row. Done on the asking machine: it sends the server the "/sites" command, and the
//! server checks the player may build.
class RZ_SiteBuilderEditorAction : SCR_BaseContextAction
{
	protected static ref RZ_SiteBuilderEditorAction s_Instance;		// the lists hold their actions weakly

	//------------------------------------------------------------------------------------------------
	static RZ_SiteBuilderEditorAction Get()
	{
		if (!s_Instance)
		{
			s_Instance = new RZ_SiteBuilderEditorAction();
			s_Instance.m_Info = SCR_UIInfo.CreateInfo("ReforgerZ: Site builder", "Build camp sites by hand and save them.");
			s_Instance.m_iOrder = 1000;
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsEnabled()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! On empty ground (nothing under the cursor), so it never crowds an entity's own rows.
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return !hoveredEntity;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCommand(RZ_PlayerUIComponent.CMD_SITES, "");
	}
}

//------------------------------------------------------------------------------------------------
//! Every context-action list gets the site builder's row, once, at its end.
modded class SCR_BaseActionsEditorComponentClass
{
	protected bool m_bRZSites;

	//------------------------------------------------------------------------------------------------
	protected void RZ_AddSites()
	{
		if (m_bRZSites)
			return;

		m_bRZSites = true;
		if (!Type().IsInherited(SCR_ContextActionsEditorComponentClass) || !m_ActionsSorted)
			return;

		m_ActionsSorted.Insert(RZ_SiteBuilderEditorAction.Get()); // VERIFY: a row added to the class's sorted list shows in Game Master's context menu, and its index is the same on the server
	}

	//------------------------------------------------------------------------------------------------
	override int GetActions(out notnull array<SCR_BaseEditorAction> outActions)
	{
		RZ_AddSites();
		return super.GetActions(outActions);
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAction GetAction(int index)
	{
		RZ_AddSites();
		return super.GetAction(index);
	}

	//------------------------------------------------------------------------------------------------
	override int FindAction(SCR_BaseEditorAction action)
	{
		RZ_AddSites();
		return super.FindAction(action);
	}
}

//------------------------------------------------------------------------------------------------
//! The client end of "Go there": Game Master's camera looks down at the site from its south side.
class RZ_SiteBuilderCamera
{
	//------------------------------------------------------------------------------------------------
	static void OnPayload(string payload)
	{
		array<string> parts = {};
		payload.Split("|", parts, false);
		if (parts.Count() < 4)
			return;

		vector centre = Vector(parts[0].ToFloat(), parts[1].ToFloat(), parts[2].ToFloat());
		float radius = parts[3].ToFloat();
		SCR_ManualCamera cam = SCR_CameraEditorComponent.GetCameraInstance();
		if (!cam)
			return;

		// From the south, out a little past the edge and up, looking at the middle
		vector eye = centre + Vector(0, radius * 0.9, -radius * 1.2);
		vector dir = centre - eye;
		dir.Normalize();
		vector t[4];
		Math3D.DirectionAndUpMatrix(dir, vector.Up, t);
		t[3] = eye;
		cam.SetWorldTransform(t); // VERIFY: setting the manual camera's world transform moves Game Master's camera there (its components may keep their own position)
	}
}

//------------------------------------------------------------------------------------------------
//! The client end of the site's guides while it's built (docs/SITES.md, "Role spots"): its edge as
//! a white ring on the ground, each role spot as a post with a ball on top, green where it's right,
//! red where something's wrong (outside, in a wall, on another spot, in the way in), yellow where a
//! missing spot is suggested (where the automated layout had one). The menu says the same in words.
//! Payload: "R|x|y|z|radius" then ";S|type|x|y|z|state" records (state 0 green, 1 red, 2 yellow).
class RZ_SiteBuilderMarks
{
	protected static ref array<ref Shape> s_aShapes;
	protected static const int RING_STEPS = 48;
	protected static const ResourceName LABELS = "{DCD088BA4828B4D3}UI/layouts/ReforgerZ/RZ_SiteMarks.layout";
	protected static const int MAX_LABELS = 48;		// Mark0-Mark47 in the layout
	protected static const float NEAR = 25;			// closer than this (from the camera) a label shows its name
	protected static const float HIGH = 2.5;		// the badge stands this far over the spot

	protected static Widget s_wLabels;
	protected static bool s_bPlacing;
	protected static ref array<vector> s_aLabelPos = {};
	protected static ref array<string> s_aLabelName = {};
	protected static ref array<string> s_aLabelLetter = {};
	protected static ref array<int> s_aLabelState = {};
	protected static ref array<int> s_aLabelId = {};		// the cone's RplId (0 none): the label follows the cone
	protected static bool s_bPreview;						// a spot being placed: the last label follows the cursor
	protected static vector s_vPreview;
	protected static string s_sPreviewLetter;
	protected static string s_sPreviewName;

	//------------------------------------------------------------------------------------------------
	//! The spot about to be put down, at Game Master's cursor (UI/RZ_SiteHud.c); off with an empty type.
	static void SetPreview(bool on, vector pos, string type, string name)
	{
		s_bPreview = on;
		s_vPreview = pos;
		s_sPreviewLetter = Letter(type);
		if (type == "route")
			s_sPreviewLetter = "+";

		s_sPreviewName = name;
		if (on)
			StartLabels();
	}

	//------------------------------------------------------------------------------------------------
	static void OnPayload(string payload)
	{
		if (!s_aShapes)
			s_aShapes = {};

		s_aShapes.Clear();
		s_aLabelPos.Clear();
		s_aLabelName.Clear();
		s_aLabelLetter.Clear();
		s_aLabelState.Clear();
		s_aLabelId.Clear();
		if (payload.IsEmpty())
		{
			StopLabels();
			return;
		}

		array<string> records = {};
		payload.Split(";", records, true);
		// The patrol loops' points, by loop, in the order they were put down
		array<ref array<vector>> loops = {};
		for (int k = 0; k < 3; k++)
		{
			loops.Insert(new array<vector>());
		}

		foreach (string rec : records)
		{
			array<string> f = {};
			rec.Split("|", f, false);
			if (f.Count() >= 5 && f[0] == "R")
			{
				Ring(Vector(f[1].ToFloat(), f[2].ToFloat(), f[3].ToFloat()), f[4].ToFloat());
				continue;
			}

			if (f.Count() < 6 || f[0] != "S")
				continue;

			vector at = Vector(f[2].ToFloat(), f[3].ToFloat(), f[4].ToFloat());
			int state = f[5].ToInt();
			// The label is the spot's mark now (its badge follows the cone); suggestions keep a post
			if (state == 2)
				Post(at, state);
			string type = f[1];
			string problem = "";
			if (f.Count() >= 7)
				problem = f[6];

			int loop = 1;
			if (f.Count() >= 8)
				loop = Math.Clamp(f[7].ToInt(), 1, 3);

			string name = RZ_HandSpots.Label(type);
			string letter = Letter(type);
			if (type == "route")
			{
				loops[loop - 1].Insert(at);
				name = string.Format("Patrol %1, point %2", loop, loops[loop - 1].Count());
				letter = loops[loop - 1].Count().ToString();
			}

			if (state == 2)
				name += " (suggested)";
			else if (!problem.IsEmpty())
				name += " · " + problem;

			s_aLabelPos.Insert(at);
			s_aLabelName.Insert(name);
			s_aLabelLetter.Insert(letter);
			s_aLabelState.Insert(state);
			int id = 0;
			if (f.Count() >= 9)
				id = f[8].ToInt();

			s_aLabelId.Insert(id);
		}

		foreach (int li, array<vector> pts : loops)
		{
			Loop(pts, li);
		}

		StartLabels();
	}

	//------------------------------------------------------------------------------------------------
	//! A spot type's badge letter.
	protected static string Letter(string type)
	{
		switch (type)
		{
			case "gate": return "G";
			case "watch": return "W";
			case "sit": return "S";
			case "work": return "K";
			case "sleep": return "Z";
			case "lean": return "L";
			case "qm": return "Q";
			case "leader": return "Ld";
			case "cook": return "C";
			case "meal": return "T";
			case "water": return "Wa";
			case "wash": return "Ws";
			case "medic": return "M";
			case "stores": return "St";
			case "chop": return "Ch";
			case "latrine": return "La";
		}

		return "?";
	}

	//------------------------------------------------------------------------------------------------
	//! A patrol loop: its points joined in order, dashed, and closed back to the first.
	protected static void Loop(array<vector> pts, int index)
	{
		int n = pts.Count();
		if (n < 2)
			return;

		array<int> colours = {ARGB(255, 232, 178, 58), ARGB(255, 93, 202, 165), ARGB(255, 133, 183, 235)};
		int colour = colours[index % 3];
		int segments = n - 1;
		if (n >= 3)
			segments = n;

		for (int i = 0; i < segments; i++)
		{
			vector a = RZ_Util.GroundPos(pts[i]) + Vector(0, 0.4, 0);
			vector b = RZ_Util.GroundPos(pts[(i + 1) % n]) + Vector(0, 0.4, 0);
			float len = vector.Distance(a, b);
			int dashes = Math.Max(1, Math.Floor(len / 1.5));
			float count = dashes;
			for (int d = 0; d < dashes; d++)
			{
				float from = d;
				vector line[2];
				line[0] = a + (b - a) * (from / count);
				line[1] = a + (b - a) * ((from + 0.6) / count);
				s_aShapes.Insert(Shape.CreateLines(colour, ShapeFlags.NOZBUFFER, line, 2));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void StartLabels()
	{
		if (!s_wLabels)
		{
			WorkspaceWidget ws = GetGame().GetWorkspace();
			if (ws)
				s_wLabels = ws.CreateWidgets(LABELS);

			// Round badges: the icon set's disc
			for (int i = 0; s_wLabels && i < MAX_LABELS; i++)
			{
				RZ_UIW.Icon(s_wLabels, "MarkBadge" + i.ToString(), "marker_disc");
			}
		}

		if (!s_bPlacing)
		{
			s_bPlacing = true;
			GetGame().GetCallqueue().CallLater(PlaceLabels, 0, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void StopLabels()
	{
		if (s_bPlacing)
		{
			s_bPlacing = false;
			GetGame().GetCallqueue().Remove(PlaceLabels);
		}

		if (s_wLabels)
			s_wLabels.RemoveFromHierarchy();

		s_wLabels = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame: each label over its cone; the name when the camera is near, else the letter.
	protected static void PlaceLabels()
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		BaseWorld world = GetGame().GetWorld();
		if (!s_wLabels || !ws || !world)
			return;

		vector cam[4];
		world.GetCurrentCamera(cam);
		for (int i = 0; i < MAX_LABELS; i++)
		{
			string idx = i.ToString();
			Widget label = RZ_UIW.Find(s_wLabels, "Mark" + idx);
			if (!label)
				continue;

			// The last label is the preview while a spot is being placed
			bool preview = s_bPreview && i == MAX_LABELS - 1;
			if (i >= s_aLabelPos.Count() && !preview)
			{
				label.SetVisible(false);
				continue;
			}

			vector spot;
			string letter;
			string name;
			int state;
			if (preview)
			{
				spot = s_vPreview;
				letter = s_sPreviewLetter;
				name = s_sPreviewName;
				state = 2;
			}
			else
			{
				spot = s_aLabelPos[i];
				letter = s_aLabelLetter[i];
				name = s_aLabelName[i];
				state = s_aLabelState[i];
				// The cone itself where it's streamed in: dragging it in Game Master moves the label
				if (s_aLabelId[i] != 0)
				{
					RplId rid = s_aLabelId[i];
					RplComponent rc = RplComponent.Cast(Replication.FindItem(rid));
					if (rc && rc.GetEntity())
						spot = rc.GetEntity().GetOrigin();
				}
			}

			vector top = RZ_Util.GroundPos(spot) + Vector(0, HIGH, 0);
			vector screen = ws.ProjWorldToScreen(top, world); // VERIFY: reference-resolution units, z below 0 behind the camera (as the nametags read it)
			if (screen[2] < 0)
			{
				label.SetVisible(false);
				continue;
			}

			bool near = preview || vector.Distance(cam[3], top) < NEAR;
			Color badge = RZ_SiteBuilderMenu.Lin(151, 196, 89);
			if (state == 1)
				badge = RZ_SiteBuilderMenu.Lin(240, 149, 149);
			else if (state == 2)
				badge = RZ_SiteBuilderMenu.Lin(232, 178, 58);

			RZ_UIW.Tint(s_wLabels, "MarkBadge" + idx, badge);
			RZ_UIW.SetText(s_wLabels, "MarkLetter" + idx, letter);
			RZ_UIW.SetText(s_wLabels, "MarkName" + idx, name);
			RZ_UIW.Show(s_wLabels, "MarkName" + idx, near);
			RZ_UIW.Show(s_wLabels, "MarkNameBg" + idx, near);
			RZ_UIW.SetPos(label, screen[0] - 160, screen[1] - 84);
			label.SetVisible(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Ring(vector centre, float radius)
	{
		int white = ARGB(255, 255, 255, 255);
		vector last = Edge(centre, radius, 0);
		for (int i = 1; i <= RING_STEPS; i++)
		{
			vector next = Edge(centre, radius, 360 * i / RING_STEPS);
			vector pts[2];
			pts[0] = last;
			pts[1] = next;
			s_aShapes.Insert(Shape.CreateLines(white, ShapeFlags.NOZBUFFER, pts, 2)); // VERIFY: debug shapes made on a game client draw in the world for that player (Game Master's camera too), not only in Workbench
			last = next;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static vector Edge(vector centre, float radius, float deg)
	{
		vector at = centre + Vector(Math.Sin(deg * Math.DEG2RAD) * radius, 0, Math.Cos(deg * Math.DEG2RAD) * radius);
		return RZ_Util.GroundPos(at) + Vector(0, 0.3, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Post(vector at, int state)
	{
		int colour = ARGB(255, 40, 220, 60);
		if (state == 1)
			colour = ARGB(255, 235, 40, 40);
		else if (state == 2)
			colour = ARGB(255, 245, 210, 40);

		vector foot = RZ_Util.GroundPos(at);
		vector pts[2];
		pts[0] = foot;
		pts[1] = foot + Vector(0, 2.2, 0);
		s_aShapes.Insert(Shape.CreateLines(colour, ShapeFlags.NOZBUFFER, pts, 2));
		s_aShapes.Insert(Shape.CreateSphere(colour, ShapeFlags.NOZBUFFER | ShapeFlags.NOOUTLINE, foot + Vector(0, 2.2, 0), 0.25));
	}
}
