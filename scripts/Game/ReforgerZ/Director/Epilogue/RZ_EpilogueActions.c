//! The epilogue's player actions without typing (docs/UI.md, Controllers): on the notebook item,
//! "The season, as the Zone tells it" (/season) and "Keep my name out of it, or put it back"
//! (/anonymous). Everything else in the week is a choice on a person or a table (RZ_EpiSpots), a
//! leader's reply, or RZ_LeaderAction.

//------------------------------------------------------------------------------------------------
enum RZ_EEpiNotebook
{
	SEASON,
	ANONYMOUS
}

//------------------------------------------------------------------------------------------------
//! On the notebook item (next to RZ_DeadNameAction).
class RZ_EpiNotebookAction : RZ_GroupedAction
{
	[Attribute("0", UIWidgets.ComboBox, "What this action does: the season view, or the names opt-out", "", ParamEnumArray.FromEnum(RZ_EEpiNotebook))]
	protected RZ_EEpiNotebook m_eKind;

	//------------------------------------------------------------------------------------------------
	override string MenuTitle(IEntity owner)
	{
		return "Your name and the season";
	}

	//------------------------------------------------------------------------------------------------
	override bool Label(out string outName)
	{
		if (m_eKind == RZ_EEpiNotebook.ANONYMOUS)
			outName = "My name in the world: keep it out, or put it back";
		else
			outName = "The season, as the Zone tells it";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		if (m_eKind == RZ_EEpiNotebook.ANONYMOUS)
		{
			RZ_EpiActions.Anonymous(gm, playerId, -1);
			return;
		}

		if (gm.GetEpilogue())
			gm.NotifyPlayer(playerId, "The season", gm.GetEpilogue().SeasonText(playerId));
	}
}

//------------------------------------------------------------------------------------------------
class RZ_EpiActions
{
	//------------------------------------------------------------------------------------------------
	//! /anonymous (on 1, off 0, -1 the other way): stored by identity in hall_of_fame.json, so it
	//! lasts across seasons.
	static void Anonymous(RZ_GameModeComponent gm, int playerId, int on)
	{
		RZ_HallOfFame hall = gm.GetHall();
		if (!hall)
		{
			gm.NotifyPlayer(playerId, "Your name", "There's no hall of fame on this server to keep you out of.");
			return;
		}

		bool anon = hall.SetAnonymous(gm.GetProfile(playerId).m_sIdentity, on);
		if (anon)
			gm.NotifyPlayer(playerId, "Your name", "From now on the world calls you a survivor: no boards, no reel, no hall of fame. Switch it back the same way.");
		else
			gm.NotifyPlayer(playerId, "Your name", "Your name goes back where you earned it: boards, the reel and the hall of fame.");
	}

	//------------------------------------------------------------------------------------------------
	//! /lastentry <text>: your line on the Names board, filtered, 80 characters.
	static void LastEntry(RZ_GameModeComponent gm, int playerId, string text)
	{
		RZ_Epilogue epi = gm.GetEpilogue();
		if (!epi || !epi.ClosingOn())
		{
			gm.NotifyPlayer(playerId, "The Names board", "There's no board to write on yet.");
			return;
		}

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent || RZ_Util.DistXZ(ent.GetOrigin(), epi.GetSpots().NamesPos()) > 30)
		{
			gm.NotifyPlayer(playerId, "The Names board", "You'll have to be at the board to write on it. It stands at the bend on the north road.");
			return;
		}

		epi.GetClosing().WriteLine(playerId, text);
	}
}
