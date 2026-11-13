//------------------------------------------------------------------------------------------------
//! Nobody disappears in sight (the author, 2026-11-13: a duty officer vanished the moment a scene
//! ended). A person a job, a scene or an errand no longer needs is handed here instead of deleted:
//! with no player within SEEN m they go at once; otherwise they walk off, away from the nearest
//! player, and are put away once nobody is within SEEN m (or, after GIVE_UP s, once nobody is
//! within NEAR m). A lone character goes into an empty group for the walk, as a wanderer does.
//! Props, the dead and anything without AI are deleted as before.
class RZ_Leaving
{
	protected static const ResourceName GROUP = "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et";
	protected static const float SEEN = 120;		// a player this near would see them vanish
	protected static const float NEAR = 40;			// after GIVE_UP: never put away closer than this
	protected static const float WALK = 150;		// how far they walk off
	protected static const float GIVE_UP = 300;		// seconds walking before NEAR is enough
	protected static const int TICK_MS = 2000;

	protected static ref array<IEntity> s_aBodies;
	protected static ref array<IEntity> s_aGroups;		// the group made for the walk (null: their own)
	protected static ref array<float> s_aSince;
	protected static float s_fClock;
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	//! Removes `e` (a character, a group or a prop) without anyone seeing it vanish.
	static void Remove(IEntity e)
	{
		if (!e)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		vector at = RZ_Util.GetUnitPosition(e);
		bool person = ChimeraCharacter.Cast(e) || SCR_AIGroup.Cast(e);
		if (!person || RZ_Util.CountAlive(e) == 0 || RZ_Util.NearestDistance(at, players) > SEEN)
		{
			RZ_Util.DeleteUnit(e);
			return;
		}

		IEntity group = e;
		IEntity made = null;
		if (ChimeraCharacter.Cast(e))
		{
			group = RZ_Util.GroupOf(e);
			if (!group)
			{
				made = RZ_Util.WrapInGroup(e, GROUP);
				group = made;
			}
		}

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!group || !gm || !gm.GetSpawner())
		{
			// Nothing to walk them with: they stay put until nobody's near
			Track(e, null);
			return;
		}

		// Away from the nearest player, on land
		vector from = at;
		float best = float.MAX;
		foreach (IEntity p : players)
		{
			if (!p)
				continue;

			float d = RZ_Util.DistXZ(p.GetOrigin(), at);
			if (d < best)
			{
				best = d;
				from = p.GetOrigin();
			}
		}

		vector dir = at - from;
		dir[1] = 0;
		if (dir.Length() < 0.5)
			dir = Vector(0, 0, 1);

		dir.Normalize();
		vector away = RZ_Mission.LandPointNear(at + dir * WALK, 0, 30);
		RZ_Util.RedirectGroup(group, away, gm.GetSpawner().GetMoveWaypointPrefab());
		Track(e, made);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Track(IEntity e, IEntity made)
	{
		if (!s_aBodies)
		{
			s_aBodies = {};
			s_aGroups = {};
			s_aSince = {};
		}

		s_aBodies.Insert(e);
		s_aGroups.Insert(made);
		s_aSince.Insert(s_fClock);
		if (!s_bTicking)
		{
			s_bTicking = true;
			GetGame().GetCallqueue().CallLater(Tick, TICK_MS, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		s_fClock += TICK_MS * 0.001;
		if (!s_aBodies || s_aBodies.IsEmpty())
		{
			GetGame().GetCallqueue().Remove(Tick);
			s_bTicking = false;
			return;
		}

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		for (int i = s_aBodies.Count() - 1; i >= 0; i--)
		{
			IEntity e = s_aBodies[i];
			IEntity made = s_aGroups[i];
			bool gone = !e;
			if (!gone)
			{
				float near = RZ_Util.NearestDistance(RZ_Util.GetUnitPosition(e), players);
				gone = near > SEEN || (s_fClock - s_aSince[i] > GIVE_UP && near > NEAR);
				if (gone)
				{
					if (made)
						RZ_Util.UnwrapGroup(e, made);

					RZ_Util.DeleteUnit(e);
				}
			}

			if (!gone)
				continue;

			if (made)
				RZ_Util.DeleteUnit(made);

			s_aBodies.Remove(i);
			s_aGroups.Remove(i);
			s_aSince.Remove(i);
		}
	}
}
