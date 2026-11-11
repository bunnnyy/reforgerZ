//! Heat hordes and favourite-spot lookers from the packs (docs/INFECTED.md, "Hordes and lookers from
//! the packs"). Inside a pack's range the horde is the pack's own: by day its day party only (so its
//! cap of 6 bodies near a player cluster stands: nothing new is spawned), at night its hunters and
//! young out on their trails, inside the night's commitment. Lookers sent to a favourite spot are a
//! young group of its at night. Nothing is spawned here: the parts are sent as data (their noise),
//! and get bodies the usual way near players. Server only.

//------------------------------------------------------------------------------------------------
class RZ_PackHordes
{
	static const float SEND_REACH = 1500;	// m: a part this close to the heat is sent
	static const float HORDE_HOLD = 240;	// s it waits at the heat once there
	static const float LOOK_HOLD = 120;		// s lookers stay at a busy spot

	//------------------------------------------------------------------------------------------------
	//! The pack whose range holds pos (its range, not tonight's hunting range), or null.
	static RZ_Tribe RangeAt(RZ_Tribes tribes, vector pos)
	{
		if (!tribes || !tribes.IsEnabled())
			return null;

		RZ_Tribe best;
		float bestD = float.MAX;
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (t.IsRover())
				continue;

			float d = RZ_Util.DistXZ(pos, t.RangeCentre());
			if (d <= t.m_fRadius && d < bestD)
			{
				best = t;
				bestD = d;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A heat horde at hot. True when hot is inside a pack's range: the horde is that pack's (by day
	//! its day party, at night its hunting groups out on their trails), and no town's comes, even
	//! when the pack has nothing free to send. sent: the groups sent.
	static bool HeatHorde(RZ_GameModeComponent gm, vector hot, out int sent)
	{
		sent = 0;
		RZ_Tribes tribes = gm.GetTribes();
		RZ_Tribe t = RangeAt(tribes, hot);
		if (!t)
			return false;

		bool night = tribes.GetPhase() == RZ_ETribePhase.NIGHT;
		// The night's commitment: no vigil, not weakened, not deaf to where a hunt lost someone, and
		// nothing more while its hunt runs
		if (t.m_bVigil || t.IsWeakened() || t.m_Hunt || RZ_TribeBodies.DeafTo(t, hot))
			return true;

		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (!p.m_bOut || p.m_bHome || p.m_iTarget >= 0 || p.m_bGathering)
				continue;

			if (RZ_Util.DistXZ(p.m_vPos, hot) > SEND_REACH)
				continue;

			bool day = p.m_eKind == RZ_ETribePart.DAY;
			if (night && (!p.IsHunting() || day))
				continue;

			if (!night && !day)
				continue;

			Send(tribes, p, hot, HORDE_HOLD);
			sent++;
		}

		RZ_Log.Info(string.Format("Pack horde: %1 groups of the pack at %2 sent to the heat at %3 (%4).", sent, t.m_sPlace, RZ_SettlementScanner.GridReference(hot), PhaseWord(night)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Favourite-spot lookers at night inside a pack's range: one of its young groups out on its
	//! trails walks over to look. True when the spot is inside a range at night (whether or not a
	//! group was free): the town's lookers don't come into a pack's range at night.
	static bool Lookers(RZ_GameModeComponent gm, vector at)
	{
		RZ_Tribes tribes = gm.GetTribes();
		if (!tribes || tribes.GetPhase() != RZ_ETribePhase.NIGHT)
			return false;

		RZ_Tribe t = RangeAt(tribes, at);
		if (!t)
			return false;

		if (t.m_bVigil || t.IsWeakened() || t.m_Hunt)
			return true;

		RZ_TribePart best;
		float bestD = SEND_REACH;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind != RZ_ETribePart.YOUNG || !p.m_bOut || p.m_bHome || p.m_iTarget >= 0 || p.m_bGathering)
				continue;

			float d = RZ_Util.DistXZ(p.m_vPos, at);
			if (d < bestD)
			{
				best = p;
				bestD = d;
			}
		}

		if (best)
		{
			Send(tribes, best, at, LOOK_HOLD);
			RZ_Log.Info(string.Format("Pack lookers: young of the pack at %1 sent to look at a busy spot at %2.", t.m_sPlace, RZ_SettlementScanner.GridReference(at)));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A part walks to the spot as if it heard it (its noise), within 20 m, and waits there.
	protected static void Send(RZ_Tribes tribes, RZ_TribePart p, vector at, float hold)
	{
		p.m_vNoise = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(at, 0, 20));
		p.m_fNoiseUntil = tribes.GetClock() + RZ_Util.DistXZ(p.m_vPos, at) / RZ_InfectedBodies.DATA_SPEED + hold;
	}

	//------------------------------------------------------------------------------------------------
	protected static string PhaseWord(bool night)
	{
		if (night)
			return "night, its hunters";

		return "day, its day party";
	}
}
