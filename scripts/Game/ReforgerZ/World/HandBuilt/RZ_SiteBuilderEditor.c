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

	//------------------------------------------------------------------------------------------------
	static void OnPayload(string payload)
	{
		if (!s_aShapes)
			s_aShapes = {};

		s_aShapes.Clear();
		if (payload.IsEmpty())
			return;

		array<string> records = {};
		payload.Split(";", records, true);
		foreach (string rec : records)
		{
			array<string> f = {};
			rec.Split("|", f, false);
			if (f.Count() >= 5 && f[0] == "R")
				Ring(Vector(f[1].ToFloat(), f[2].ToFloat(), f[3].ToFloat()), f[4].ToFloat());
			else if (f.Count() >= 6 && f[0] == "S")
				Post(Vector(f[2].ToFloat(), f[3].ToFloat(), f[4].ToFloat()), f[5].ToInt());
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
