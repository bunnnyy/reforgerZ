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
