//! Offer reach (docs/MISSIONS.md, "Who gets offered what"): most players run, so a giver offers
//! work within walking reach of where they stand. Work whose place is chosen is chosen near the
//! giver asked (MakeFor's near hint, NarrowTowns); work whose place is fixed is offered by the
//! nearest giver of its faction that has it within reach; with none in reach, it's offered as
//! before and its pitch says it's a long walk (FarLine).
class RZ_OfferReach
{
	static const int HERE = 0;			// within reach of this giver (or reach is off)
	static const int ELSEWHERE = 1;		// another giver of the faction has it within reach: theirs
	static const int FAR = 2;			// no giver of the faction has it within reach: offered here, far

	// The giver a contract is being made for (MakeFor): the town pickers prefer towns within reach
	protected static vector s_vNear;
	protected static float s_fNearReach;

	//------------------------------------------------------------------------------------------------
	//! The jobs' offer reach (m), kept to 0 or 1000-10000; 0 = off.
	static float JobReach(RZ_GameModeComponent gm)
	{
		if (!gm || !gm.GetContractSettings())
			return 0;

		float reach = gm.GetContractSettings().m_fOfferReach;
		if (reach <= 0)
			return 0;

		return Math.Clamp(reach, 1000, 10000);
	}

	//------------------------------------------------------------------------------------------------
	//! Where `faction` hands out contracts: its quartermaster, its holdings (an outpost only with its
	//! own quartermaster on), and the Haven's safehouse radios.
	static void Givers(RZ_GameModeComponent gm, int faction, notnull array<vector> result)
	{
		RZ_FactionPerks perks = gm.GetPerks();
		vector qm;
		if (perks && perks.GetQuartermasterPos(faction, qm))
			result.Insert(qm);

		RZ_Territory territory = gm.GetTerritory();
		if (territory)
		{
			bool outposts = territory.GetSettings() && territory.GetSettings().m_bOutpostQuartermaster;
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (h.m_eFaction == faction && (h.m_bStronghold || outposts))
					result.Insert(h.m_vPos);
			}
		}

		RZ_Safehouses houses = gm.GetSafehouses();
		if (faction == RZ_EFaction.SURVIVORS && houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				result.Insert(houses.Get(i).m_vPos);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Whether a giver of `faction` standing at `giver` offers work at `place`: HERE, ELSEWHERE or FAR.
	static int Decide(RZ_GameModeComponent gm, int faction, vector giver, vector place)
	{
		float reach = JobReach(gm);
		if (reach <= 0 || place == vector.Zero || giver == vector.Zero || RZ_Util.DistXZ(giver, place) <= reach)
			return HERE;

		array<vector> givers = {};
		Givers(gm, faction, givers);
		foreach (vector g : givers)
		{
			if (RZ_Util.DistXZ(g, place) <= reach)
				return ELSEWHERE;
		}

		return FAR;
	}

	//------------------------------------------------------------------------------------------------
	//! A contract's decision for a giver of `faction` at `giver`: a global one, or one made on asking
	//! at this giver, is always HERE.
	static int DecideFor(RZ_GameModeComponent gm, RZ_Mission m, int faction, vector giver)
	{
		if (m.m_bGlobal || (m.m_vAskedAt != vector.Zero && RZ_Util.DistXZ(m.m_vAskedAt, giver) <= 300))
			return HERE;

		return Decide(gm, faction, giver, m.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! "It's a long walk: about 9 km north-east." Empty when it's within reach.
	static string FarLine(RZ_GameModeComponent gm, vector from, vector place)
	{
		float reach = JobReach(gm);
		if (reach <= 0 || from == vector.Zero || place == vector.Zero)
			return "";

		float dist = RZ_Util.DistXZ(from, place);
		if (dist <= reach)
			return "";

		int km = RZ_Util.RoundToInt(dist / 1000);
		string dir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(from, place));
		return string.Format("It's a long walk: about %1 km %2.", km, dir);
	}

	//------------------------------------------------------------------------------------------------
	//! While a giver makes a contract (MakeFor): the town pickers keep to towns within reach of
	//! `giver` when any qualify. ClearNear after.
	static void SetNear(RZ_GameModeComponent gm, vector giver)
	{
		s_vNear = giver;
		s_fNearReach = JobReach(gm);
	}

	//------------------------------------------------------------------------------------------------
	static void ClearNear()
	{
		s_vNear = vector.Zero;
		s_fNearReach = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! With a near hint set and any of `towns` within its reach, drops the rest. Otherwise unchanged.
	static void NarrowTowns(RZ_GameModeComponent gm, notnull array<int> towns)
	{
		if (s_fNearReach <= 0 || s_vNear == vector.Zero || towns.IsEmpty())
			return;

		array<int> near = {};
		foreach (int i : towns)
		{
			if (RZ_Util.DistXZ(gm.GetSettlements()[i].m_vCenter, s_vNear) <= s_fNearReach)
				near.Insert(i);
		}

		if (near.IsEmpty())
			return;

		towns.Clear();
		towns.InsertAll(near);
	}
}
