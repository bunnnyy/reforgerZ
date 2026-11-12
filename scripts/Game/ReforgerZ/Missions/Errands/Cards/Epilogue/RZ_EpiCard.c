//! The epilogue deck's base (docs/ERRANDS.md, "The epilogue deck"; docs/EPILOGUE.md): the last
//! errands, dealt by errands' dealer only in the epilogue week (m_bAfterCure), once per identity per
//! subject (a beat, a name, a place), outside the season's cards a day, paid through the small-pay
//! ledger like any errand. A card is dealt from a beat: one of the week's beats within 3 km of the
//! giver, and at most two of the deck offered or held by one player within 3 km at once (Why, then
//! Room). Helpers read the epilogue's record: the route, its terms, the queue and the boards.
//! Server only.
class RZ_EpiCard : RZ_ErrandCard
{
	static const float DECK_RADIUS = 3000;
	static const int DECK_AT_ONCE = 2;

	// The deck's live errands (offered or held), to keep two a player within 3 km
	protected static ref array<ref RZ_Errand> s_aDealt = {};

	//------------------------------------------------------------------------------------------------
	void RZ_EpiCard()
	{
		m_bAfterCure = true;
		m_sFamily = "epilogue";
	}

	//------------------------------------------------------------------------------------------------
	//! The card's own reason (each card's), then room for it.
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!WhyCard(dealer, e))
			return false;

		return DeckRoom(e);
	}

	//------------------------------------------------------------------------------------------------
	bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Room for one more of the deck for this player here: a beat within 3 km of the giver, and fewer
	//! than two of the deck offered or held by them within 3 km. Records the errand when there is.
	static bool DeckRoom(RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi)
			return false;

		if (epi.GetBeats().NearestEcho(e.m_vGiver, DECK_RADIUS).IsEmpty())
			return false;

		int count = 0;
		for (int i = s_aDealt.Count() - 1; i >= 0; i--)
		{
			RZ_Errand d = s_aDealt[i];
			if (!d || d.m_eState == RZ_EErrandState.DONE)
			{
				s_aDealt.Remove(i);
				continue;
			}

			if (d != e && d.m_iAsker == e.m_iAsker && RZ_Util.DistXZ(d.m_vGiver, e.m_vGiver) <= DECK_RADIUS)
				count++;
		}

		if (count >= DECK_AT_ONCE)
			return false;

		if (!s_aDealt.Contains(e))
			s_aDealt.Insert(e);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! An unarmed step (Copies for the Far House, Last Letters, Processing): a gun in your hands at
	//! its place spoils it (the card's Still returns false and the giver says the over line).
	static bool ArmedAtStep(RZ_Errand e)
	{
		if (e.m_eState != RZ_EErrandState.HELD || e.m_bStepDone || e.m_vPlace == vector.Zero)
			return false;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(e.m_iAsker);
		if (!ent || RZ_Util.DistXZ(ent.GetOrigin(), e.m_vPlace) > 60)
			return false;

		return RZ_Util.IsArmed(ent);
	}

	//------------------------------------------------------------------------------------------------
	//! Infected within `radius` of pos (the spawner's count, stopping at `stopAt`).
	static int InfectedNear(RZ_GameModeComponent gm, vector pos, float radius, int stopAt)
	{
		if (!gm.GetSpawner())
			return 0;

		return gm.GetSpawner().CountZombiesNear(pos, radius, stopAt);
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
