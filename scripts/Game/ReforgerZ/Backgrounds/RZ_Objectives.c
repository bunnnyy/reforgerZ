enum RZ_EObjective
{
	REACH_TOWN,			// travel to a specific town (background-flavoured reason)
	SURVIVE,			// stay alive N minutes
	DELIVER_SAMPLES,	// deliver N samples to the lab this life
	COMPLETE_MISSIONS,	// take part in N successful missions this life
	EARN_TRUST,			// become Trusted (or better) with a faction
	FIND_HAVEN,			// first meaningful life (new-player guidance): reach a Haven safehouse
	LAST_GOAL			// the epilogue week: the background's last goal (Director/Epilogue/RZ_EpilogueClosing.c)
}

//! One personal objective per life gives every character a small story of their
//! own on top of the server-wide missions. Completing one gives a Legacy point
//! (saved across lives, used for unlocks later) and a little reputation.
class RZ_Objectives
{
	static const float REACH_MIN_DIST = 1200;
	static const float REACH_MAX_DIST = 4000;
	static const float HAVEN_MIN_DIST = 200;

	//------------------------------------------------------------------------------------------------
	static void Assign(RZ_GameModeComponent gm, RZ_PlayerProfile profile, RZ_Background bg, vector spawnPos)
	{
		// A player's first life that counts is always about finding the Haven
		if (gm.GetGuidance() && gm.GetGuidance().WantsHavenGoal(profile))
		{
			int house = PickHavenGoal(gm, spawnPos);
			if (house >= 0)
			{
				profile.m_iObjType = RZ_EObjective.FIND_HAVEN;
				profile.m_iObjTarget = house;
				profile.m_iObjGoal = 0;
				profile.m_bObjDone = false;
				return;
			}
		}

		// The epilogue week: the background's last goal, finished through the closing work
		RZ_Epilogue epi = RZ_Epilogue.Get();
		if (epi && epi.GetClosing().WantsLastGoal(profile, bg.m_sId))
		{
			profile.m_iObjType = RZ_EObjective.LAST_GOAL;
			profile.m_iObjTarget = -1;
			profile.m_iObjGoal = 0;
			profile.m_bObjDone = false;
			return;
		}

		array<int> pool = {
			RZ_EObjective.REACH_TOWN, RZ_EObjective.REACH_TOWN,
			RZ_EObjective.SURVIVE, RZ_EObjective.COMPLETE_MISSIONS,
			RZ_EObjective.EARN_TRUST, RZ_EObjective.DELIVER_SAMPLES
		};

		// Lab techs lean toward research goals
		if (bg.m_fResearchMult > 1)
			pool.Insert(RZ_EObjective.DELIVER_SAMPLES);

		if (gm.GetCure().IsCured())
		{
			while (pool.Contains(RZ_EObjective.DELIVER_SAMPLES))
			{
				pool.RemoveItem(RZ_EObjective.DELIVER_SAMPLES);
			}
		}

		profile.m_iObjType = pool.GetRandomElement();
		profile.m_bObjDone = false;

		switch (profile.m_iObjType)
		{
			case RZ_EObjective.REACH_TOWN:
				profile.m_iObjTarget = PickDistantTown(gm, spawnPos);
				if (profile.m_iObjTarget < 0)
				{
					profile.m_iObjType = RZ_EObjective.SURVIVE;
					profile.m_iObjGoal = 45;
				}
				break;
			case RZ_EObjective.SURVIVE:
				profile.m_iObjGoal = 45;
				break;
			case RZ_EObjective.DELIVER_SAMPLES:
				profile.m_iObjGoal = 2;
				break;
			case RZ_EObjective.COMPLETE_MISSIONS:
				profile.m_iObjGoal = 2;
				break;
			case RZ_EObjective.EARN_TRUST:
				profile.m_iObjTarget = PickTrustFaction(bg);
				profile.m_iObjGoal = RZ_ETier.TRUSTED;
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	static string Describe(RZ_GameModeComponent gm, RZ_PlayerProfile profile)
	{
		switch (profile.m_iObjType)
		{
			case RZ_EObjective.REACH_TOWN:
			{
				RZ_Settlement s = gm.GetSettlement(profile.m_iObjTarget);
				if (!s)
					return "";

				return string.Format(GetReachFlavour(profile.m_sBackgroundId), s.m_sName);
			}
			case RZ_EObjective.SURVIVE:
				return string.Format("Stay alive for %1 minutes.", profile.m_iObjGoal);
			case RZ_EObjective.DELIVER_SAMPLES:
				return string.Format("Get %1 infected samples to Halcyon's lab.", profile.m_iObjGoal);
			case RZ_EObjective.COMPLETE_MISSIONS:
				return string.Format("Help out: be part of %1 successful missions.", profile.m_iObjGoal);
			case RZ_EObjective.EARN_TRUST:
				return string.Format("Earn the trust of %1: become %2 with them.", RZ_Factions.GetNameInText(profile.m_iObjTarget), RZ_Reputation.GetTierName(RZ_ETier.TRUSTED));
			case RZ_EObjective.LAST_GOAL:
				return RZ_EpiText.LastGoal(profile.m_sBackgroundId);
			case RZ_EObjective.FIND_HAVEN:
			{
				RZ_Safehouse goal = gm.GetSafehouses().Get(profile.m_iObjTarget);
				if (!goal)
					return "Find the Haven.";

				return string.Format("Find the Haven. People say the safehouse in %1 takes people in.", goal.m_sName);
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(RZ_GameModeComponent gm, int playerId, RZ_PlayerProfile profile, IEntity ent, float dt)
	{
		profile.m_fLifeSeconds += dt;
		if (profile.m_bObjDone || profile.m_iObjType < 0)
			return;

		bool complete = false;
		switch (profile.m_iObjType)
		{
			case RZ_EObjective.REACH_TOWN:
			{
				RZ_Settlement s = gm.GetSettlement(profile.m_iObjTarget);
				if (s && RZ_Arrive.InArea(ent, s.m_vCenter, s.m_fRadius))	// in the town, or at its edge
					complete = true;
				break;
			}
			case RZ_EObjective.SURVIVE:
				complete = profile.m_fLifeSeconds >= profile.m_iObjGoal * 60;
				break;
			case RZ_EObjective.DELIVER_SAMPLES:
				complete = profile.m_iLifeSamples >= profile.m_iObjGoal;
				break;
			case RZ_EObjective.COMPLETE_MISSIONS:
				complete = profile.m_iLifeMissions >= profile.m_iObjGoal;
				break;
			case RZ_EObjective.EARN_TRUST:
				complete = RZ_Reputation.IsAtLeast(profile.GetTier(profile.m_iObjTarget), RZ_ETier.TRUSTED);
				break;
			case RZ_EObjective.FIND_HAVEN:
				// Any safehouse will do: the Haven is the people, not the building
				complete = gm.GetSafehouses().FindAt(ent.GetOrigin()) >= 0;
				break;
		}

		if (!complete)
			return;

		profile.m_bObjDone = true;
		profile.m_iLegacy++;

		// No loot: a goal pays Legacy and a little Fame, so a fresh life can't farm crates
		gm.NotifyPlayer(playerId, "Personal goal complete", string.Format("%1\nLegacy: %2", Describe(gm, profile), profile.m_iLegacy));
		gm.AddFame(playerId, RZ_EFaction.SURVIVORS, 10);
		gm.OnLegacyChanged(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest safehouse that isn't where the player woke up (a safehouse spawn point
	//! would finish the goal at once). -1 if there's none.
	protected static int PickHavenGoal(RZ_GameModeComponent gm, vector from)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		int best = -1;
		float bestDist = float.MAX;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			float d = RZ_Util.DistXZ(from, h.m_vPos);
			if (d < HAVEN_MIN_DIST || d >= bestDist)
				continue;

			bestDist = d;
			best = i;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected static int PickDistantTown(RZ_GameModeComponent gm, vector from)
	{
		array<int> good = {};
		int farthest = -1;
		float farthestDist = 0;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			float d = RZ_Util.DistXZ(from, s.m_vCenter);
			if (d >= REACH_MIN_DIST && d <= REACH_MAX_DIST)
				good.Insert(i);

			if (d > farthestDist)
			{
				farthestDist = d;
				farthest = i;
			}
		}

		if (!good.IsEmpty())
			return good.GetRandomElement();

		// Small maps: just go somewhere that isn't here
		if (farthestDist > 300)
			return farthest;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A faction the background already leans toward, but isn't Trusted by yet.
	protected static int PickTrustFaction(RZ_Background bg)
	{
		array<int> leaning = {RZ_EFaction.BANDITS, RZ_EFaction.MILITARY, RZ_EFaction.INSTITUTE, RZ_EFaction.SURVIVORS};
		foreach (int f : leaning)
		{
			int startFame = bg.GetStartFame(f);
			if (startFame > 0 && startFame < 40)
				return f;
		}

		array<int> options = {};
		foreach (int g : leaning)
		{
			if (g != RZ_EFaction.BANDITS && bg.GetStartFame(g) < 40 && bg.GetStartInfamy(g) < 15)
				options.Insert(g);
		}

		if (options.IsEmpty())
			return RZ_EFaction.SURVIVORS;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	protected static string GetReachFlavour(string backgroundId)
	{
		switch (backgroundId)
		{
			case "nurse": return "Check whether anyone is still alive at the clinic in %1.";
			case "mechanic": return "Find your brother's garage in %1.";
			case "hunter": return "Make it to your hunting cabin near %1.";
			case "police": return "Report in at the station in %1.";
			case "excon": return "Go back for the stash you buried near %1. Someone may have beaten you to it.";
			case "labtech": return "Recover your research notes from %1.";
			case "radioham": return "Track down the signal coming out of %1.";
		}

		return "Find out what happened to your family in %1.";
	}
}
