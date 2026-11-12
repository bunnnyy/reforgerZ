//! Callbacks at the door (docs/DESIGN.md, "Callbacks"): a story job's callback can be a person, not
//! only a line. When one marked for the door comes due (RZ_StoryKit.Later with atDoor), a stand-in
//! (the civilian puzzle prefab) turns up 15-25 m from the player, out of their sight, within 300 m of
//! the callback's place, and says it; they go after 90 s, once nobody can see them. One at a time on
//! the server, inside the body budget (an asker); no prefab, no room, nowhere unseen, or the player
//! further off: the line alone, as any callback. Words only: no gifts, no checks. Server only.
class RZ_DoorCallers
{
	static const string MARK = "door|";
	static const float RANGE = 300;
	static const float STAY_SECONDS = 90;
	static const float STAY_MAX = 300;		// still in someone's sight this long: they go anyway
	static const float NEAR_MIN = 15;
	static const float NEAR_MAX = 25;

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

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		vector spot;
		RZ_Shared shared = RZ_Shared.Get();
		if (!RZ_InfectedBodies.FindHiddenSpot(me.GetOrigin(), NEAR_MIN, NEAR_MAX, players, spot) || (shared && !shared.CanSpawnBody(RZ_BodyBudget.ASKER, spot)))
		{
			gm.NotifyPlayerAs(playerId, title, text, RZ_ENoticeTag.AUTO);
			return;
		}

		s_Caller = RZ_Util.SpawnPrefab(prefab, spot, Math.RandomFloat(0, 360));
		s_fSince = 0;
		if (s_Caller)
		{
			RZ_PuzzleObjectComponent.Tag(s_Caller, -1, RZ_TalkScene.OBJECT_INDEX, "", "");
			if (shared)
				shared.AddBody(RZ_BodyBudget.ASKER, s_Caller);
		}

		gm.NotifyPlayerAs(playerId, title, text, RZ_ENoticeTag.AUTO);
		RZ_Log.Info(string.Format("Callbacks: %1 at the door for %2", title, RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! On the callbacks' tick: the caller goes after 90 s, once nobody can see them.
	static void Tick(float dt)
	{
		if (!s_Caller)
			return;

		s_fSince += dt;
		if (!GoneUnseen(s_Caller, s_fSince, STAY_SECONDS))
			return;

		RZ_Util.DeleteUnit(s_Caller);
		s_Caller = null;
	}

	//------------------------------------------------------------------------------------------------
	//! A caller or a visitor `age` s old may go: past `stay` and out of every player's sight (or past
	//! STAY_MAX, seen or not).
	static bool GoneUnseen(IEntity person, float age, float stay)
	{
		if (age < stay)
			return false;

		if (age >= STAY_MAX || !person)
			return true;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		return !RZ_InfectedBodies.InSight(person.GetOrigin(), players, true);
	}
}
