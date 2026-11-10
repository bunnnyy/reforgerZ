enum RZ_EObjective
{
	REACH_TOWN,			// travel to a specific town (background-flavoured reason)
	SURVIVE,			// stay alive N minutes
	DELIVER_SAMPLES,	// deliver N samples to the lab this life
	COMPLETE_MISSIONS,	// take part in N successful missions this life
	EARN_TRUST			// reach N reputation with a faction
}

//! One personal objective per life gives every character a small story of their
//! own on top of the server-wide missions. Completing one gives a Legacy point
//! (saved across lives, used for unlocks later) and a little reputation.
class RZ_Objectives
{
	static const float REACH_MIN_DIST = 1200;
	static const float REACH_MAX_DIST = 4000;

	//------------------------------------------------------------------------------------------------
	static void Assign(RZ_GameModeComponent gm, RZ_PlayerProfile profile, RZ_Background bg, vector spawnPos)
	{
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
				profile.m_iObjGoal = Math.Max(40, bg.GetStartRep(profile.m_iObjTarget) + 25);
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
				return string.Format("Get %1 infected samples to the Institute lab.", profile.m_iObjGoal);
			case RZ_EObjective.COMPLETE_MISSIONS:
				return string.Format("Help out: be part of %1 successful missions.", profile.m_iObjGoal);
			case RZ_EObjective.EARN_TRUST:
				return string.Format("Earn the trust of the %1 (reputation %2).", RZ_Factions.GetName(profile.m_iObjTarget), profile.m_iObjGoal);
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
				if (s && RZ_Util.DistXZ(ent.GetOrigin(), s.m_vCenter) <= s.m_fRadius * 0.6)
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
				complete = profile.GetRep(profile.m_iObjTarget) >= profile.m_iObjGoal;
				break;
		}

		if (!complete)
			return;

		profile.m_bObjDone = true;
		profile.m_iLegacy++;

		// Reaching the town turns up a stash: something personal left behind
		if (profile.m_iObjType == RZ_EObjective.REACH_TOWN)
			gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(ent.GetOrigin(), 3, 6));

		gm.NotifyPlayer(playerId, "Personal goal complete", string.Format("%1\nLegacy: %2", Describe(gm, profile), profile.m_iLegacy));
		gm.AddReputation(playerId, RZ_EFaction.SURVIVORS, 10);
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
	protected static int PickTrustFaction(RZ_Background bg)
	{
		if (bg.m_iRepBandits > 0)
			return RZ_EFaction.BANDITS;
		if (bg.m_iRepMilitary > 0)
			return RZ_EFaction.MILITARY;
		if (bg.m_iRepInstitute > 0)
			return RZ_EFaction.INSTITUTE;

		array<int> options = {RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_EFaction.INSTITUTE};
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
			case "excon": return "Dig up the stash you buried near %1.";
			case "labtech": return "Recover your research notes from %1.";
			case "radioham": return "Track down the signal coming out of %1.";
		}

		return "Find out what happened to your family in %1.";
	}
}
