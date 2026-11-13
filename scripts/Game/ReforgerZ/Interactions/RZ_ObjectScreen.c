//! The object menu (docs/UI.md, "The object menu"): one small generic screen for props and places
//! with several options or something to read. Instead of a row of world prompts, one prompt opens
//! it: a title, what you see (the read's text) and the options as a list, pad first (D-pad, A, B).
//!
//! It isn't a system of its own: it lists an entity's own actions. Every action that can sit in it
//! derives from RZ_GroupedAction and splits its old code into Label, Visible and Run. On one entity:
//! - the puzzle slots (RZ_ReadClueAction, RZ_PuzzleUseAction, RZ_PuzzleChoiceAction, RZ_KeypadAction)
//!   form a menu when the puzzle object says so (RZ_PuzzleObjectComponent.WantsMenu: a prop with
//!   labelled slots, never a person; world interaction spots set it by kind, RZ_Interactions.UsesMenu);
//!   the read is the opener and its text is the menu's description;
//! - any other grouped actions (safehouse services, a quartermaster's asks, the lab, the notebook
//!   item's name and season, a radio item's channels, hand-overs) form one when two or more show;
//!   the first is the opener and is labelled with the group's title ("Ask the quartermaster").
//! While the menu is used (UI > Object row layout set, and it opened on this machine) the other
//! members hide. The opener's PerformAction opens the screen on the server (RZ_ObjectScreen.TryOpen);
//! a pick comes back as an index and runs that action's own Run, on the server, with what it says
//! caught for the status line. Without the slot, or when the client can't open the menu, every
//! action is a world prompt as before.
//!
//! A menu can also be fed rows as data (RZ_ObjectRows, OpenRows): a work site's choice, where one
//! prompt at a spot stands for several things to do there (RZ_WorkSites.Gather: a duty beat's
//! options, a sorting game's subject). The rows' own Pick decides what a row does.
//!
//! Payload (RZ_UIFeed.PAYLOAD_OBJECT): O|serial|title|sub|text|status|working|left|total|read label|
//! hint, then S|label|value|state|left per status tile (RZ_ObjectStatus) and R|index|label|enabled|
//! reason|tag per row (enabled 0: locked, with the full reason and a short tag); X|reason closes;
//! F|why: the server couldn't open one. Line breaks in the text travel as ^.

//------------------------------------------------------------------------------------------------
//! An action that can be a row of the object menu. Subclasses implement Label, Visible and Run
//! (not GetActionNameScript, CanBeShownScript and PerformAction, which this class owns).
class RZ_GroupedAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	//! The action's own name. False: the prefab's name.
	bool Label(out string outName)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Shown at all for this user (the action's own rule, before any menu hides it).
	bool Visible(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! What it does. Server only: from the world prompt, or a pick in the object menu.
	void Run(IEntity pOwnerEntity, IEntity pUserEntity)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! False: never a row (a quick action, or one that opens a screen of its own).
	bool InMenu()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A puzzle slot: grouped only when the puzzle object wants a menu.
	bool IsSlot()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The read: when it opens the menu its text is the description, not a row.
	bool Describes()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The menu's title when this action opens it.
	string MenuTitle(IEntity owner)
	{
		return "Options";
	}

	//------------------------------------------------------------------------------------------------
	//! Can't be done now: a locked row in the object menu, with the full reason (said in the status
	//! line when it's picked) and a short tag in capitals (NEEDS POWER). By default the action's own
	//! CanBePerformed and reason (RZ_ObjectStatus.TagFor). Server only.
	bool Locked(IEntity user, out string reason, out string tag)
	{
		if (CanBePerformed(user))
			return false;

		reason = GetCannotPerformReason();
		tag = RZ_ObjectStatus.TagFor(reason);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The object menu's status strip when this action opens it: tiles, hint and a label over the
	//! read (RZ_ObjectStatus). None by default. Server only.
	void MenuStatus(IEntity owner, IEntity user, RZ_ObjectStatus status)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Hidden by another screen (the talk, car repair or crafting screen holds these slots). On a
	//! person with a Talk prompt (RZ_TalkAction: a quartermaster, the safehouse medic), the talk
	//! screen holds every one of them.
	protected bool HiddenByScreens()
	{
		return RZ_TalkMenu.HidesActionsOn(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (RZ_ActionMenus.IsTitledOpener(this))
		{
			outName = RZ_ActionMenus.Title(GetOwner(), this);
			return true;
		}

		return Label(outName);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!Visible(user) || HiddenByScreens())
			return false;

		return !RZ_ActionMenus.Hides(this, user);
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (RZ_ObjectScreen.TryOpen(this, pOwnerEntity, pUserEntity))
			return;

		Run(pOwnerEntity, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
//! Which of an entity's actions form a menu, and which one opens it. The same on every machine
//! (labels and the puzzle object's flag are replicated); the client adds whether it uses the menu.
class RZ_ActionMenus
{
	protected static ref array<BaseUserAction> s_aList = {};
	protected static ref array<RZ_GroupedAction> s_aCache = {};
	protected static IEntity s_CacheOwner;
	protected static IEntity s_CacheUser;
	protected static float s_fCacheAt = -1;
	protected static bool s_bCacheMenu;

	//------------------------------------------------------------------------------------------------
	//! The members on `owner` for `user`, in the actions manager's order; true if they form a menu.
	static bool Members(IEntity owner, IEntity user, notnull array<RZ_GroupedAction> members)
	{
		members.Clear();
		if (!owner)
			return false;

		ActionsManagerComponent actions = ActionsManagerComponent.Cast(owner.FindComponent(ActionsManagerComponent));
		if (!actions)
			return false;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(owner.FindComponent(RZ_PuzzleObjectComponent));
		bool slots = comp && comp.WantsMenu();
		s_aList.Clear();
		actions.GetActionsList(s_aList);
		int rows = 0;
		foreach (BaseUserAction a : s_aList)
		{
			RZ_GroupedAction ga = RZ_GroupedAction.Cast(a);
			if (!ga || !ga.InMenu() || (ga.IsSlot() && !slots) || !ga.Visible(user))
				continue;

			members.Insert(ga);
			if (!ga.Describes())
				rows++;
		}

		if (slots)
			return rows > 0;

		return rows > 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The members, cached for one frame (CanBeShown asks once per action per frame).
	protected static bool Cached(IEntity owner, IEntity user)
	{
		float now = -1;
		if (GetGame().GetWorld())
			now = GetGame().GetWorld().GetWorldTime();

		if (now >= 0 && owner == s_CacheOwner && user == s_CacheUser && now == s_fCacheAt)
			return s_bCacheMenu;

		s_CacheOwner = owner;
		s_CacheUser = user;
		s_fCacheAt = now;
		s_bCacheMenu = Members(owner, user, s_aCache);
		return s_bCacheMenu;
	}

	//------------------------------------------------------------------------------------------------
	//! The action that opens the menu: the read if there is one, else the first member.
	static RZ_GroupedAction Opener(notnull array<RZ_GroupedAction> members)
	{
		foreach (RZ_GroupedAction ga : members)
		{
			if (ga.Describes())
				return ga;
		}

		if (members.IsEmpty())
			return null;

		return members[0];
	}

	//------------------------------------------------------------------------------------------------
	//! This machine shows `action`'s entity as a menu, and `action` isn't its opener: hidden.
	static bool Hides(RZ_GroupedAction action, IEntity user)
	{
		// A dedicated server hides nothing: a client without the menu may use any member
		if (!action.InMenu() || RplSession.Mode() == RplMode.Dedicated || !RZ_ObjectMenu.Available())
			return false;

		IEntity owner = action.GetOwner();
		if (!Cached(owner, user))
			return false;

		if (!s_aCache.Contains(action))
			return false;

		return Opener(s_aCache) != action;
	}

	//------------------------------------------------------------------------------------------------
	//! `action` opens a menu without a read: it carries the group's title instead of its own name.
	static bool IsTitledOpener(RZ_GroupedAction action)
	{
		if (action.Describes() || !action.InMenu() || !RZ_ObjectMenu.Available())
			return false;

		IEntity user = RZ_UIW.LocalEntity();
		IEntity owner = action.GetOwner();
		if (!user || !Cached(owner, user))
			return false;

		return Opener(s_aCache) == action;
	}

	//------------------------------------------------------------------------------------------------
	//! The menu's title: a quartermaster's are asks; else the opener's own.
	static string Title(IEntity owner, RZ_GroupedAction opener)
	{
		if (owner && owner.FindComponent(RZ_QuartermasterComponent))
			return "Ask the quartermaster";

		return opener.MenuTitle(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! "Look at the stove" -> "The stove"; "Listen to the radio" -> "The radio".
	static string TitleFromRead(string label)
	{
		array<string> verbs = {"Look at ", "Look over ", "Read ", "Listen to ", "Check ", "Examine ", "Inspect ", "Use "};
		string t = label;
		foreach (string v : verbs)
		{
			if (t.StartsWith(v))
			{
				t = t.Substring(v.Length(), t.Length() - v.Length());
				break;
			}
		}

		if (t.IsEmpty())
			return label;

		string first = t.Substring(0, 1);
		first.ToUpper();
		return first + t.Substring(1, t.Length() - 1);
	}
}

//------------------------------------------------------------------------------------------------
//! Rows given as data rather than an entity's actions (RZ_ObjectScreen.OpenRows). Server only.
class RZ_ObjectRows
{
	//------------------------------------------------------------------------------------------------
	//! The rows now, in order (their labels as the prompt said them).
	void Labels(notnull array<string> labels) {}

	//------------------------------------------------------------------------------------------------
	//! Row `index` can't be done now: why, in full, and a short tag.
	bool Locked(int index, IEntity user, out string reason, out string tag) { return false; }

	//------------------------------------------------------------------------------------------------
	//! Row `index` picked by `user` (checked: it's there and not locked). What it says goes to the
	//! menu's status line; it may close the menu.
	void Pick(int index, IEntity user) {}

	//------------------------------------------------------------------------------------------------
	//! The description under the title, as it stands now.
	string Text() { return ""; }

	//------------------------------------------------------------------------------------------------
	//! This player's client couldn't open the menu: the rows become world prompts for them.
	void Failed(int playerId) {}
}

//------------------------------------------------------------------------------------------------
//! The object menu, server side: who has one open, on what, and what a pick does.
class RZ_ObjectScreen
{
	protected static const int MAX_ROWS = 16;

	protected static ref map<int, IEntity> s_mOwner;
	protected static ref map<int, int> s_mSerial;
	protected static ref map<int, string> s_mTitle;
	protected static ref map<int, string> s_mText;
	protected static ref array<int> s_aNoMenu;		// players whose client couldn't open it: prompts for them
	protected static ref map<int, ref RZ_ObjectRows> s_mRows;	// a menu fed as data (OpenRows)
	protected static int s_iNextSerial = 1;
	protected static int s_iWrapping = -1;

	//------------------------------------------------------------------------------------------------
	//! UI > Object row layout is set (the client checks the same).
	static bool Available()
	{
		return !RZ_UISettings.Get().m_sObjectRowLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected static void Ensure()
	{
		if (s_mOwner)
			return;

		s_mOwner = new map<int, IEntity>();
		s_mSerial = new map<int, int>();
		s_mTitle = new map<int, string>();
		s_mText = new map<int, string>();
		s_mRows = new map<int, ref RZ_ObjectRows>();
		s_aNoMenu = {};
	}

	//------------------------------------------------------------------------------------------------
	//! A menu can be opened for this player now: the slot is set and their client hasn't failed one.
	static bool MenuFor(int playerId)
	{
		if (!Available())
			return false;

		Ensure();
		return !s_aNoMenu.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens a menu of data rows on `owner` (the thing the prompt is on) for `playerId`, titled
	//! `title`. False: no menu for them (no slot, their client failed one, or another screen is up).
	static bool OpenRows(int playerId, IEntity owner, string title, RZ_ObjectRows rows)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !owner || !rows || playerId <= 0 || !MenuFor(playerId))
			return false;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = gm.GetScreenGuard();
		if (!ui || !guard)
			return false;

		if (!guard.Open(playerId, RZ_ScreenGuard.KIND_OBJECT, owner, title, -1))
		{
			ui.SendPayload(RZ_UIFeed.PAYLOAD_OBJECT, "F|busy");
			return false;
		}

		string sub = "";
		RZ_Settlement here = gm.GetSettlement(gm.FindNearestSettlement(owner.GetOrigin()));
		if (here && RZ_Util.DistXZ(here.m_vCenter, owner.GetOrigin()) < 600)
			sub = here.m_sName;

		s_mOwner.Set(playerId, owner);
		s_mSerial.Set(playerId, s_iNextSerial);
		s_iNextSerial++;
		s_mTitle.Set(playerId, title + RZ_UIText.SEP + sub);
		s_mText.Set(playerId, rows.Text());
		s_mRows.Set(playerId, rows);
		Send(gm, playerId, "");
		RZ_Log.Info(string.Format("Object menu: %1 for player %2 (a choice).", title, playerId));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Closes the player's menu (a row was picked that's done in the world now).
	static void CloseFor(int playerId)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && IsOpenFor(playerId))
			Close(gm, playerId, "done");
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOpenFor(int playerId)
	{
		return s_mOwner && s_mOwner.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_GroupedAction.PerformAction (server): opens the menu when `action` is its opener.
	//! False: the action runs as a world prompt (no menu here, or the player's client can't show one).
	static bool TryOpen(RZ_GroupedAction action, IEntity owner, IEntity user)
	{
		if (!Available() || !action.InMenu())
			return false;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(user);
		if (!gm || playerId <= 0)
			return false;

		Ensure();
		if (s_aNoMenu.Contains(playerId))
			return false;

		array<RZ_GroupedAction> members = {};
		if (!RZ_ActionMenus.Members(owner, user, members) || RZ_ActionMenus.Opener(members) != action)
			return false;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = gm.GetScreenGuard();
		if (!ui || !guard)
			return false;

		// The read first: what it says is the description (a person, a paper or a screen of its own
		// takes over instead)
		string text = "";
		string title = RZ_ActionMenus.Title(owner, action);
		if (action.Describes())
		{
			guard.Forget(playerId);
			RZ_ScreenCapture.Begin(playerId);
			action.Run(owner, user);
			text = RZ_ScreenCapture.End();
			if (guard.IsAnyOpen(playerId))
				return true;
		}

		string sub = "";
		RZ_Settlement here = gm.GetSettlement(gm.FindNearestSettlement(owner.GetOrigin()));
		if (here && RZ_Util.DistXZ(here.m_vCenter, owner.GetOrigin()) < 600)
			sub = here.m_sName;

		if (!guard.Open(playerId, RZ_ScreenGuard.KIND_OBJECT, owner, title, -1))
		{
			ui.SendPayload(RZ_UIFeed.PAYLOAD_OBJECT, "F|busy");
			if (action.Describes())
			{
				if (!text.IsEmpty())
					gm.NotifyPlayer(playerId, title, text);

				return true;
			}

			return false;
		}

		s_mOwner.Set(playerId, owner);
		s_mSerial.Set(playerId, s_iNextSerial);
		s_iNextSerial++;
		s_mTitle.Set(playerId, title + RZ_UIText.SEP + sub);
		s_mText.Set(playerId, text);
		Send(gm, playerId, "");
		RZ_Log.Info(string.Format("Object menu: %1 for player %2.", title, playerId));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The screen guard or the player closed it.
	static void OnScreenGone(int playerId)
	{
		if (!s_mOwner)
			return;

		s_mOwner.Remove(playerId);
		s_mSerial.Remove(playerId);
		s_mTitle.Remove(playerId);
		s_mText.Remove(playerId);
		s_mRows.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The player's client couldn't open the menu (no preset or layout): world prompts for them
	//! from now on, and the opener runs as a prompt now.
	static void OnClientFailed(int playerId)
	{
		Ensure();
		if (!s_aNoMenu.Contains(playerId))
			s_aNoMenu.Insert(playerId);

		IEntity owner;
		if (!s_mOwner.Find(playerId, owner) || !owner)
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (gm && gm.GetScreenGuard())
			gm.GetScreenGuard().Close(playerId);

		// Data rows: their owner turns them into prompts
		RZ_ObjectRows data;
		if (s_mRows.Find(playerId, data) && data)
		{
			OnScreenGone(playerId);
			data.Failed(playerId);
			return;
		}

		array<RZ_GroupedAction> members = {};
		RZ_ActionMenus.Members(owner, user, members);
		RZ_GroupedAction opener = RZ_ActionMenus.Opener(members);
		OnScreenGone(playerId);
		if (opener && user && !opener.Describes())
			opener.Run(owner, user);
	}

	//------------------------------------------------------------------------------------------------
	//! A player left the server.
	static void Forget(int playerId)
	{
		if (!s_mOwner)
			return;

		OnScreenGone(playerId);
		s_aNoMenu.RemoveItem(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The rows: the members that aren't the read.
	protected static void Rows(IEntity owner, IEntity user, notnull array<RZ_GroupedAction> rows)
	{
		array<RZ_GroupedAction> members = {};
		RZ_ActionMenus.Members(owner, user, members);
		rows.Clear();
		foreach (RZ_GroupedAction ga : members)
		{
			if (!ga.Describes() && rows.Count() < MAX_ROWS)
				rows.Insert(ga);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the player's menu as it stands now, with `status` (what the last pick said).
	static void Send(RZ_GameModeComponent gm, int playerId, string status)
	{
		IEntity owner;
		int serial;
		if (!s_mOwner || !s_mOwner.Find(playerId, owner) || !s_mSerial.Find(playerId, serial))
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ui)
			return;

		if (!owner || !user)
		{
			Close(gm, playerId, "gone");
			return;
		}

		// A timed action under way (boiling, cracking a channel, pumping): its countdown
		string working = "";
		float left = 0;
		float total = 0;
		RZ_Interactions hub = RZ_Interactions.Get();
		if (hub)
		{
			RZ_TimedTask t = hub.GetTask(playerId);
			if (t)
			{
				working = t.m_sLabel;
				left = t.m_fLeft;
				total = t.m_fTotal;
			}
		}

		string head;
		string text;
		s_mTitle.Find(playerId, head);
		s_mText.Find(playerId, text);
		array<string> parts = {};
		RZ_UIText.Split(head, RZ_UIText.SEP, parts);

		// The status strip: the thing's tiles, what would change them, and a label over the read
		RZ_ObjectRows data;
		s_mRows.Find(playerId, data);
		RZ_ObjectStatus st = new RZ_ObjectStatus();
		if (data)
		{
			text = data.Text();
		}
		else
		{
			array<RZ_GroupedAction> members = {};
			RZ_ActionMenus.Members(owner, user, members);
			RZ_ObjectStatus.Fill(owner, user, playerId, RZ_ActionMenus.Opener(members), st);
		}

		string payload = "O" + RZ_UIText.SEP + serial.ToString() + RZ_UIText.SEP + RZ_UIText.Clean(RZ_UIText.Str(parts, 0), 80)
			+ RZ_UIText.SEP + RZ_UIText.Clean(RZ_UIText.Str(parts, 1), 80) + RZ_UIText.SEP + Flat(text, 1400)
			+ RZ_UIText.SEP + Flat(status, 500) + RZ_UIText.SEP + RZ_UIText.Clean(working, 60)
			+ RZ_UIText.SEP + RZ_Util.RoundToInt(left).ToString() + RZ_UIText.SEP + RZ_Util.RoundToInt(total).ToString();
		payload += RZ_UIText.SEP + RZ_UIText.Clean(st.m_sReadLabel, 60) + RZ_UIText.SEP + RZ_UIText.Clean(st.m_sHint, 120);
		payload += st.Records();

		// The rows: one it can't do now is locked, with its full reason and a short tag
		if (data)
		{
			array<string> labels = {};
			data.Labels(labels);
			foreach (int r, string rowLabel : labels)
			{
				if (r >= MAX_ROWS)
					break;

				int rowOn = 1;
				string rowReason = "";
				string rowTag = "";
				if (data.Locked(r, user, rowReason, rowTag))
					rowOn = 0;

				payload += "\nR" + RZ_UIText.SEP + r.ToString() + RZ_UIText.SEP + RZ_UIText.Clean(rowLabel, 90) + RZ_UIText.SEP + rowOn.ToString()
					+ RZ_UIText.SEP + RZ_UIText.Clean(rowReason, 200) + RZ_UIText.SEP + RZ_UIText.Clean(rowTag, 16);
			}

			ui.SendPayload(RZ_UIFeed.PAYLOAD_OBJECT, payload);
			return;
		}

		array<RZ_GroupedAction> rows = {};
		Rows(owner, user, rows);
		foreach (int i, RZ_GroupedAction ga : rows)
		{
			string label;
			if (!ga.Label(label))
				label = ga.GetActionName();

			int on = 1;
			string reason = "";
			string tag = "";
			if (ga.Locked(user, reason, tag))
				on = 0;

			payload += "\nR" + RZ_UIText.SEP + i.ToString() + RZ_UIText.SEP + RZ_UIText.Clean(label, 90) + RZ_UIText.SEP + on.ToString()
				+ RZ_UIText.SEP + RZ_UIText.Clean(reason, 200) + RZ_UIText.SEP + RZ_UIText.Clean(tag, 16);
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_OBJECT, payload);
	}

	//------------------------------------------------------------------------------------------------
	//! Text for one field: line breaks as ^, no separators.
	protected static string Flat(string text, int maxLen)
	{
		string t = text;
		t.Replace("\n", "^");
		return RZ_UIText.Clean(t, maxLen);
	}

	//------------------------------------------------------------------------------------------------
	//! A pick (RZ_PlayerUIComponent.RpcAsk_ObjectPick). Checked here: the menu is open on this serial,
	//! the thing is there and the player beside it; then the row's own Run decides.
	static void Pick(RZ_GameModeComponent gm, int playerId, int serial, int index)
	{
		IEntity owner;
		int current;
		if (!s_mOwner || !s_mOwner.Find(playerId, owner) || !s_mSerial.Find(playerId, current) || serial != current)
			return;

		RZ_ScreenGuard guard = gm.GetScreenGuard();
		if (!guard || !guard.IsOpen(playerId, RZ_ScreenGuard.KIND_OBJECT))
		{
			OnScreenGone(playerId);
			return;
		}

		guard.Touch(playerId);
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!owner || !user || !RZ_Util.IsCharacterAlive(user))
		{
			Close(gm, playerId, "gone");
			return;
		}

		if (RZ_Util.DistXZ(user.GetOrigin(), owner.GetOrigin()) > RZ_DialogueSettings.Get().m_fRange)
			return;

		// Data rows: their own Pick, after the same checks
		RZ_ObjectRows data;
		if (s_mRows.Find(playerId, data) && data)
		{
			PickData(gm, playerId, data, index, user);
			return;
		}

		array<RZ_GroupedAction> rows = {};
		Rows(owner, user, rows);
		if (index < 0 || index >= rows.Count())
			return;

		// A locked row does nothing: its full reason goes to the status line
		RZ_GroupedAction row = rows[index];
		string reason;
		string tag;
		if (row.Locked(user, reason, tag))
		{
			Send(gm, playerId, reason);
			return;
		}

		RZ_ScreenCapture.Begin(playerId);
		row.Run(owner, user);
		string said = RZ_ScreenCapture.End();

		// The pick opened a screen of its own (the talk screen, the reader): that one is open now
		if (!guard.IsOpen(playerId, RZ_ScreenGuard.KIND_OBJECT))
		{
			OnScreenGone(playerId);
			return;
		}

		if (s_mOwner.Contains(playerId))
			Send(gm, playerId, said);
		else if (!said.IsEmpty())
			gm.NotifyPlayer(playerId, RZ_ScreenCapture.LastTitle(), said);
	}

	//------------------------------------------------------------------------------------------------
	protected static void PickData(RZ_GameModeComponent gm, int playerId, RZ_ObjectRows data, int index, IEntity user)
	{
		array<string> labels = {};
		data.Labels(labels);
		if (index < 0 || index >= labels.Count() || index >= MAX_ROWS)
			return;

		string reason;
		string tag;
		if (data.Locked(index, user, reason, tag))
		{
			Send(gm, playerId, reason);
			return;
		}

		RZ_ScreenCapture.Begin(playerId);
		data.Pick(index, user);
		string said = RZ_ScreenCapture.End();
		if (s_mOwner.Contains(playerId))
			Send(gm, playerId, said);
		else if (!said.IsEmpty())
			gm.NotifyPlayer(playerId, RZ_ScreenCapture.LastTitle(), said);
	}

	//------------------------------------------------------------------------------------------------
	//! Around a timed action's end or stop (RZ_Interactions.TickTasks): what it says goes to the
	//! player's open menu instead of a card.
	static void WrapBegin(int playerId)
	{
		if (!IsOpenFor(playerId))
			return;

		s_iWrapping = playerId;
		RZ_ScreenCapture.Begin(playerId);
	}

	//------------------------------------------------------------------------------------------------
	static void WrapEnd(int playerId)
	{
		if (s_iWrapping != playerId)
			return;

		s_iWrapping = -1;
		string said = RZ_ScreenCapture.End();
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return;

		if (IsOpenFor(playerId))
			Send(gm, playerId, said);
		else if (!said.IsEmpty())
			gm.NotifyPlayer(playerId, RZ_ScreenCapture.LastTitle(), said);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Close(RZ_GameModeComponent gm, int playerId, string reason)
	{
		OnScreenGone(playerId);
		if (gm.GetScreenGuard())
			gm.GetScreenGuard().Close(playerId);

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_OBJECT, "X|" + reason);
	}
}
