//! The radio screen, server side (docs/UI.md, "The radio screen"). "View radio" opens it: on a radio
//! item (RZ_ViewRadioAction, beside vanilla's Turn on/off while it's held up, or on the ground) and on
//! a house radio (its fourth option, RZ_SecureRadio.HandleHouseRadio). The server sends what's on
//! the air now on each channel; the client adds the log it already keeps for the notebook's Radio
//! tab (RZ_PlayerUIComponent.GetRadioLog) and which secure channel you've cracked (the HUD's feed).
//! It changes nothing in the game: it only shows.
//!
//! Payload (RZ_UIFeed.PAYLOAD_RADIO): V|title|sub|powered, then C|kind|faction|name|state|line per
//! channel (the broadcast, the four open channels, the three secure ones; kind is RZ_ENoticeKind,
//! state and line empty for a secure channel, which the client fills); X|reason closes; F|why: the
//! server couldn't open one.

//------------------------------------------------------------------------------------------------
//! "View radio": on a radio item's ActionsManagerComponent (the wind-up radio, any radio you want
//! it on). Put one in the "knob_toggle" context with In hand on, beside vanilla's Turn on/off, and
//! one in the "default" context for the radio on the ground.
class RZ_ViewRadioAction : ScriptedUserAction
{
	[Attribute("0", desc: "1: shown on the radio held up in hand and looked at (vanilla's inspection; use the knob_toggle context, beside Turn on/off). 0: shown on the radio lying on the ground (the default context)")]
	protected bool m_bInHand;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = RZ_RadioScreen.VIEW_LABEL;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Only with the radio screen (UI > Radio channel layout, and it opens on this machine).
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner || !user || !RZ_RadioMenu.Available())
			return false;

		// VERIFY: a carried radio has a parent (the character, or a vest or pack on it); one on the ground has none
		bool carried = owner.GetParent() != null;
		if (!m_bInHand)
			return !carried;

		// In hand: while it's held up and looked at, as vanilla's Turn on/off (SCR_RadioToggleUserAction)
		CharacterControllerComponent control = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return carried && control && control.GetInspect();
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: the server runs PerformAction for an action on a carried item in the inspection
	//! context (vanilla's Turn on/off is server-side the same way)
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		RZ_RadioScreen.Open(gm, playerId, pOwnerEntity, "Radio", true);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_RadioScreen
{
	static const string VIEW_LABEL = "View radio";

	//------------------------------------------------------------------------------------------------
	//! UI > Radio channel layout is set (the client checks the same, with its own fallbacks).
	static bool Available()
	{
		return !RZ_UISettings.Get().m_sRadioChannelLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the radio screen on `radio` for the player. `powered`: a house set without power pulls
	//! in two open channels, as its Tune does. False when it isn't used or the server is at its cap
	//! of open screens (the client is told, and says the log is in the notebook).
	static bool Open(RZ_GameModeComponent gm, int playerId, IEntity radio, string title, bool powered)
	{
		if (!gm || !radio || playerId <= 0 || !Available())
			return false;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = gm.GetScreenGuard();
		if (!ui || !guard)
			return false;

		// From the object menu (a house radio's row): the radio screen takes its place
		bool fromMenu = RZ_ObjectScreen.IsOpenFor(playerId);
		if (!guard.Open(playerId, RZ_ScreenGuard.KIND_RADIO, radio, title, -1))
		{
			ui.SendPayload(RZ_UIFeed.PAYLOAD_RADIO, "F|busy");
			return false;
		}

		if (fromMenu)
		{
			RZ_ObjectScreen.OnScreenGone(playerId);
			ui.SendPayload(RZ_UIFeed.PAYLOAD_OBJECT, "X|replaced");
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_RADIO, Build(gm, radio, title, powered));
		RZ_Log.Info(string.Format("Radio screen: %1 for player %2.", title, playerId));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Build(RZ_GameModeComponent gm, IEntity radio, string title, bool powered)
	{
		string sub = "";
		RZ_Settlement here = gm.GetSettlement(gm.FindNearestSettlement(radio.GetOrigin()));
		if (here && RZ_Util.DistXZ(here.m_vCenter, radio.GetOrigin()) < 600)
			sub = here.m_sName;

		int power = 0;
		if (powered)
			power = 1;

		string payload = string.Format("V|%1|%2|%3", RZ_UIText.Clean(title, 60), RZ_UIText.Clean(sub, 60), power);

		// The government recording, always there
		string broadcast = "";
		if (gm.GetBroadcast())
			broadcast = gm.GetBroadcast().GetText();

		payload += Channel(RZ_ENoticeKind.BROADCAST, -1, "Government broadcast", "ON AIR", broadcast);

		// The open channels: what each says now; a house set without power pulls in two
		RZ_FactionLife life = gm.GetFactionLife();
		array<int> open = {RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE};
		int heard = 0;
		foreach (int f : open)
		{
			string line = "";
			if (life && life.IsActive())
				line = life.ChatterFor(f);

			string state = "QUIET";
			if (!line.IsEmpty() && !powered && heard >= 2)
			{
				state = "TOO FAINT";
				line = "";
			}
			else if (!line.IsEmpty())
			{
				state = "ON AIR";
				heard++;
			}

			payload += Channel(RZ_ENoticeKind.OPEN_CHANNEL, f, "Open channel: " + RZ_Factions.GetShortName(f), state, line);
		}

		// The secure channels: the client knows the one you've cracked, and what it said
		array<int> secure = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE};
		foreach (int g : secure)
		{
			payload += Channel(RZ_ENoticeKind.SECURE_CHANNEL, g, RZ_Util.Capitalize(RZ_SecureRadio.ChannelName(g)), "", "");
		}

		return payload;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Channel(int kind, int faction, string name, string state, string line)
	{
		return string.Format("\nC|%1|%2|%3|%4|%5", kind, faction, RZ_UIText.Clean(name, 60), state, RZ_UIText.Clean(line, 400));
	}
}
