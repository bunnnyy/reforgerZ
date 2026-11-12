//! Callbacks at the door (docs/DESIGN.md, "Callbacks"): a story job's callback can be a person, not
//! only a line. When one marked for the door comes due (RZ_StoryKit.Later with atDoor), a stand-in
//! (the civilian puzzle prefab) turns up a few metres from the player, within 300 m of the callback's
//! place, and says it; they go after 90 s. One at a time on the server; no prefab, or the player
//! further off: the line alone, as any callback. Words only: no gifts, no checks. Server only.
class RZ_DoorCallers
{
	static const string MARK = "door|";
	static const float RANGE = 300;
	static const float STAY_SECONDS = 90;

	protected static IEntity s_Caller;
	protected static float s_fSince;

	//------------------------------------------------------------------------------------------------
	//! A callback's title with the door mark: the title without it, and true.
	static bool Strip(string title, out string plain)
	{
		plain = title;
		if (!title.StartsWith(MARK))
			return false;

		plain = title.Substring(MARK.Length(), title.Length() - MARK.Length());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A door callback is due for this player at `at`: a stand-in says it when there's room for one.
	static void Say(RZ_GameModeComponent gm, int playerId, string title, string text, vector at)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		ResourceName prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (!me || s_Caller || prefab.IsEmpty() || RZ_Util.DistXZ(me.GetOrigin(), at) > RANGE)
		{
			gm.NotifyPlayerAs(playerId, title, text, RZ_ENoticeTag.AUTO);
			return;
		}

		s_Caller = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(me.GetOrigin(), 4, 7)), Math.RandomFloat(0, 360));
		s_fSince = 0;
		if (s_Caller)
			RZ_PuzzleObjectComponent.Tag(s_Caller, -1, RZ_TalkScene.OBJECT_INDEX, "", "");

		gm.NotifyPlayerAs(playerId, title, text, RZ_ENoticeTag.AUTO);
		RZ_Log.Info(string.Format("Callbacks: %1 at the door for %2", title, RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! On the callbacks' tick: the caller goes after 90 s.
	static void Tick(float dt)
	{
		if (!s_Caller)
			return;

		s_fSince += dt;
		if (s_fSince < STAY_SECONDS)
			return;

		RZ_Util.DeleteUnit(s_Caller);
		s_Caller = null;
	}
}
