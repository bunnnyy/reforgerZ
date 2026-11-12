//! The reasons missions read from facts other systems already hold (docs/DESIGN.md, "Story blocks";
//! the store is RZ_WorldMemory): checked every 30 s on the errands' slow tick, errands on or not, and
//! posted while the fact holds, cleared when it stops. The rest of the reasons are posted by the
//! systems that own their facts (RZ_WorldWriters). Also the reasons a finished job or condition
//! leaves behind (Ended). Server only.
//!
//! | Reason          | Holds while                                                              |
//! | house.water     | a safehouse's water fault (RZ_Faults)                                    |
//! | house.dark      | a safehouse's fuel fault                                                 |
//! | post.pressure   | a Bastion holding has a town the infected hold within 1.5 km             |
//! | leader.rogue    | a leader who isn't a stronghold's has gone Rogue                         |
//! | lab.short       | a Halcyon station's sensor has gone dark                                 |
//! | lab.power       | a Halcyon station's freezers are low on fuel                             |
//! | racket.ran      | Protection Racket collected at a house (half a season day)               |
//! | crew.skimmed    | Skim the Toll was done (half a season day)                               |
//! | house.feud      | a word carried between two houses with a threat in it (A Word to {house})|
//! | horde.passed    | Every Ridge Calls, the Siren Test, Red Dusk ended near a town            |
//! | tribe.hungry    | a pack's hunger is in its second band or higher                          |
//! | lab.power       | also left by a Power Surge                                               |
class RZ_ReasonPosts
{
	protected static const float PRESSURE_RANGE = 1500;

	protected static ref map<string, bool> s_mPosted;		// "key|subject" -> posted by this watcher

	//------------------------------------------------------------------------------------------------
	protected static map<string, bool> Posted()
	{
		if (!s_mPosted)
			s_mPosted = new map<string, bool>();

		return s_mPosted;
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 s (RZ_Errands.Tick): post the reasons whose facts hold, clear the ones that stopped.
	static void Watch(RZ_GameModeComponent gm, RZ_Faults faults)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		if (!facts)
			return;

		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && faults)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (!h)
					continue;

				string house = "house:" + i.ToString();
				Hold(facts, "house.water", house, faults.Has(RZ_Faults.HousePlace(i), RZ_EFault.WATER), string.Format("%1's barrels are low.", h.m_sName), h.m_vPos, RZ_EFaction.SURVIVORS);
				Hold(facts, "house.dark", house, faults.Has(RZ_Faults.HousePlace(i), RZ_EFault.FUEL), string.Format("%1's generator is on its last cupful.", h.m_sName), h.m_vPos, RZ_EFaction.SURVIVORS);
			}
		}

		RZ_Territory territory = gm.GetTerritory();
		if (territory)
		{
			foreach (RZ_Holding hold : territory.GetHoldings())
			{
				string subject = "holding:" + hold.m_iId.ToString();
				if (hold.m_eFaction == RZ_EFaction.MILITARY)
					Hold(facts, "post.pressure", subject, BadTownNear(gm, hold.m_vPos), string.Format("Bastion's people at %1 have infected on their doorstep.", hold.m_sName), hold.m_vPos, RZ_EFaction.MILITARY);

				if (hold.m_eFaction == RZ_EFaction.INSTITUTE && faults)
				{
					string place = RZ_Faults.HoldingPlace(hold.m_iId);
					Hold(facts, "lab.short", subject, faults.Has(place, RZ_EFault.SENSOR), string.Format("One of Halcyon's sensors near %1 has gone quiet.", hold.m_sName), hold.m_vPos, RZ_EFaction.INSTITUTE);
					Hold(facts, "lab.power", subject, faults.Has(place, RZ_EFault.FUEL), string.Format("Halcyon's freezers at %1 are running on the last of the fuel.", hold.m_sName), hold.m_vPos, RZ_EFaction.INSTITUTE);
				}
			}
		}

		// The packs: a hungry one (in the Haven's words; player text never names what they are)
		RZ_Tribes tribes = gm.GetTribes();
		if (tribes && tribes.GetPeople())
		{
			foreach (RZ_Tribe t : tribes.GetTribeList())
			{
				string lot = RZ_Tribes.Capitalize(tribes.GetPeople().NameAt(t, RZ_EFaction.SURVIVORS));
				Hold(facts, "tribe.hungry", "tribe:" + t.m_iId.ToString(), t.Band() >= 2, string.Format("%1 are hungry, and closer to the fields every night.", lot), t.m_vLair, -1);
			}
		}

		RZ_Leaders leaders = gm.GetLeaders();
		if (leaders)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				array<RZ_Leader> list = {};
				leaders.GetFactionLeaders(f, list);
				foreach (RZ_Leader l : list)
				{
					bool rogue = l.m_eLoyalty == RZ_ELoyalty.ROGUE && l.IsPresent() && !l.IsStronghold();
					Hold(facts, "leader.rogue", "leader:" + l.m_iId.ToString(), rogue, string.Format("%1 has stopped answering to anyone.", l.m_sName), l.m_vPos, f);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A town the infected hold (infested or overrun) within 1.5 km of pos.
	protected static bool BadTownNear(RZ_GameModeComponent gm, vector pos)
	{
		foreach (RZ_Settlement s : gm.GetSettlements())
		{
			RZ_ESettlementState state = s.GetState();
			if ((state == RZ_ESettlementState.INFESTED || state == RZ_ESettlementState.OVERRUN) && RZ_Util.DistXZ(s.m_vCenter, pos) <= PRESSURE_RANGE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Post the reason while `holds`, clear it once it stops (only the ones this watcher posted).
	protected static void Hold(RZ_WorldFacts facts, string key, string subject, bool holds, string text, vector pos, int faction)
	{
		string id = key + "|" + subject;
		bool was = Posted().Contains(id);
		if (holds && !was)
		{
			facts.Memory().PostReason(key, subject, text, pos, faction);
			Posted().Set(id, true);
		}
		else if (!holds && was)
		{
			facts.Memory().ClearReason(key, subject);
			Posted().Remove(id);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A job or condition finished (RZ_MissionManager): the reasons it leaves behind.
	static void Ended(RZ_GameModeComponent gm, RZ_Mission m, bool success)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		if (!facts || !m || m.m_bWithdrawn)
			return;

		string town = RZ_WorldFacts.Town(m.m_iSettlement);
		string place = m.GetLocationName(gm);
		if (success && m.Type() == RZ_Mission_ProtectionRacket)
			facts.Memory().PostReason("racket.ran", town, string.Format("The crew collected at %1.", place), m.m_vPos, RZ_EFaction.BANDITS, 0.5);

		if (success && m.Type() == RZ_Job_SkimTheToll)
			facts.Memory().PostReason("crew.skimmed", town, string.Format("Someone's been skimming the toll near %1.", place), m.m_vPos, RZ_EFaction.BANDITS, 0.5);

		if (m.Type() == RZ_Event_PowerSurge && gm.HasLab())
			facts.Memory().PostReason("lab.power", "lab", "The surge took the lab's lights for an hour. The freezers felt it.", gm.GetLabPosition(), RZ_EFaction.INSTITUTE, 0.5);

		// The conditions that drive the infected past towns: a horde passed
		if (m.Type() == RZ_Event_EveryRidgeCalls || m.Type() == RZ_Event_SirenTest || m.Type() == RZ_Event_RedDusk)
		{
			int near = gm.FindNearestSettlement(m.m_vPos);
			RZ_Settlement s = gm.GetSettlement(near);
			if (s && RZ_Util.DistXZ(s.m_vCenter, m.m_vPos) <= 3000)
				facts.Memory().PostReason("horde.passed", RZ_WorldFacts.Town(near), string.Format("Something big came through %1 in the night.", s.m_sName), s.m_vCenter, -1, 0.5);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A word carried between two houses with a threat in it: the houses fall out (A Word to {house}).
	static void Feud(RZ_GameModeComponent gm, int fromHouse, int toHouse)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_WorldFacts facts = gm.GetFacts();
		if (!houses || !facts)
			return;

		RZ_Safehouse a = houses.Get(fromHouse);
		RZ_Safehouse b = houses.Get(toHouse);
		if (!a || !b)
			return;

		facts.Memory().PostReason("house.feud", "house:" + toHouse.ToString(), string.Format("%1 and %2 aren't speaking.", a.m_sName, b.m_sName), b.m_vPos, RZ_EFaction.SURVIVORS);
	}
}
