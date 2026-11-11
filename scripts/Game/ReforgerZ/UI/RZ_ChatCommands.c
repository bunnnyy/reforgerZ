//! Chat commands (client): "/note your text" pins a note on the safehouse notice board you're at,
//! "/noteclear" and "/noteclear all" clear boards (server admins), "/tribes" writes the infected to
//! the server log (admins). The hall of fame is the notebook's Hall of fame tab.
//! Each is sent to the server through RZ_PlayerUIComponent.SendCommand; the server checks
//! everything again. Registered once, from the first notice the client receives
//! (RZ_GameModeComponent.QueueHint), so it works without any layout.
class RZ_ChatCommands
{
	protected static ref RZ_ChatCommands s_Instance;

	//------------------------------------------------------------------------------------------------
	static void Register()
	{
		if (s_Instance)
			return;

		SCR_ChatPanelManager chat = SCR_ChatPanelManager.GetInstance();
		if (!chat)
			return;

		s_Instance = new RZ_ChatCommands();
		chat.GetCommandInvoker("note").Insert(s_Instance.OnNote);
		chat.GetCommandInvoker("noteclear").Insert(s_Instance.OnNoteClear);
		chat.GetCommandInvoker("tribes").Insert(s_Instance.OnTribes);
	}

	//------------------------------------------------------------------------------------------------
	protected void Send(int cmd, string text)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendCommand(cmd, text);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNote(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_NOTE, data);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNoteClear(SCR_ChatPanel panel, string data)
	{
		string arg = data;
		arg.ToLower();
		if (arg.Contains("all"))
			Send(RZ_PlayerUIComponent.CMD_CLEAR_ALL, "");
		else
			Send(RZ_PlayerUIComponent.CMD_CLEAR_BOARD, "");
	}

	//------------------------------------------------------------------------------------------------
	//! "/tribes" (admins): the infected to the server log; "/tribes stir" and "/tribes site" are
	//! play-test aids (docs/INFECTED.md, Admin visibility).
	protected void OnTribes(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_TRIBES, data);
	}
}
