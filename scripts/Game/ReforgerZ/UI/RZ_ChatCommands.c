//! Chat commands (client): "/note your text" pins a note on the safehouse notice board you're at,
//! "/noteclear" and "/noteclear all" clear boards (server admins), "/tribes" writes the infected to
//! the server log (admins), "/talkdebug" reads the conversation trees of the person you're talking
//! to (admins), "/traces" reads the door marks near you to the server log (admins), "/mydead
//! hidden|shown" keeps your name out of public text if you die out there,
//! "/season", "/anonymous [off]" and "/lastentry <text>" (the epilogue week), "/epilogue ..." (admins),
//! "/sites" opens the site builder (admins, game masters), "/fog [all|reset]" the map's fog (admins), "/level [0-3]" your guidance level (admins), "/rep <name>" reads a player's reputation (server admins;
//! docs/REPUTATION.md, "Reputations"). The hall of fame is the notebook's Hall of fame tab.
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
		chat.GetCommandInvoker("rep").Insert(s_Instance.OnRep);
		chat.GetCommandInvoker("talkdebug").Insert(s_Instance.OnTalkDebug);
		chat.GetCommandInvoker("mydead").Insert(s_Instance.OnMyDead);
		chat.GetCommandInvoker("epilogue").Insert(s_Instance.OnEpilogue);
		chat.GetCommandInvoker("season").Insert(s_Instance.OnSeason);
		chat.GetCommandInvoker("anonymous").Insert(s_Instance.OnAnonymous);
		chat.GetCommandInvoker("lastentry").Insert(s_Instance.OnLastEntry);
		chat.GetCommandInvoker("traces").Insert(s_Instance.OnTraces);
		chat.GetCommandInvoker("sites").Insert(s_Instance.OnSites);
		chat.GetCommandInvoker("fog").Insert(s_Instance.OnFog);
		chat.GetCommandInvoker("level").Insert(s_Instance.OnLevel);
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
	protected void OnRep(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_REP, data);
	}

	//------------------------------------------------------------------------------------------------
	//! "/talkdebug" (admins): conversation trees, read for the person you're talking to (docs/DIALOGUE.md).
	protected void OnTalkDebug(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_TALKDEBUG, data);
	}

	//------------------------------------------------------------------------------------------------
	//! "/mydead hidden" or "/mydead shown" (or just "/mydead" to switch): your name in public text if
	//! you die out there (docs/INFECTED.md, Your dead run). The notebook item's action does the same.
	protected void OnMyDead(SCR_ChatPanel panel, string data)
	{
		string arg = data;
		arg.ToLower();
		Send(RZ_PlayerUIComponent.CMD_MYDEAD, arg);
	}

	//------------------------------------------------------------------------------------------------
	//! "/epilogue ..." (admins): the epilogue week's tools (docs/EPILOGUE.md, Admin tools).
	protected void OnEpilogue(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_EPILOGUE, data);
	}

	//------------------------------------------------------------------------------------------------
	//! "/season": the season as the Zone tells it (the notebook item's action does the same).
	protected void OnSeason(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_SEASON, data);
	}

	//------------------------------------------------------------------------------------------------
	//! "/anonymous" or "/anonymous off" (the notebook item's action switches it).
	protected void OnAnonymous(SCR_ChatPanel panel, string data)
	{
		string arg = data;
		arg.ToLower();
		Send(RZ_PlayerUIComponent.CMD_ANON, arg);
	}

	//------------------------------------------------------------------------------------------------
	//! "/lastentry your line": your line on the Names board (at the board, "Write your line" does it
	//! without typing).
	protected void OnLastEntry(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_LASTENTRY, data);
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
	//! "/traces" (admins): the marked doors within 300 m to the server log; "/traces town <name>",
	//! "/traces write <symbol>" (World > Traces > Debug), "/traces clear <town>" (docs/TRACES.md).
	protected void OnTraces(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_TRACES, data);
	}

	//------------------------------------------------------------------------------------------------
	//! "/sites" (admins, game masters): the site builder (docs/SITES.md, "Hand-built sites"). Also the
	//! talk screen's [Admin] list and Game Master's context menu, without typing.
	protected void OnSites(SCR_ChatPanel panel, string data)
	{
		RZ_Log.Info("Site builder: /sites typed, asking the server.");
		Send(RZ_PlayerUIComponent.CMD_SITES, data);
	}

	//------------------------------------------------------------------------------------------------
	//! "/fog" (admins): the map's fog read-out; "/fog all" clears your map, "/fog reset" fogs it again
	//! (World/RZ_MapFog.c). Also the talk screen's [Admin] list, without typing.
	protected void OnFog(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_FOG, data);
	}

	//------------------------------------------------------------------------------------------------
	//! "/level" (admins): your guidance level; "/level 0" to "/level 3" sets it from your next character
	//! (Guidance/RZ_Guidance.c). Also the talk screen's [Admin] list, without typing.
	protected void OnLevel(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_LEVEL, data);
	}

	//------------------------------------------------------------------------------------------------
	//! "/tribes" (admins): the infected to the server log; "/tribes stir" and "/tribes site" are
	//! play-test aids (docs/INFECTED.md, Admin visibility).
	protected void OnTribes(SCR_ChatPanel panel, string data)
	{
		Send(RZ_PlayerUIComponent.CMD_TRIBES, data);
	}
}
