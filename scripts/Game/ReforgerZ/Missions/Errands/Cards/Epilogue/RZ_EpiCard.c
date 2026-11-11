//! The epilogue deck's base (docs/ERRANDS.md, "The epilogue deck"; docs/EPILOGUE.md): the last
//! errands, dealt by errands' dealer only in the epilogue week (m_bAfterCure), once per identity per
//! subject (a beat, a name, a place), outside the season's cards a day, paid through the small-pay
//! ledger like any errand. Helpers read the epilogue's record: the route, its terms, the queue and
//! the boards. Server only.
class RZ_EpiCard : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	void RZ_EpiCard()
	{
		m_bAfterCure = true;
		m_sFamily = "epilogue";
	}

	//------------------------------------------------------------------------------------------------
	//! The week's story, once it's built (never on the Unsigned last day).
	static RZ_Epilogue Week()
	{
		RZ_Epilogue epi = RZ_Epilogue.Get();
		if (!epi || !epi.IsBuilt() || epi.Data().lastDay || epi.Phase() <= 0)
			return null;

		return epi;
	}

	//------------------------------------------------------------------------------------------------
	//! The route that controls the cure (RZ_CureFinale.HAVEN ... INDEP), -1 none.
	static int Route()
	{
		RZ_Epilogue epi = Week();
		if (!epi)
			return -1;

		return epi.Route();
	}

	//------------------------------------------------------------------------------------------------
	//! The queue at the north road, vector.Zero if there's none.
	static vector Queue()
	{
		RZ_Epilogue epi = Week();
		if (!epi)
			return vector.Zero;

		return epi.GetSpots().QueuePos();
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's way of things after the cure: its route, or its book balanced.
	static bool RedHandWay(RZ_GameModeComponent gm)
	{
		int last = RZ_MajorHelp.World(gm, "book.last");
		return Route() == RZ_CureFinale.REDHAND || last == 1 || last == 2;
	}

	//------------------------------------------------------------------------------------------------
	//! A point `metres` up the queue's road toward `toward` (the queue itself if they're close).
	static vector UpTheRoad(vector toward, float metres)
	{
		vector q = Queue();
		if (q == vector.Zero)
			return q;

		vector dir = toward - q;
		dir[1] = 0;
		float len = dir.Length();
		if (len < 1)
			return q;

		dir = dir * (1 / len);
		return RZ_Util.GroundPos(q + dir * Math.Min(metres, len));
	}

	//------------------------------------------------------------------------------------------------
	//! A named dead person still lying out (Bring Them Home's list), of `faction` (-2: any), within
	//! `radius` of pos; null if none.
	static RZ_EpiName LyingNear(int faction, vector pos, float radius)
	{
		RZ_Epilogue epi = Week();
		if (!epi)
			return null;

		foreach (RZ_EpiName n : epi.Data().names)
		{
			if (n.state != 0 || (faction > -2 && n.faction != faction))
				continue;

			if (RZ_Util.DistXZ(Vector(n.x, 0, n.z), pos) <= radius)
				return n;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A name found again by its text.
	static RZ_EpiName NameCalled(string name)
	{
		RZ_Epilogue epi = Week();
		if (!epi)
			return null;

		foreach (RZ_EpiName n : epi.Data().names)
		{
			if (n.name == name)
				return n;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A name goes home by an errand (the board, the lab): the wall lists it with who brought it.
	static void NameHome(RZ_EpiName n, string home, int playerId)
	{
		if (!n || n.state == 2)
			return;

		n.state = 2;
		n.home = home;
		n.by = RZ_EpiNames.Now(playerId);
		RZ_Log.Info(string.Format("Epilogue: %1 went home to %2 by an errand.", n.name, home));
	}
}
