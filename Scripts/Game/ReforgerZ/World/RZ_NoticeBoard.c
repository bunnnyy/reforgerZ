//! Safehouse notice boards (docs/DESIGN.md, "Haven safehouses"): short notes players leave for each
//! other between sessions. Cosmetic and social only: a note changes nothing in the game.
//!
//! - Read: "Read the notice board" (RZ_SafehouseAction BOARD_READ) at any safehouse.
//! - Write: type "/note your text" in chat while at a safehouse, or use the board menu's box
//!   (UI > Board menus). "Leave a note on the board" says how.
//! - The newest few per board, a length limit, one note per player every few minutes, the author's
//!   name, a word filter, and notes come down by themselves after a few real days. Saved with the
//!   safehouse. Admins clear a board with "/noteclear" (or every board with "/noteclear all").
//! Server only, except the payload the board menu reads.

//! One note on a board. Plain fields: saved as they are.
class RZ_Note
{
	string author;
	string ident;
	string text;
	int unix;
}

//------------------------------------------------------------------------------------------------
class RZ_NoticeBoard
{
	protected static const float EXPIRE_INTERVAL = 600;
	protected static const float AT_BOARD = 60;		// a note is pinned at the safehouse you're at
	protected static const int DAY = 86400;

	protected RZ_GameModeComponent m_GM;
	protected RZ_SafehouseSettings m_Settings;
	protected RZ_Safehouses m_Houses;
	protected ref array<string> m_aFilter = {};
	protected float m_fExpireTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_NoticeBoard(RZ_GameModeComponent gm, RZ_SafehouseSettings settings, RZ_Safehouses houses)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Houses = houses;
		RZ_ItemHelper.Words(settings.m_sNoteFilter, m_aFilter);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_Settings.m_bEnabled && m_Settings.m_bNoticeBoards;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Title(RZ_Safehouse h)
	{
		return "Notice board: " + h.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! Every few minutes: old notes come down.
	void Tick(float dt)
	{
		m_fExpireTimer += dt;
		if (m_fExpireTimer < EXPIRE_INTERVAL)
			return;

		m_fExpireTimer = 0;
		for (int i = 0; i < m_Houses.Count(); i++)
		{
			Expire(m_Houses.Get(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Expire(RZ_Safehouse h)
	{
		if (!h)
			return;

		int now = RZ_Guidance.UnixNow();
		int keep = RZ_Util.RoundToInt(Math.Max(0.1, m_Settings.m_fNoteDays) * DAY);
		for (int n = h.m_aNotes.Count() - 1; n >= 0; n--)
		{
			RZ_Note note = h.m_aNotes[n];
			if (!note || (now > 0 && note.unix > 0 && now - note.unix > keep))
				h.m_aNotes.RemoveOrdered(n);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "2 days ago", "3 hours ago", "just now".
	protected static string AgeText(int unix)
	{
		int now = RZ_Guidance.UnixNow();
		if (now <= 0 || unix <= 0)
			return "some time ago";

		int seconds = Math.Max(0, now - unix);
		if (seconds < 3600)
			return "just now";

		if (seconds < DAY)
			return string.Format("%1 hour(s) ago", seconds / 3600);

		return string.Format("%1 day(s) ago", seconds / DAY);
	}

	// ================================================================ Reading
	//------------------------------------------------------------------------------------------------
	//! From "Read the notice board": the board menu (UI > Board menus) or one notice.
	void Read(int playerId, int idx)
	{
		RZ_Safehouse h = m_Houses.Get(idx);
		if (!h)
			return;

		if (!IsEnabled())
		{
			m_GM.NotifyPlayer(playerId, "Safehouse: " + h.m_sName, "Somebody took the board down for firewood.");
			return;
		}

		Expire(h);
		if (SendMenu(playerId))
			return;

		string text = "";
		for (int n = h.m_aNotes.Count() - 1; n >= 0; n--)
		{
			RZ_Note note = h.m_aNotes[n];
			text += string.Format("%1 (%2): %3\n", note.author, AgeText(note.unix), note.text);
		}

		if (text.IsEmpty())
			text = "Nothing pinned up but an old water rota and a child's drawing of a house.";

		m_GM.NotifyPlayer(playerId, Title(h), text);
	}

	//------------------------------------------------------------------------------------------------
	//! From "Leave a note on the board": the menu with its box, or how to do it in chat.
	void HowToWrite(int playerId, int idx)
	{
		RZ_Safehouse h = m_Houses.Get(idx);
		if (!h || !IsEnabled())
			return;

		if (SendMenu(playerId))
			return;

		m_GM.NotifyPlayer(playerId, Title(h), string.Format("There's a pencil on a string and a few scraps of paper.\n(Type /note and your message in chat while you're here: %1 characters at most, one note every %2 minutes. Everyone who comes by can read it for %3 days.)",
			m_Settings.m_iNoteLength, RZ_Util.RoundToInt(m_Settings.m_fNoteGapMinutes), RZ_Util.RoundToInt(m_Settings.m_fNoteDays)));
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the board to the player's menu if board menus are on. False = use notices.
	protected bool SendMenu(int playerId)
	{
		if (!RZ_UISettings.Get().m_bBoardMenus)
			return false;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!ui)
			return false;

		ui.SendPayload(RZ_UIFeed.PAYLOAD_BOARD, BuildPayload(playerId));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The board the player stands at, -1 if none.
	protected int BoardAt(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent)
			return -1;

		return m_Houses.FindNearest(ent.GetOrigin(), AT_BOARD);
	}

	//------------------------------------------------------------------------------------------------
	//! For the board menu (RZ_UIFeed, PAYLOAD_BOARD), the board the player stands at:
	//! B|safehouse name|notes|longest note|can write (0/1)
	//! N|author|age|text                newest first
	string BuildPayload(int playerId)
	{
		RZ_Safehouse h = m_Houses.Get(BoardAt(playerId));
		if (!h || !IsEnabled())
			return "";

		Expire(h);
		string canWrite = "1";
		if (m_Houses.IsExiled(playerId))
			canWrite = "0";

		RZ_UILine head = new RZ_UILine("B");
		string s = head.Add(h.m_sName).AddInt(h.m_aNotes.Count()).AddInt(m_Settings.m_iNoteLength).Add(canWrite).End();
		for (int n = h.m_aNotes.Count() - 1; n >= 0; n--)
		{
			RZ_Note note = h.m_aNotes[n];
			RZ_UILine ln = new RZ_UILine("N");
			s += ln.Add(note.author).Add(AgeText(note.unix)).Add(note.text).End();
		}

		return s;
	}

	// ================================================================ Writing
	//------------------------------------------------------------------------------------------------
	//! "/note ..." in chat or the menu's box: pins a note at the safehouse the player is at.
	void Post(int playerId, string raw)
	{
		if (!IsEnabled() || playerId <= 0)
			return;

		int idx = BoardAt(playerId);
		RZ_Safehouse h = m_Houses.Get(idx);
		if (!h)
		{
			m_GM.NotifyPlayer(playerId, "Notice board", "There's no board here. Every Haven safehouse has one by the door.");
			return;
		}

		if (m_Houses.IsExiled(playerId))
		{
			m_GM.NotifyPlayer(playerId, Title(h), "Someone takes the paper out of your hand and tears it in half.");
			return;
		}

		string text = RZ_UIText.Clean(Filter(raw), Math.Max(20, m_Settings.m_iNoteLength));
		if (text.IsEmpty() || text == " ")
			return;

		RZ_PlayerProfile profile = m_GM.FindProfile(playerId);
		if (!profile || profile.m_sIdentity.IsEmpty())
			return;

		// One note every few minutes per player, on any board (survives restarts: notes are saved)
		int now = RZ_Guidance.UnixNow();
		int gap = RZ_Util.RoundToInt(Math.Max(0, m_Settings.m_fNoteGapMinutes) * 60);
		int last = LastNoteBy(profile.m_sIdentity);
		if (gap > 0 && last > 0 && now > 0 && now - last < gap)
		{
			m_GM.NotifyPlayer(playerId, Title(h), string.Format("You've only just pinned one up. Give it %1 minute(s).", Math.Max(1, (gap - (now - last) + 59) / 60)));
			return;
		}

		RZ_Note note = new RZ_Note();
		note.author = RZ_Util.PlayerName(playerId);
		note.ident = profile.m_sIdentity;
		note.text = text;
		note.unix = now;
		h.m_aNotes.Insert(note);
		int cap = Math.Max(1, m_Settings.m_iBoardNotes);
		while (h.m_aNotes.Count() > cap)
		{
			h.m_aNotes.RemoveOrdered(0);
		}

		RZ_Log.Info(string.Format("Note at %1 by %2: %3", h.m_sName, note.author, note.text));
		if (!SendMenu(playerId))
			m_GM.NotifyPlayer(playerId, Title(h), "You pin it up between a missing-persons photo and the water rota.");
	}

	//------------------------------------------------------------------------------------------------
	//! Real-world time of this player's newest note on any board, 0 if none.
	protected int LastNoteBy(string ident)
	{
		int last = 0;
		for (int i = 0; i < m_Houses.Count(); i++)
		{
			RZ_Safehouse h = m_Houses.Get(i);
			foreach (RZ_Note note : h.m_aNotes)
			{
				if (note.ident == ident && note.unix > last)
					last = note.unix;
			}
		}

		return last;
	}

	//------------------------------------------------------------------------------------------------
	//! Words from the filter (Haven > Note filter) blanked out with asterisks, whatever their case.
	protected string Filter(string text)
	{
		if (m_aFilter.IsEmpty())
			return text;

		string result = text;
		string lower = text;
		lower.ToLower();
		foreach (string word : m_aFilter)
		{
			int wlen = word.Length();
			if (wlen == 0)
				continue;

			string stars = "";
			for (int k = 0; k < wlen; k++)
			{
				stars += "*";
			}

			int i = 0;
			while (i + wlen <= lower.Length())
			{
				if (lower.Substring(i, wlen) != word)
				{
					i++;
					continue;
				}

				int restLen = lower.Length() - i - wlen;
				result = result.Substring(0, i) + stars + result.Substring(i + wlen, restLen);
				lower = lower.Substring(0, i) + stars + lower.Substring(i + wlen, restLen);
				i += wlen;
			}
		}

		return result;
	}

	// ================================================================ Admins
	//------------------------------------------------------------------------------------------------
	//! "/noteclear" (the board the admin stands at, or the nearest within 1 km) or "/noteclear all".
	void Clear(int playerId, bool all)
	{
		if (!IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Notice board", "Only a server admin can clear a board.");
			return;
		}

		int cleared = 0;
		if (all)
		{
			for (int i = 0; i < m_Houses.Count(); i++)
			{
				RZ_Safehouse house = m_Houses.Get(i);
				cleared += house.m_aNotes.Count();
				house.m_aNotes.Clear();
			}

			m_GM.NotifyPlayer(playerId, "Notice board", string.Format("Every board cleared (%1 notes).", cleared));
			RZ_Log.Info(string.Format("An admin cleared every notice board (%1 notes).", cleared));
			return;
		}

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_Safehouse h;
		if (ent)
			h = m_Houses.Get(m_Houses.FindNearest(ent.GetOrigin(), 1000));

		if (!h)
		{
			m_GM.NotifyPlayer(playerId, "Notice board", "No safehouse within 1 km. Stand near one, or use /noteclear all.");
			return;
		}

		cleared = h.m_aNotes.Count();
		h.m_aNotes.Clear();
		m_GM.NotifyPlayer(playerId, Title(h), string.Format("Cleared (%1 notes).", cleared));
		RZ_Log.Info(string.Format("An admin cleared the notice board at %1 (%2 notes).", h.m_sName, cleared));
	}

	//------------------------------------------------------------------------------------------------
	static bool IsAdmin(int playerId)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm || playerId <= 0)
			return false;

		return pm.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR) || pm.HasPlayerRole(playerId, EPlayerRole.SESSION_ADMINISTRATOR); // VERIFY: PlayerManager.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR / SESSION_ADMINISTRATOR)
	}
}
