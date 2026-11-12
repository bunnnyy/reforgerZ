//! A Debt Gone Quiet (docs/ERRANDS.md, the crews; Collections re-angled): a crew's book-keeper, for
//! someone Feared (band 2+) with anyone. Not a collection: a man in the book has stopped paying and
//! stopped answering. Find out why before the crew does. He's alive, and paying for medicine. Lean
//! on him, pay it yourself, or say he's dead (a lie, found when he's seen at the next toll).
class RZ_Errand_DebtGoneQuiet : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "debt_gone_quiet";
		m_sTitle = "A Debt Gone Quiet";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("{name} at {subject_place} has a line in the book, and has paid every week since the spring. Now nothing, and no answer at the door. Find out why before the lads go up there and find out their way.",
			"A line in the book went quiet: {name}, at {subject_place}. Paid on time for weeks, then stopped. Go and ask nicely. You're good at nicely, people say. Or the other thing.");
		Over("Never mind {name}. The lads went up there themselves, and the line's moving again.");

		RZ_ErrandWay w = Way("press");
		w.m_sLabel = "Find him, and lean on him";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find {name} at {subject_place} and get the money owed (15 s there), then come back to {giver}.";
		w.m_iFavor = 6;
		w.Done("Paid. Good. I don't want to know what you said to get it. I want it in the book.");

		w = Way("pay");
		w.m_sLabel = "Pay what he owes yourself";
		w.m_iFavorCost = 10;
		w.m_iFame = 1;
		w.Done("Your money, against that line. The book doesn't care whose money it is. I do, a bit. Nobody pays somebody else's line for nothing.");

		w = Way("lie");
		w.m_sLabel = "Say he's dead";
		w.m_iFavor = 3;
		w.Done("Dead? Shame. I'll draw a line through the name. Somebody else will owe it now.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_PlayerProfile p = dealer.GetGM().GetProfile(playerId);
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (RZ_Checks.FearBand(p.Feared(f)) >= 2)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		int house = HouseNear(gm, e.m_vGiver, 1500, -1, 0);
		if (house < 0)
			return false;

		RZ_Safehouse h = gm.GetSafehouses().Get(house);
		Place(e, h.m_vPos, h.m_sName);
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + house * 211 + RZ_SeasonClock.Day());
		string name = RZ_FactionVoice.PersonName(rng, RZ_EFaction.SURVIVORS);
		e.SetText("name", name);
		e.m_iValue = house;
		e.m_sSubject = "debtor:" + name;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 0)
		{
			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 3, "leaned on a sick man's family", 0, "leaned on a man paying for medicine");
			said.Insert(e.Fill("Paid. Turns out the money had been going on medicine for a sick wife. You know that now. So does all of {subject_place}, after you were at the door."));
			return;
		}

		if (w.m_iIndex == 2)
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.debt_quiet", e.m_sSubject, RZ_EFaction.BANDITS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 3,
				e.Fill("{name} walked up to the toll last night, very much alive, with his wife's prescription in his hand. The book-keeper remembers who said he was dead."));
	}
}
