//! Nametags on each client (docs/UI.md, "Nametags"): a small tag over the people near you that you
//! can talk to, their name, their role and who they're with (the faction's name in its colour).
//! The server sends the nearest few (Dialogue/RZ_Nametags.c: each body's RplId, name, role and
//! faction). Four times a second this keeps the ones within Dialogue > Nametag range that you can
//! see (one sight trace each), at most Nametag max, nearest first; every frame it moves those tags
//! over their heads (one projection each; the text is set only when a slot's person changes).
//! Needs NametagPanel with Nametag0-2 in the HUD layout. Hidden while a menu is open, while you're
//! dead, and with Dialogue > Nametags off. Never a player or the infected (the server lists neither).
//! Changes nothing in the game. Called from the HUD's update.

//------------------------------------------------------------------------------------------------
class RZ_NametagEntry
{
	RplId m_Id;
	IEntity m_Body;		// weak: found by its RplId while it's streamed in
	string m_sName;
	string m_sRole;
	int m_iFaction = -1;
	string m_sKey;		// the whole record, so a slot's text is set only when it changes
	float m_fDist;
}

//------------------------------------------------------------------------------------------------
class RZ_NametagClient
{
	protected static const float CHECK = 0.25;	// seconds between choosing who shows
	protected static const float HEAD = 2.05;	// the tag's foot above the body's origin (m)
	protected static const int SLOTS = 3;		// Nametag0-2 in the HUD layout

	protected static ref array<ref RZ_NametagEntry> s_aList = {};
	protected static ref array<ref RZ_NametagEntry> s_aShown = {};
	protected static ref array<string> s_aSlotKey = {"", "", ""};
	protected static ref array<Widget> s_aTags = {};	// the HUD's Nametag0-2
	protected static Widget s_Root;						// the HUD they were found in
	protected static float s_fTimer;

	//------------------------------------------------------------------------------------------------
	//! The server's list (RZ_PlayerUIComponent): N|id|name|role|faction per person; "" none.
	static void OnPayload(string payload)
	{
		array<ref RZ_NametagEntry> old = {};
		foreach (RZ_NametagEntry was : s_aList)
		{
			old.Insert(was);
		}

		s_aList.Clear();
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(payload, recs);
		foreach (array<string> f : recs)
		{
			if (RZ_UIText.Str(f, 0) != "N")
				continue;

			RZ_NametagEntry e = new RZ_NametagEntry();
			e.m_Id = RZ_UIText.Int(f, 1);
			e.m_sName = RZ_UIText.Str(f, 2);
			e.m_sRole = RZ_UIText.Str(f, 3);
			e.m_iFaction = RZ_UIText.Int(f, 4);
			e.m_sKey = e.m_sName + "|" + e.m_sRole + "|" + e.m_iFaction.ToString();
			foreach (RZ_NametagEntry prev : old)
			{
				if (prev.m_Id == e.m_Id)
					e.m_Body = prev.m_Body;
			}

			s_aList.Insert(e);
		}

		s_fTimer = CHECK;
	}

	//------------------------------------------------------------------------------------------------
	//! From the HUD's update, every frame.
	static void Tick(float timeSlice, Widget root)
	{
		// The slots, found once per HUD
		if (root != s_Root)
		{
			s_Root = root;
			s_aTags.Clear();
			Widget panel = RZ_UIW.Find(root, "NametagPanel");
			for (int i = 0; i < SLOTS; i++)
			{
				s_aSlotKey[i] = "";
				Widget tag = RZ_UIW.Find(panel, "Nametag" + i.ToString());
				if (tag)
					s_aTags.Insert(tag);
			}
		}

		if (s_aTags.IsEmpty())
			return;

		s_fTimer += timeSlice;
		if (s_fTimer >= CHECK)
		{
			s_fTimer = 0;
			Choose();
		}

		Place();
	}

	//------------------------------------------------------------------------------------------------
	//! Who shows: within range, in sight, the nearest few. Nobody while a menu is open or you're dead.
	protected static void Choose()
	{
		s_aShown.Clear();
		RZ_DialogueSettings settings = RZ_DialogueSettings.Get();
		IEntity me = RZ_UIW.LocalEntity();
		if (!settings.m_bNametags || !me || !RZ_Util.IsCharacterAlive(me) || MenuOpen())
			return;

		int most = Math.Min(settings.m_iNametagMax, SLOTS);
		float range = settings.m_fNametagRange;
		foreach (RZ_NametagEntry e : s_aList)
		{
			if (!e.m_Body)
				e.m_Body = Body(e.m_Id);

			if (!e.m_Body || !RZ_Util.IsCharacterAlive(e.m_Body))
				continue;

			e.m_fDist = vector.Distance(me.GetOrigin(), e.m_Body.GetOrigin());
			if (e.m_fDist > range || !RZ_DisguiseWatch.CanSee(me, e.m_Body))
				continue;

			// Nearest first
			int at = 0;
			while (at < s_aShown.Count() && s_aShown[at].m_fDist <= e.m_fDist)
			{
				at++;
			}

			s_aShown.InsertAt(e, at);
		}

		while (s_aShown.Count() > most)
		{
			s_aShown.Remove(s_aShown.Count() - 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Each slot over its person's head, or hidden; behind the camera, hidden.
	protected static void Place()
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		BaseWorld world = GetGame().GetWorld();
		foreach (int i, Widget tag : s_aTags)
		{
			RZ_NametagEntry e;
			if (i < s_aShown.Count())
				e = s_aShown[i];

			if (!e || !e.m_Body || !ws || !world)
			{
				tag.SetVisible(false);
				continue;
			}

			// The text only when this slot's person changes
			if (s_aSlotKey[i] != e.m_sKey)
			{
				s_aSlotKey[i] = e.m_sKey;
				Fill(tag, e);
			}

			vector screen = ws.ProjWorldToScreen(e.m_Body.GetOrigin() + Vector(0, HEAD, 0), world); // VERIFY: reference-resolution units, z below 0 behind the camera (as vanilla's nametags read it)
			if (screen[2] < 0)
			{
				tag.SetVisible(false);
				continue;
			}

			RZ_UIW.SetPos(tag, screen[0], screen[1]);
			tag.SetVisible(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Name in bone, the role under it, the faction's name in its colour (none: no line).
	protected static void Fill(Widget tag, RZ_NametagEntry e)
	{
		RZ_UIW.SetText(tag, "NametagName", e.m_sName);
		RZ_UIW.SetText(tag, "NametagRole", e.m_sRole);
		RZ_UIW.Show(tag, "NametagRole", !e.m_sRole.IsEmpty());
		bool faction = RZ_Factions.IsValid(e.m_iFaction);
		RZ_UIW.Show(tag, "NametagFaction", faction);
		if (!faction)
			return;

		RZ_UIW.SetText(tag, "NametagFaction", RZ_Factions.GetName(e.m_iFaction));
		RZ_UIW.Tint(tag, "NametagFaction", RZ_UIColor.Faction(e.m_iFaction));
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity Body(RplId id)
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(id));
		if (!rpl)
			return null;

		return rpl.GetEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected static bool MenuOpen()
	{
		MenuManager menus = GetGame().GetMenuManager();
		return menus && (menus.IsAnyMenuOpen() || menus.IsAnyDialogOpen());
	}
}
