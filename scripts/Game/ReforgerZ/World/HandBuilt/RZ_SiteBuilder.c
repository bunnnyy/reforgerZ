//! The site builder (docs/SITES.md, "Hand-built sites"): a server admin's or game master's menu for
//! building camp sites by hand in Game Master. It lists every site camp life lays out on this map with
//! its state (Automated / Hand-built, in use / Hand-built, not in use), takes you to one, clears its
//! automated pieces so you build on bare ground (or on them), and saves what you placed there in
//! world coordinates (RZ_HandBuilt). Opened from Game Master's context menu ("ReforgerZ: Site
//! builder", RZ_SiteBuilderEditor.c), the talk screen's [Admin] list, or "/sites" in chat. It is the
//! object menu fed as data rows (RZ_ObjectRows), so it works with a pad like every menu.
//!
//! What a site's save holds: every Game Master object (not a character, a group, a vehicle or an
//! item) standing within the site's radius of its centre that this session's game master placed,
//! plus the objects the builder put out to be edited (the save it loaded, or the automated pieces it
//! started from) that are still there. Camp life's own pieces, loot and people are never saved.
//!
//! Role spots (RZ_HandSpots, docs/SITES.md "Role spots"): while a site is built the menu puts them
//! down at the builder's feet, facing their way, each marked by a small Game Master object (Camp
//! life > Spot marker) that can be moved or deleted in Game Master; the save reads them from their
//! markers. The menu shows the site's checklist, and the builder's client draws the site's edge and
//! the spots, green, red or yellow (RZ_SiteBuilderMarks).

//------------------------------------------------------------------------------------------------
//! One admin's builder: what the menu shows and the site they're building.
class RZ_SiteBuilderSession
{
	int m_iPlayer;
	int m_iView;					// RZ_SiteBuilder.VIEW_*
	int m_iFilter;					// 0 all, 1 automated only, 2 hand-built only
	bool m_bNearest;				// nearest first, else by faction then name
	int m_iPage;
	string m_sSelected;				// the site the site view shows
	ref RZ_SiteEntry m_Building;	// the site being built, or null
	ref array<IEntity> m_aPut = {};	// what the builder put out there to be edited
	string m_sSig;					// what stood there when it was last loaded or saved
	string m_sArmed;				// a row pressed once that needs a second press ("save", "restore", "delete")
	string m_sThen;					// after the unsaved-changes question: "go:<id>", "build:<id>:<how>", "stop"
	ref array<ref RZ_HandSpot> m_aSpots = {};		// the role spots put down there (RZ_HandSpots)
	ref array<IEntity> m_aMarks = {};				// each spot's marker (null: none), Game Master can move it
	ref array<bool> m_aMarked = {};					// a marker was put out for the spot (gone since: it was deleted)
	ref array<ref RZ_HandSpot> m_aSuggest = {};		// where the automated layout had each kind of spot
	int m_iSpotType;								// the type the place row puts down (RZ_HandSpots.Types)
	string m_sSaid;									// the last thing the builder was told (the screen's confirmation line)
	string m_sHow;									// how this build started: "saved", "auto" or "empty" (the bar's Reset)
}

//------------------------------------------------------------------------------------------------
//! The menu's rows, worked out each time it's sent.
class RZ_SiteBuilderRows : RZ_ObjectRows
{
	protected RZ_SiteBuilderSession m_Session;
	protected ref array<string> m_aActs = {};
	protected ref array<string> m_aLabels = {};
	protected ref array<string> m_aLocks = {};

	//------------------------------------------------------------------------------------------------
	void RZ_SiteBuilderRows(RZ_SiteBuilderSession s)
	{
		m_Session = s;
	}

	//------------------------------------------------------------------------------------------------
	protected void Build()
	{
		m_aActs.Clear();
		m_aLabels.Clear();
		m_aLocks.Clear();
		RZ_SiteBuilder.Rows(m_Session, m_aActs, m_aLabels, m_aLocks);
	}

	//------------------------------------------------------------------------------------------------
	override void Labels(notnull array<string> labels)
	{
		Build();
		labels.Copy(m_aLabels);
	}

	//------------------------------------------------------------------------------------------------
	override bool Locked(int index, IEntity user, out string reason, out string tag)
	{
		if (index < 0 || index >= m_aLocks.Count() || m_aLocks[index].IsEmpty())
			return false;

		reason = m_aLocks[index];
		tag = "LOCKED";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(int index, IEntity user)
	{
		Build();
		if (index < 0 || index >= m_aActs.Count())
			return;

		RZ_SiteBuilder.Act(m_Session, m_aActs[index]);
	}

	//------------------------------------------------------------------------------------------------
	override string Text()
	{
		return RZ_SiteBuilder.Describe(m_Session);
	}

	//------------------------------------------------------------------------------------------------
	override void Closed(int playerId)
	{
		RZ_SiteBuilder.OnClosed(m_Session);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_SiteBuilder
{
	static const int VIEW_LIST = 0;
	static const int VIEW_SITE = 1;
	static const int VIEW_LEAVE = 2;		// the site being built has unsaved changes: save, discard or stay
	protected static const int PAGE = 10;	// sites a page (the menu has 16 rows)
	protected static const string TITLE = "Site builder";

	protected static ref map<int, ref RZ_SiteBuilderSession> s_mSessions;

	// ================================================================ Opening
	//------------------------------------------------------------------------------------------------
	//! A server admin, or a game master with the full editor.
	static bool MayBuild(int playerId)
	{
		if (RZ_NoticeBoard.IsAdmin(playerId))
			return true;

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return false;

		SCR_EditorManagerEntity manager = core.GetEditorManager(playerId);
		return manager && !manager.IsLimited(); // VERIFY: a game master's editor manager isn't limited, a player's (photo mode only) is
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the menu for the player (server): from the Game Master action, the [Admin] reply, "/sites".
	static void Open(int playerId)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetCampLife())
		{
			RZ_Log.Info(string.Format("Site builder: player %1 asked, but camp life isn't running.", playerId));
			return;
		}

		if (!MayBuild(playerId))
		{
			RZ_Log.Info(string.Format("Site builder: player %1 asked, but is neither a server admin nor a game master with the full editor.", playerId));
			gm.NotifyPlayer(playerId, TITLE, "Only a server admin or a game master can build sites.");
			return;
		}

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!user)
		{
			RZ_Log.Info(string.Format("Site builder: player %1 asked with no character.", playerId));
			gm.NotifyPlayer(playerId, TITLE, "The site builder opens on your character: spawn one first.");
			return;
		}

		// The site builder's own screen (RZ_SiteBuilderScreen); a client that can't open it asks for
		// the object menu instead (OpenRowsMenu)
		RZ_SiteBuilderSession screen = SessionOf(playerId);
		if (screen.m_sSelected.IsEmpty() && screen.m_Building)
			screen.m_sSelected = screen.m_Building.m_sId;

		RZ_SiteBuilderScreen.Send(screen);
		RZ_Log.Info(string.Format("Site builder: opened for player %1.", playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! The object-menu version of the site builder: rows (the fallback when the screen can't open).
	static void OpenRowsMenu(int playerId)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!gm || !user)
			return;

		RZ_SiteBuilderSession s = SessionOf(playerId);
		if (gm.GetScreenGuard() && gm.GetScreenGuard().IsAnyOpen(playerId) && !gm.GetScreenGuard().IsOpen(playerId, RZ_ScreenGuard.KIND_OBJECT))
		{
			// From the talk screen's [Admin] list: that screen gives way
			gm.GetScreenGuard().Forget(playerId);
		}

		if (!RZ_ObjectScreen.OpenRows(playerId, user, TITLE, new RZ_SiteBuilderRows(s)))
		{
			RZ_Log.Info(string.Format("Site builder: player %1's menu couldn't open (the object row layout, or another screen open).", playerId));
			gm.NotifyPlayer(playerId, TITLE, "The menu couldn't open (UI > Object row layout is empty, or another screen is open). Close other screens and try again.");
			return;
		}

		RZ_Log.Info(string.Format("Site builder: the object menu for player %1.", playerId));
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_SiteBuilderSession SessionOf(int playerId)
	{
		if (!s_mSessions)
			s_mSessions = new map<int, ref RZ_SiteBuilderSession>();

		RZ_SiteBuilderSession s;
		if (!s_mSessions.Find(playerId, s))
		{
			s = new RZ_SiteBuilderSession();
			s.m_iPlayer = playerId;
			s_mSessions.Insert(playerId, s);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The menu closed: a site with unsaved changes is said so (it stays as it is, to come back to).
	static void OnClosed(RZ_SiteBuilderSession s)
	{
		s.m_sArmed = "";
		if (s.m_iView == VIEW_LEAVE)
			s.m_iView = VIEW_SITE;

		if (!s.m_Building || !Unsaved(s))
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
			gm.NotifyPlayer(s.m_iPlayer, TITLE, string.Format("%1 has unsaved changes. Open the site builder to save or discard them.", s.m_Building.m_sTitle));
	}

	// ================================================================ The sites and their states
	//------------------------------------------------------------------------------------------------
	protected static void Sites(notnull array<ref RZ_SiteEntry> result)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetCampLife())
			gm.GetCampLife().ListSites(result);
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_SiteEntry SiteById(string id)
	{
		array<ref RZ_SiteEntry> all = {};
		Sites(all);
		s_aKeepById = all;
		foreach (RZ_SiteEntry e : all)
		{
			if (e.m_sId == id)
				return e;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! 0 automated, 1 hand-built in use, 2 hand-built not in use.
	static int StateOf(RZ_SiteEntry e)
	{
		RZ_HandBuiltSite hs = RZ_HandBuilt.Get().ForPlace(e.m_eKind, e.m_vPos);
		if (!hs)
			return 0;

		if (hs.m_bUse)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	static string StateText(int state)
	{
		if (state == 1)
			return "HAND-BUILT, IN USE";

		if (state == 2)
			return "HAND-BUILT, NOT IN USE";

		return "AUTOMATED";
	}

	//------------------------------------------------------------------------------------------------
	//! "Haven safehouse, near Morton": a title with the town it's in when the title doesn't say.
	protected static string Name(RZ_SiteEntry e)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || e.m_sTitle.Contains(","))
			return e.m_sTitle;

		RZ_Settlement town = gm.GetSettlement(gm.FindNearestSettlement(e.m_vPos));
		if (!town)
			return e.m_sTitle;

		return e.m_sTitle + ", near " + town.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector PlayerPos(int playerId)
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!user)
			return vector.Zero;

		return user.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	//! The sites the list shows, in its order.
	protected static void Listed(RZ_SiteBuilderSession s, notnull array<RZ_SiteEntry> result)
	{
		array<ref RZ_SiteEntry> all = {};
		Sites(all);
		vector me = PlayerPos(s.m_iPlayer);
		array<RZ_SiteEntry> shown = {};
		array<float> keys = {};
		foreach (RZ_SiteEntry e : all)
		{
			int state = StateOf(e);
			if (s.m_iFilter == 1 && state != 0)
				continue;

			if (s.m_iFilter == 2 && state == 0)
				continue;

			float key;
			if (s.m_bNearest)
				key = RZ_Util.DistXZ(me, e.m_vPos);
			else
				key = e.m_eFaction * 100000 + e.m_eKind * 10000 + shown.Count();

			int at = 0;
			while (at < keys.Count() && keys[at] <= key)
			{
				at++;
			}

			keys.InsertAt(key, at);
			shown.InsertAt(e, at);
		}

		// Keep the shared list's entries alive in the result (the caller keeps `all` no longer)
		foreach (RZ_SiteEntry kept : shown)
		{
			result.Insert(kept);
		}

		s_aKeep = all;
	}

	protected static ref array<ref RZ_SiteEntry> s_aKeep;		// the last list's entries (Listed hands out weak ones)
	protected static ref array<ref RZ_SiteEntry> s_aKeepById;	// the same for SiteById

	// ================================================================ Capturing what stands there
	//------------------------------------------------------------------------------------------------
	//! What the site's save would hold now: the builder's own objects still there and the objects
	//! this session's game master placed, within the site's radius. `outside`: how many of those
	//! stand beyond it (not saved).
	static void Capture(RZ_SiteBuilderSession s, notnull array<IEntity> result, out int outside)
	{
		outside = 0;
		RZ_SiteEntry site = s.m_Building;
		if (!site)
			return;

		foreach (IEntity put : s.m_aPut)
		{
			if (!put || IsMark(s, put))
				continue;

			if (RZ_Util.DistXZ(put.GetOrigin(), site.m_vPos) <= site.m_fRadius)
				result.Insert(put);
			else
				outside++;
		}

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;

		set<SCR_EditableEntityComponent> all = new set<SCR_EditableEntityComponent>();
		core.GetAllEntities(all, true); // VERIFY: onlyDirect gives the top-level editable entities (a composition's root, not its parts)
		foreach (SCR_EditableEntityComponent ec : all)
		{
			if (!ec || ec.GetAuthorPlayerID() <= 0)
				continue;

			if (ec.GetEntityType() != EEditableEntityType.GENERIC)
				continue;

			IEntity e = ec.GetOwner();
			if (!e || result.Contains(e) || RZ_CampLife.IsDressing(e) || IsMark(s, e))
				continue;

			float d = RZ_Util.DistXZ(e.GetOrigin(), site.m_vPos);
			if (d <= site.m_fRadius)
				result.Insert(e);
			else if (d <= site.m_fRadius * 1.5)
				outside++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A spot's marker, never an object of the site: one of the session's, or a stray one of the
	//! marker's prefab standing on a spot (it lost its spot, and isn't saved as a cone).
	protected static bool IsMark(RZ_SiteBuilderSession s, IEntity e)
	{
		if (s.m_aMarks.Contains(e))
			return true;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetCampLifeSettings() || !e.GetPrefabData())
			return false;

		ResourceName marker = gm.GetCampLifeSettings().m_sSpotMarker;
		if (marker.IsEmpty() || e.GetPrefabData().GetPrefabName() != marker)
			return false;

		foreach (RZ_HandSpot sp : s.m_aSpots)
		{
			if (RZ_Util.DistXZ(sp.m_vPos, e.GetOrigin()) < 0.5)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A short fingerprint of what stands there: changes when anything is placed, moved or removed.
	protected static string Sig(RZ_SiteBuilderSession s)
	{
		array<IEntity> now = {};
		int outside;
		Capture(s, now, outside);
		float sum = 0;
		foreach (int i, IEntity e : now)
		{
			vector p = e.GetOrigin();
			vector a = e.GetYawPitchRoll();
			sum += (p[0] * 3.1 + p[1] * 7.7 + p[2] * 1.3 + a[0] * 0.37 + a[1] * 0.11 + a[2] * 0.53) * (Math.Mod(i, 7) + 1);
		}

		SyncSpots(s);
		foreach (int k, RZ_HandSpot sp : s.m_aSpots)
		{
			sum += (sp.m_vPos[0] * 2.3 + sp.m_vPos[2] * 5.9 + sp.m_fYaw * 0.29) * (Math.Mod(k, 5) + 2);
		}

		return string.Format("%1|%2|%3", now.Count(), s.m_aSpots.Count(), sum.ToString(-1, 1));
	}

	//------------------------------------------------------------------------------------------------
	static bool Unsaved(RZ_SiteBuilderSession s)
	{
		return s.m_Building && Sig(s) != s.m_sSig;
	}

	// ================================================================ The rows
	//------------------------------------------------------------------------------------------------
	//! The rows of the view the session is on: their action codes, labels and why each is locked
	//! ("" when it isn't).
	static void Rows(RZ_SiteBuilderSession s, notnull array<string> acts, notnull array<string> labels, notnull array<string> locks)
	{
		if (s.m_iView == VIEW_LEAVE && s.m_Building)
		{
			Row(acts, labels, locks, "leave_save", "Save, then go on", "");
			Row(acts, labels, locks, "leave_discard", "Discard the changes, then go on", "");
			Row(acts, labels, locks, "leave_stay", "Stay and keep building", "");
			return;
		}

		RZ_SiteEntry site;
		if (s.m_iView == VIEW_SITE)
			site = SiteById(s.m_sSelected);

		if (!site)
		{
			s.m_iView = VIEW_LIST;
			ListRows(s, acts, labels, locks);
			return;
		}

		SiteRows(s, site, acts, labels, locks);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Row(notnull array<string> acts, notnull array<string> labels, notnull array<string> locks, string act, string label, string lock)
	{
		acts.Insert(act);
		labels.Insert(label);
		locks.Insert(lock);
	}

	//------------------------------------------------------------------------------------------------
	protected static void ListRows(RZ_SiteBuilderSession s, notnull array<string> acts, notnull array<string> labels, notnull array<string> locks)
	{
		string show = "Show: all sites";
		if (s.m_iFilter == 1)
			show = "Show: automated sites only";
		else if (s.m_iFilter == 2)
			show = "Show: hand-built sites only";

		string order = "Order: by faction";
		if (s.m_bNearest)
			order = "Order: nearest first";

		Row(acts, labels, locks, "filter", show, "");
		Row(acts, labels, locks, "order", order, "");
		Row(acts, labels, locks, "next_unbuilt", "Next site not built yet (nearest)", "");
		if (s.m_Building)
			Row(acts, labels, locks, "open:" + s.m_Building.m_sId, "Back to the site you're building: " + Name(s.m_Building), "");

		array<RZ_SiteEntry> shown = {};
		Listed(s, shown);
		int pages = Math.Max(1, (shown.Count() + PAGE - 1) / PAGE);
		if (s.m_iPage >= pages)
			s.m_iPage = 0;

		int first = s.m_iPage * PAGE;
		for (int i = first; i < shown.Count() && i < first + PAGE; i++)
		{
			RZ_SiteEntry e = shown[i];
			string label = Name(e) + " · " + StateText(StateOf(e));
			RZ_HandBuiltSite listed = RZ_HandBuilt.Get().ForPlace(e.m_eKind, e.m_vPos);
			if (listed)
			{
				int listMissing;
				label += " · spots " + RZ_HandSpots.Short(e.m_eFaction, e.m_eKind, listed.m_aSpots, listMissing);
				if (listed.m_bPinned)
					label += " · PINNED";
			}

			if (s.m_Building && s.m_Building.m_sId == e.m_sId)
				label += " · BUILDING NOW";

			Row(acts, labels, locks, "open:" + e.m_sId, label, "");
		}

		if (pages > 1)
			Row(acts, labels, locks, "page", string.Format("More sites (page %1 of %2)", s.m_iPage + 1, pages), "");
	}

	//------------------------------------------------------------------------------------------------
	protected static void SiteRows(RZ_SiteBuilderSession s, RZ_SiteEntry site, notnull array<string> acts, notnull array<string> labels, notnull array<string> locks)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_HandBuilt store = RZ_HandBuilt.Get();
		RZ_HandBuiltSite saved = store.ForPlace(site.m_eKind, site.m_vPos);
		bool here = s.m_Building && s.m_Building.m_sId == site.m_sId;
		string other = OtherBuilder(s, site.m_sId);

		Row(acts, labels, locks, "go", "Go there", "");
		if (here)
		{
			string saveLabel = "Save this site";
			if (s.m_sArmed == "save" && saved)
				saveLabel = string.Format("Save: press again to replace the save from %1", saved.m_sSaved);

			Row(acts, labels, locks, "save", saveLabel, "");
			array<string> types = {};
			RZ_HandSpots.Types(types);
			string type = types[s.m_iSpotType % types.Count()];
			Row(acts, labels, locks, "spot_place", "Put a spot at your feet: " + RZ_HandSpots.Label(type), "");
			Row(acts, labels, locks, "spot_next", "Spot type: " + RZ_HandSpots.Label(type) + ". Next type", "");
			Row(acts, labels, locks, "spot_back", "Previous spot type", "");
			string removeLock = "";
			string removeLabel = "Take up the nearest spot";
			int nearest = NearestSpot(s);
			if (nearest < 0)
				removeLock = "No spots put down yet.";
			else
				removeLabel = string.Format("Take up the nearest spot (%1, %2 m)", RZ_HandSpots.Label(s.m_aSpots[nearest].m_sType), RZ_Util.RoundToInt(RZ_Util.DistXZ(PlayerPos(s.m_iPlayer), s.m_aSpots[nearest].m_vPos)));

			Row(acts, labels, locks, "spot_remove", removeLabel, removeLock);
			Row(acts, labels, locks, "stop", "Stop building", "");
		}
		else if (saved)
		{
			Row(acts, labels, locks, "build_saved", string.Format("Build here: edit the saved site (%1 objects)", saved.m_aObjects.Count()), other);
		}
		else
		{
			string autoLock = other;
			if (autoLock.IsEmpty() && gm.GetCampLife().PiecesAt(site.m_sId, site.m_vPos, site.m_fRadius) == 0)
				autoLock = "Go there first and wait a moment: the automated layout is put out once you're near.";

			Row(acts, labels, locks, "build_empty", "Build here: start empty", other);
			Row(acts, labels, locks, "build_auto", "Build here: start from the automated layout", autoLock);
		}

		if (saved)
		{
			string useLabel = "In use: HAND-BUILT. Switch to automated";
			if (!saved.m_bUse)
				useLabel = "In use: AUTOMATED. Switch to hand-built";

			Row(acts, labels, locks, "use", useLabel, "");
			if (site.m_eKind != RZ_ECampPlace.ROAD_POST)
			{
				string pinLabel = "Keep this site here every season: OFF. Switch on";
				if (saved.m_bPinned)
					pinLabel = "Keep this site here every season: ON. Switch off";

				Row(acts, labels, locks, "pin", pinLabel, "");
			}
		}

		if (store.BackupCount(site.m_sId) > 0)
		{
			string restoreLabel = string.Format("Restore the last save (%1 kept)", store.BackupCount(site.m_sId));
			if (s.m_sArmed == "restore")
				restoreLabel = "Restore: press again to go back to the save before this one";

			Row(acts, labels, locks, "restore", restoreLabel, other);
		}

		Row(acts, labels, locks, "list", "Back to the list", "");
		if (saved)
		{
			string deleteLabel = "Delete the hand-built";
			if (s.m_sArmed == "delete")
				deleteLabel = "Delete: press again to delete it for good (the automated layout comes back)";

			Row(acts, labels, locks, "delete", deleteLabel, other);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Who else is building this site ("" nobody), as a row's lock.
	protected static string OtherBuilder(RZ_SiteBuilderSession s, string id)
	{
		if (!s_mSessions)
			return "";

		foreach (int pid, RZ_SiteBuilderSession o : s_mSessions)
		{
			if (o != s && o.m_Building && o.m_Building.m_sId == id)
				return string.Format("%1 is building this site now.", RZ_Util.PlayerName(pid));
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The description over the rows.
	static string Describe(RZ_SiteBuilderSession s)
	{
		if (s.m_iView == VIEW_LEAVE && s.m_Building)
			return string.Format("%1 has unsaved changes.\nSave them, discard them (the site goes back to its last save), or stay.", Name(s.m_Building));

		RZ_SiteEntry site;
		if (s.m_iView == VIEW_SITE)
			site = SiteById(s.m_sSelected);

		if (!site)
			return ListText(s);

		RZ_HandBuiltSite saved = RZ_HandBuilt.Get().ForPlace(site.m_eKind, site.m_vPos);
		string text = Name(site) + "\n" + StateText(StateOf(site));
		if (saved)
		{
			string where = "on this machine";
			if (saved.m_bShipped)
				where = "shipped with the mod";

			text += string.Format("\n%1 objects, saved %2, %3.", saved.m_aObjects.Count(), saved.m_sSaved, where);
		}
		else
		{
			text += "\nNothing built: camp life lays it out.";
		}

		text += string.Format("\n%1 m from you. Everything you place within %2 m of its centre belongs to it.", RZ_Util.RoundToInt(RZ_Util.DistXZ(PlayerPos(s.m_iPlayer), site.m_vPos)), RZ_Util.RoundToInt(site.m_fRadius));
		if (saved && saved.m_bPinned)
			text += " PINNED: it stands here every season.";

		// The checklist: what's being built, else the save's
		array<ref RZ_HandSpot> spots;
		bool buildingHere = s.m_Building && s.m_Building.m_sId == site.m_sId;
		if (buildingHere)
		{
			SyncSpots(s);
			spots = s.m_aSpots;
		}
		else if (saved)
		{
			spots = saved.m_aSpots;
		}

		if (buildingHere || saved)
		{
			int missing;
			text += "\n" + RZ_HandSpots.Text(site.m_eFaction, site.m_eKind, spots, missing);
			if (buildingHere)
			{
				int bad = Problems(s);
				if (bad > 0)
					text += string.Format("\n%1 spots are red: %2", bad, FirstProblem(s));

				SendMarks(s);
			}
		}
		if (s.m_Building && s.m_Building.m_sId == site.m_sId)
		{
			array<IEntity> now = {};
			int outside;
			Capture(s, now, outside);
			text += string.Format("\nBUILDING NOW: %1 objects.", now.Count());
			if (Unsaved(s))
				text += " UNSAVED CHANGES.";

			if (outside > 0)
				text += string.Format(" %1 stand outside the site and won't be saved.", outside);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string ListText(RZ_SiteBuilderSession s)
	{
		array<ref RZ_SiteEntry> all = {};
		Sites(all);
		int counts[3];
		foreach (RZ_SiteEntry e : all)
		{
			counts[StateOf(e)] = counts[StateOf(e)] + 1;
		}

		string text = string.Format("%1 sites on %2: %3 automated, %4 hand-built in use, %5 hand-built not in use.", all.Count(), RZ_HandBuilt.Get().GetWorld(), counts[0], counts[1], counts[2]);
		int orphans = Orphans(all);
		if (orphans > 0)
			text += string.Format(" %1 saves are for places with no site this season (kept for when one stands there again).", orphans);

		text += "\nAUTOMATED: camp life lays it out. HAND-BUILT, IN USE: your build is used. HAND-BUILT, NOT IN USE: kept, but the automated layout is used.";
		if (s.m_Building)
		{
			text += "\nYou're building " + Name(s.m_Building) + ".";
			if (Unsaved(s))
				text += " UNSAVED CHANGES.";
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Saves of this map that no site standing now matches (the season put the holding elsewhere).
	protected static int Orphans(notnull array<ref RZ_SiteEntry> all)
	{
		RZ_HandBuilt store = RZ_HandBuilt.Get();
		set<string> used = new set<string>();
		foreach (RZ_SiteEntry e : all)
		{
			RZ_HandBuiltSite hs = store.ForPlace(e.m_eKind, e.m_vPos);
			if (hs)
				used.Insert(hs.m_sId);
		}

		array<RZ_HandBuiltSite> saves = {};
		store.All(saves);
		int n = 0;
		foreach (RZ_HandBuiltSite save : saves)
		{
			if (!used.Contains(save.m_sId))
				n++;
		}

		return n;
	}

	// ================================================================ What a row does
	//------------------------------------------------------------------------------------------------
	static void Act(RZ_SiteBuilderSession s, string act)
	{
		string armed = s.m_sArmed;
		s.m_sArmed = "";
		if (act == "filter")
		{
			s.m_iFilter = (s.m_iFilter + 1) % 3;
			s.m_iPage = 0;
		}
		else if (act == "order")
		{
			s.m_bNearest = !s.m_bNearest;
			s.m_iPage = 0;
		}
		else if (act == "page")
		{
			s.m_iPage++;
		}
		else if (act == "next_unbuilt")
		{
			NextUnbuilt(s);
		}
		else if (act.IndexOf("open:") == 0)
		{
			s.m_sSelected = act.Substring(5, act.Length() - 5);
			s.m_iView = VIEW_SITE;
		}
		else if (act == "list")
		{
			s.m_iView = VIEW_LIST;
		}
		else if (act == "go")
		{
			Leaving(s, "go:" + s.m_sSelected);
		}
		else if (act == "build_saved")
		{
			Leaving(s, "build:" + s.m_sSelected + ":saved");
		}
		else if (act == "build_empty")
		{
			Leaving(s, "build:" + s.m_sSelected + ":empty");
		}
		else if (act == "build_auto")
		{
			Leaving(s, "build:" + s.m_sSelected + ":auto");
		}
		else if (act == "save")
		{
			Save(s, armed == "save");
		}
		else if (act == "stop")
		{
			if (Unsaved(s))
			{
				s.m_sThen = "stop";
				s.m_iView = VIEW_LEAVE;
			}
			else
			{
				Stop(s);
			}
		}
		else if (act == "use")
		{
			SwitchUse(s);
		}
		else if (act == "pin")
		{
			SwitchPin(s);
		}
		else if (act == "spot_place")
		{
			PlaceSpot(s);
		}
		else if (act == "spot_next")
		{
			s.m_iSpotType++;
		}
		else if (act == "spot_back")
		{
			array<string> types = {};
			RZ_HandSpots.Types(types);
			s.m_iSpotType = (s.m_iSpotType + types.Count() - 1) % types.Count();
		}
		else if (act == "spot_remove")
		{
			RemoveSpot(s, NearestSpot(s));
		}
		else if (act == "restore")
		{
			Restore(s, armed == "restore");
		}
		else if (act == "delete")
		{
			Delete(s, armed == "delete");
		}
		else if (act == "leave_save")
		{
			if (Save(s, true))
				Then(s);
		}
		else if (act == "leave_discard")
		{
			Stop(s);
			Then(s);
		}
		else if (act == "leave_stay")
		{
			s.m_sThen = "";
			s.m_iView = VIEW_SITE;
			if (s.m_Building)
				s.m_sSelected = s.m_Building.m_sId;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Going somewhere else while building: with unsaved changes the question first; else the build
	//! there ends (it's saved) and it goes on.
	protected static void Leaving(RZ_SiteBuilderSession s, string then)
	{
		s.m_sThen = then;
		bool sameSite = s.m_Building && then.Contains(s.m_Building.m_sId);
		if (s.m_Building && !sameSite && Unsaved(s))
		{
			s.m_iView = VIEW_LEAVE;
			return;
		}

		if (s.m_Building && !sameSite)
			Stop(s);

		Then(s);
	}

	//------------------------------------------------------------------------------------------------
	//! What was waiting on the unsaved-changes question.
	protected static void Then(RZ_SiteBuilderSession s)
	{
		string then = s.m_sThen;
		s.m_sThen = "";
		s.m_iView = VIEW_SITE;
		if (then == "stop")
		{
			if (s.m_Building)
				Stop(s);

			return;
		}

		array<string> parts = {};
		then.Split(":", parts, false);
		if (parts.Count() < 2)
			return;

		RZ_SiteEntry site = SiteById(parts[1]);
		if (!site)
			return;

		s.m_sSelected = site.m_sId;
		if (parts[0] == "go")
		{
			Go(s, site);
			return;
		}

		if (parts[0] == "build" && parts.Count() >= 3)
		{
			if (s.m_Building && s.m_Building.m_sId != site.m_sId)
				Stop(s);

			if (!s.m_Building)
				Start(s, site, parts[2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Say(RZ_SiteBuilderSession s, string text)
	{
		s.m_sSaid = text;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
			gm.NotifyPlayer(s.m_iPlayer, TITLE, text);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest site with nothing built, shown.
	protected static void NextUnbuilt(RZ_SiteBuilderSession s)
	{
		array<ref RZ_SiteEntry> all = {};
		Sites(all);
		vector me = PlayerPos(s.m_iPlayer);
		RZ_SiteEntry best;
		float bestDist = float.MAX;
		foreach (RZ_SiteEntry e : all)
		{
			if (StateOf(e) != 0 || e.m_sId == s.m_sSelected)
				continue;

			float d = RZ_Util.DistXZ(me, e.m_vPos);
			if (d < bestDist)
			{
				bestDist = d;
				best = e;
			}
		}

		if (!best)
		{
			Say(s, "Every site here is hand-built.");
			return;
		}

		s.m_sSelected = best.m_sId;
		s.m_iView = VIEW_SITE;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes the admin to the site: their character to its edge, Game Master's camera above it.
	protected static void Go(RZ_SiteBuilderSession s, RZ_SiteEntry site)
	{
		vector stand = RZ_Mission.LandPointNear(site.m_vPos, site.m_fRadius * 0.5, site.m_fRadius * 0.8);
		if (!SCR_Global.TeleportPlayer(s.m_iPlayer, RZ_Util.GroundPos(stand))) // VERIFY: the server's teleport moves a player's own character (vanilla's Game Master "teleport here" calls the same)
		{
			Say(s, "Couldn't move you there.");
			return;
		}

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(s.m_iPlayer);
		vector c = RZ_Util.GroundPos(site.m_vPos);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_SITECAM, string.Format("%1|%2|%3|%4", c[0], c[1], c[2], site.m_fRadius));

		RZ_Log.Info(string.Format("Site builder: %1 went to %2 (%3).", RZ_Util.PlayerName(s.m_iPlayer), site.m_sTitle, site.m_sId));
		Say(s, string.Format("At %1. Its people and its layout come out in a moment.", Name(site)));
	}

	//------------------------------------------------------------------------------------------------
	//! Building starts: camp life's pieces there go (or stay to be built on), the save is put out to
	//! be edited when there is one.
	protected static void Start(RZ_SiteBuilderSession s, RZ_SiteEntry site, string how)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_CampLife camp = gm.GetCampLife();
		if (!OtherBuilder(s, site.m_sId).IsEmpty())
		{
			Say(s, OtherBuilder(s, site.m_sId));
			return;
		}

		s.m_Building = site;
		s.m_sHow = how;
		s.m_aPut.Clear();
		array<IEntity> keep;
		if (how == "auto")
			keep = s.m_aPut;

		s.m_aSuggest.Clear();
		camp.Suggestions(site.m_sId, site.m_vPos, site.m_fRadius, s.m_aSuggest);
		ClearSpots(s);
		int pieces = camp.StartBuilding(site.m_sId, site.m_vPos, site.m_fRadius, keep);
		int loaded = 0;
		RZ_HandBuiltSite saved = RZ_HandBuilt.Get().ForPlace(site.m_eKind, site.m_vPos);
		if (how == "saved" && saved)
		{
			foreach (RZ_HandSpot sp : saved.m_aSpots)
			{
				AddSpot(s, sp.m_sType, sp.m_vPos, sp.m_fYaw, sp.m_iSeats);
			}

			foreach (RZ_HandBuiltObject o : saved.m_aObjects)
			{
				IEntity e = RZ_HandBuilt.Spawn(o);
				if (e)
				{
					s.m_aPut.Insert(e);
					loaded++;
				}
			}
		}

		s.m_sSig = Sig(s);
		SendMarks(s);
		string what = "empty";
		if (how == "auto")
			what = string.Format("from the automated layout (%1 pieces)", pieces);
		else if (how == "saved")
			what = string.Format("from its save (%1 objects)", loaded);

		RZ_Log.Info(string.Format("Site builder: %1 builds %2 (%3), %4.", RZ_Util.PlayerName(s.m_iPlayer), site.m_sTitle, site.m_sId, what));
		Say(s, string.Format("Building %1, %2. Place things in Game Master within %3 m of its centre, then save.", Name(site), what, RZ_Util.RoundToInt(site.m_fRadius)));
	}

	//------------------------------------------------------------------------------------------------
	//! Saves the site being built. Replacing a save asks for a second press. True when it's saved.
	protected static bool Save(RZ_SiteBuilderSession s, bool confirmed)
	{
		RZ_SiteEntry site = s.m_Building;
		if (!site)
			return false;

		RZ_HandBuilt store = RZ_HandBuilt.Get();
		RZ_HandBuiltSite before = store.ForPlace(site.m_eKind, site.m_vPos);
		if (before && !confirmed)
		{
			s.m_sArmed = "save";
			return false;
		}

		array<IEntity> now = {};
		int outside;
		Capture(s, now, outside);
		RZ_HandBuiltSite hs = new RZ_HandBuiltSite();
		hs.m_sId = site.m_sId;
		if (before)
			hs.m_sId = before.m_sId;

		hs.m_sTitle = Name(site);
		hs.m_sKind = RZ_HandBuilt.KindWord(site.m_eKind);
		hs.m_vCentre = site.m_vPos;
		hs.m_fRadius = site.m_fRadius;
		hs.m_bUse = true;
		if (before)
			hs.m_bUse = before.m_bUse;

		hs.m_sSaved = RZ_HandBuilt.Now();
		hs.m_iFaction = site.m_eFaction;
		if (before)
			hs.m_bPinned = before.m_bPinned;

		foreach (IEntity e : now)
		{
			hs.m_aObjects.Insert(RZ_HandBuilt.Read(e));
		}

		SyncSpots(s);
		int spotsOutside = 0;
		foreach (RZ_HandSpot sp : s.m_aSpots)
		{
			if (RZ_Util.DistXZ(sp.m_vPos, site.m_vPos) > site.m_fRadius)
			{
				spotsOutside++;
				continue;
			}

			RZ_HandSpot kept = new RZ_HandSpot();
			kept.m_sType = sp.m_sType;
			kept.m_vPos = sp.m_vPos;
			kept.m_fYaw = sp.m_fYaw;
			kept.m_iSeats = sp.m_iSeats;
			hs.m_aSpots.Insert(kept);
		}

		if (!store.Save(hs))
		{
			Say(s, "Couldn't write the save (see the log).");
			return false;
		}

		s.m_sSig = Sig(s);
		int missing;
		string check = RZ_HandSpots.Short(site.m_eFaction, site.m_eKind, hs.m_aSpots, missing);
		int bad = Problems(s);
		RZ_Log.Info(string.Format("Hand-built: saved %1 (%2): %3 objects to %4/%5/%6.txt; %7 outside its radius not saved.", site.m_sTitle, hs.m_sId, now.Count(), RZ_HandBuilt.DIR, store.GetWorld(), hs.m_sId, outside));
		RZ_Log.Info(string.Format("Hand-built: %1 (%2) role spots: %3 saved, %4 outside not saved, %5 red; needs %6.", site.m_sTitle, hs.m_sId, hs.m_aSpots.Count(), spotsOutside, bad, check));
		string said = string.Format("Saved: %1 objects, %2 spots.", now.Count(), hs.m_aSpots.Count());
		if (outside > 0)
			said += string.Format(" %1 outside the site weren't saved.", outside);

		if (spotsOutside > 0)
			said += string.Format(" %1 spots outside the site weren't saved.", spotsOutside);

		if (bad > 0)
			said += string.Format(" %1 spots are red: %2.", bad, FirstProblem(s));

		if (missing > 0)
			said += " Still needs: " + check + ". Its people fall back where a spot is missing.";

		if (!hs.m_bUse)
			said += " It's not in use: switch it on to see it in play.";

		Say(s, said);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Building ends: everything put out or placed there goes, and the site is laid out again from what
	//! is in use (the save, or automated).
	protected static void Stop(RZ_SiteBuilderSession s)
	{
		RZ_SiteEntry site = s.m_Building;
		if (!site)
			return;

		array<IEntity> now = {};
		int outside;
		Capture(s, now, outside);
		foreach (IEntity put : s.m_aPut)
		{
			if (put && !now.Contains(put))
				now.Insert(put);
		}

		foreach (IEntity e : now)
		{
			Remove(e);
		}

		s.m_aPut.Clear();
		ClearSpots(s);
		s.m_aSuggest.Clear();
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(s.m_iPlayer);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_SITEMARKS, "");

		s.m_Building = null;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		gm.GetCampLife().StopBuilding(site.m_sId, site.m_vPos, site.m_fRadius);
		RZ_Log.Info(string.Format("Site builder: %1 stopped building %2 (%3); %4 objects cleared, the site is %5.", RZ_Util.PlayerName(s.m_iPlayer), site.m_sTitle, site.m_sId, now.Count(), StateText(StateOf(site))));
	}

	//------------------------------------------------------------------------------------------------
	//! An object goes: through Game Master's own delete when it's one of its entities.
	protected static void Remove(IEntity e)
	{
		if (!e)
			return;

		SCR_EditableEntityComponent ec = SCR_EditableEntityComponent.Cast(e.FindComponent(SCR_EditableEntityComponent));
		if (ec)
		{
			ec.Delete(); // VERIFY: deleting an editable entity on the server takes it out of every game master's view
			return;
		}

		SCR_EntityHelper.DeleteEntityAndChildren(e);
	}

	//------------------------------------------------------------------------------------------------
	//! The site from what's in use now (it isn't being built).
	protected static void Relay(RZ_SiteEntry site)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm.GetCampLife().IsBuilding(site.m_sId))
			return;

		gm.GetCampLife().StopBuilding(site.m_sId, site.m_vPos, site.m_fRadius);
	}

	// ================================================================ Role spots
	//------------------------------------------------------------------------------------------------
	//! A spot put down (its marker with it, when Camp life > Spot marker is set).
	protected static void AddSpot(RZ_SiteBuilderSession s, string type, vector pos, float yaw, int seats)
	{
		RZ_HandSpot sp = new RZ_HandSpot();
		sp.m_sType = type;
		sp.m_vPos = pos;
		sp.m_fYaw = yaw;
		sp.m_iSeats = seats;
		if (type == "meal" && seats <= 1)
			sp.m_iSeats = RZ_HandSpots.MEAL_SEATS;

		IEntity mark;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		ResourceName prefab;
		if (gm && gm.GetCampLife() && gm.GetCampLife().GetSettings())
			prefab = gm.GetCampLife().GetSettings().m_sSpotMarker;

		if (!prefab.IsEmpty())
			mark = RZ_Util.SpawnPrefab(prefab, pos, yaw); // VERIFY: an editable prop spawned by the server can be selected, moved and deleted in Game Master

		s.m_aSpots.Insert(sp);
		s.m_aMarks.Insert(mark);
		s.m_aMarked.Insert(mark != null);
	}

	//------------------------------------------------------------------------------------------------
	//! The place row: a spot of the chosen type at the builder's feet, facing the way they face.
	protected static void PlaceSpot(RZ_SiteBuilderSession s)
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayer);
		if (!user || !s.m_Building)
			return;

		array<string> types = {};
		RZ_HandSpots.Types(types);
		string type = types[s.m_iSpotType % types.Count()];
		vector pos = RZ_HandSpots.OnFloor(user.GetOrigin());
		float yaw = user.GetYawPitchRoll()[0];
		AddSpot(s, type, pos, yaw, 1);
		RZ_Log.Info(string.Format("Site builder: %1 put a %2 spot at %3 in %4.", RZ_Util.PlayerName(s.m_iPlayer), type, RZ_SettlementScanner.GridReference(pos), s.m_Building.m_sId));
		Say(s, RZ_HandSpots.Label(type) + " put down. Move its marker in Game Master if it should stand elsewhere.");
	}

	//------------------------------------------------------------------------------------------------
	protected static void RemoveSpot(RZ_SiteBuilderSession s, int idx)
	{
		if (idx < 0 || idx >= s.m_aSpots.Count())
			return;

		Remove(s.m_aMarks[idx]);
		s.m_aSpots.Remove(idx);
		s.m_aMarks.Remove(idx);
		s.m_aMarked.Remove(idx);
	}

	//------------------------------------------------------------------------------------------------
	protected static void ClearSpots(RZ_SiteBuilderSession s)
	{
		foreach (IEntity mark : s.m_aMarks)
		{
			Remove(mark);
		}

		s.m_aSpots.Clear();
		s.m_aMarks.Clear();
		s.m_aMarked.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! The spots follow their markers: moved in Game Master, the spot moves; deleted, the spot goes.
	static void SyncSpots(RZ_SiteBuilderSession s)
	{
		for (int i = s.m_aSpots.Count() - 1; i >= 0; i--)
		{
			IEntity mark = s.m_aMarks[i];
			if (!mark)
			{
				if (s.m_aMarked[i])
				{
					s.m_aSpots.Remove(i);
					s.m_aMarks.Remove(i);
					s.m_aMarked.Remove(i);
				}

				continue;
			}

			s.m_aSpots[i].m_vPos = mark.GetOrigin();
			s.m_aSpots[i].m_fYaw = mark.GetYawPitchRoll()[0];
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static int NearestSpot(RZ_SiteBuilderSession s)
	{
		SyncSpots(s);
		vector me = PlayerPos(s.m_iPlayer);
		int best = -1;
		float bestDist = float.MAX;
		foreach (int i, RZ_HandSpot sp : s.m_aSpots)
		{
			float d = RZ_Util.DistXZ(sp.m_vPos, me);
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! How many spots stand wrong (outside, in a wall, on another, in the way in).
	protected static int Problems(RZ_SiteBuilderSession s)
	{
		if (!s.m_Building)
			return 0;

		int n = 0;
		foreach (int i, RZ_HandSpot sp : s.m_aSpots)
		{
			if (!RZ_HandSpots.Problem(sp, s.m_Building.m_vPos, s.m_Building.m_fRadius, s.m_aSpots, s.m_aMarks[i]).IsEmpty())
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The first wrong spot, in words: "Seat, 6 m east of you: in a wall or a prop".
	protected static string FirstProblem(RZ_SiteBuilderSession s)
	{
		vector me = PlayerPos(s.m_iPlayer);
		foreach (int i, RZ_HandSpot sp : s.m_aSpots)
		{
			string why = RZ_HandSpots.Problem(sp, s.m_Building.m_vPos, s.m_Building.m_fRadius, s.m_aSpots, s.m_aMarks[i]);
			if (why.IsEmpty())
				continue;

			return string.Format("%1, %2 m %3 of you: %4", RZ_HandSpots.Label(sp.m_sType), RZ_Util.RoundToInt(RZ_Util.DistXZ(me, sp.m_vPos)), RZ_Util.DirectionText(me, sp.m_vPos), why);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The builder's client draws the site's edge and its spots: green, red, and yellow where a missing
	//! spot is suggested (RZ_SiteBuilderMarks).
	protected static void SendMarks(RZ_SiteBuilderSession s)
	{
		RZ_SiteEntry site = s.m_Building;
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(s.m_iPlayer);
		if (!site || !ui)
			return;

		vector c = RZ_Util.GroundPos(site.m_vPos);
		string payload = string.Format("R|%1|%2|%3|%4", c[0], c[1], c[2], site.m_fRadius);
		foreach (int i, RZ_HandSpot sp : s.m_aSpots)
		{
			int state = 0;
			string problem = RZ_HandSpots.Problem(sp, site.m_vPos, site.m_fRadius, s.m_aSpots, s.m_aMarks[i]);
			if (!problem.IsEmpty())
				state = 1;

			problem.Replace("|", " ");
			problem.Replace(";", ",");
			// The cone's network id: the label follows the cone while it's dragged in Game Master
			string rpl = "";
			if (s.m_aMarks[i])
			{
				RplComponent rc = RplComponent.Cast(s.m_aMarks[i].FindComponent(RplComponent));
				if (rc)
					rpl = rc.Id().ToString();
			}

			payload += string.Format(";S|%1|%2|%3|%4|%5|%6|%7|%8", sp.m_sType, sp.m_vPos[0], sp.m_vPos[1], sp.m_vPos[2], state, problem, sp.m_iSeats, rpl);
		}

		// One suggestion for each kind of spot still short of what the site needs
		array<ref RZ_HandNeed> needs = {};
		int missing;
		RZ_HandSpots.Check(site.m_eFaction, site.m_eKind, s.m_aSpots, needs, missing);
		foreach (RZ_HandNeed n : needs)
		{
			if (n.m_iHave >= n.m_iWant)
				continue;

			foreach (RZ_HandSpot sug : s.m_aSuggest)
			{
				if (sug.m_sType != n.m_sType || RZ_Util.DistXZ(sug.m_vPos, site.m_vPos) > site.m_fRadius)
					continue;

				payload += string.Format(";S|%1|%2|%3|%4|2", sug.m_sType, sug.m_vPos[0], sug.m_vPos[1], sug.m_vPos[2]);
				break;
			}
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_SITEMARKS, payload);
	}

	//------------------------------------------------------------------------------------------------
	//! Pins the site here every season (the season's roll places it first), or lets it go.
	protected static void SwitchPin(RZ_SiteBuilderSession s)
	{
		RZ_SiteEntry site = SiteById(s.m_sSelected);
		if (!site)
			return;

		RZ_HandBuilt store = RZ_HandBuilt.Get();
		RZ_HandBuiltSite saved = store.ForPlace(site.m_eKind, site.m_vPos);
		if (!saved)
			return;

		store.SetPinned(saved.m_sId, !saved.m_bPinned, site.m_eFaction);
		RZ_Log.Info(string.Format("Hand-built: %1 (%2) pinned %3.", site.m_sTitle, saved.m_sId, saved.m_bPinned));
		if (saved.m_bPinned)
			Say(s, "Pinned: from the next season (or a fresh save) the site stands here every time. Ship it with the tool to keep it in the mod.");
		else
			Say(s, "Not pinned: the season places this site again; the save is used while it stands here.");
	}

	//------------------------------------------------------------------------------------------------
	protected static void SwitchUse(RZ_SiteBuilderSession s)
	{
		RZ_SiteEntry site = SiteById(s.m_sSelected);
		if (!site)
			return;

		RZ_HandBuilt store = RZ_HandBuilt.Get();
		RZ_HandBuiltSite saved = store.ForPlace(site.m_eKind, site.m_vPos);
		if (!saved)
			return;

		store.SetUse(saved.m_sId, !saved.m_bUse);
		Relay(site);
		RZ_Log.Info(string.Format("Hand-built: %1 (%2) is %3 now.", site.m_sTitle, saved.m_sId, StateText(StateOf(site))));
		if (saved.m_bUse)
			Say(s, "The hand-built site is in use.");
		else
			Say(s, "The automated layout is in use. The hand-built site is kept.");
	}

	//------------------------------------------------------------------------------------------------
	protected static void Restore(RZ_SiteBuilderSession s, bool confirmed)
	{
		RZ_SiteEntry site = SiteById(s.m_sSelected);
		if (!site)
			return;

		if (!confirmed)
		{
			s.m_sArmed = "restore";
			return;
		}

		RZ_HandBuilt store = RZ_HandBuilt.Get();
		RZ_HandBuiltSite saved = store.ForPlace(site.m_eKind, site.m_vPos);
		string id = site.m_sId;
		if (saved)
			id = saved.m_sId;

		bool building = s.m_Building && s.m_Building.m_sId == site.m_sId;
		if (building)
			Stop(s);

		if (!store.RestoreLast(id))
		{
			Say(s, "There's no earlier save.");
			return;
		}

		RZ_Log.Info(string.Format("Hand-built: %1 (%2) restored to its save before.", site.m_sTitle, id));
		if (building)
			Start(s, site, "saved");
		else
			Relay(site);

		Say(s, "The save before is back.");
	}

	//------------------------------------------------------------------------------------------------
	protected static void Delete(RZ_SiteBuilderSession s, bool confirmed)
	{
		RZ_SiteEntry site = SiteById(s.m_sSelected);
		if (!site)
			return;

		if (!confirmed)
		{
			s.m_sArmed = "delete";
			return;
		}

		RZ_HandBuilt store = RZ_HandBuilt.Get();
		RZ_HandBuiltSite saved = store.ForPlace(site.m_eKind, site.m_vPos);
		if (!saved)
			return;

		if (s.m_Building && s.m_Building.m_sId == site.m_sId)
			Stop(s);

		store.Delete(saved.m_sId);
		Relay(site);
		RZ_Log.Info(string.Format("Hand-built: %1 (%2) deleted; it's automated again.", site.m_sTitle, saved.m_sId));
		Say(s, "Deleted. Camp life lays the site out again.");
	}
}
