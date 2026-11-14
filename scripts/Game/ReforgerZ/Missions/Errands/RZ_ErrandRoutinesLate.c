//! The routines phase 8 added (docs/ERRANDS.md, "The routines"): a dead crewman's lighter, the fence's
//! market and the tag receiver. Each repeats on the routines' cooldowns like the others; one class each.
//! Server only.

//------------------------------------------------------------------------------------------------
//! His Lighter (the dead, the Red Hand's way): someone died within 1 km of a Red Hand camp in the
//! last season day. His kit is on the market before the horns stop, and the lighter's the last of it.
//! Sell it to the fence, take it to his mother at a Haven house (a local), or sell it to Bastion as
//! proof of one less (a lie the crews' rumour round finds).
class RZ_Routine_HisLighter : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "his_lighter";
		m_sTitle = "His Lighter";
		m_sFamily = "dead";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		m_sRoles = "crewman,cook";
		Offer("One of our crew didn't come back {subject_place}. We sent him off loud, and his kit was on the market before the horns stopped. That's how it is. This is the last of it: his lighter. Somebody take it off my hands.",
			"This was his lighter, a lad from our crew who died {subject_place}. Everything else went to the fence before the horns stopped. I can't sell this one. You can. Or you know someone who'd want it.");
		Over("Never mind the lighter. Somebody bought it. It's in the book.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Sell it to the fence";
		w.m_iFavor = 6;
		w.Done("Bought fair. It's in the book. That's all anyone will say about him now.");

		w = Way("give");
		w.m_sLabel = "Take it to his mother";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Take the dead lad's lighter to his mother at {house} (5 s at the door), then tell {giver}.";
		w.m_iFame = 1;
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.Done("To his mother. You didn't sell it, and you didn't rat. Fair enough. Nobody in the crew will say anything else about it.");

		w = Way("lie");
		w.m_sLabel = "Sell it to Bastion as proof of one less";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.MILITARY;
		w.m_iPayFaction = RZ_EFaction.MILITARY;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Take the lighter to Bastion's post {subject_place} as proof of one less crewman, then tell {giver} it went to the fence.";
		w.m_iFavor = 5;
		w.Done("Gone? To the fence. Right. I won't ask which fence.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Signal s = RZ_ErrandCard.FreshSignal("death", e.m_vGiver, 1000, RZ_SeasonClock.DayMinutes() / 60, e.m_iAsker);
		if (!s)
			return false;

		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, Vector(s.x, 0, s.z)));
		e.m_sSubject = "lighter:" + RZ_SettlementScanner.GridReference(Vector(s.x, 0, s.z));
		int house = RZ_ErrandCard.HouseNear(gm, e.m_vGiver, 1500, -1, 0);
		if (house >= 0)
		{
			RZ_Safehouse h = gm.GetSafehouses().Get(house);
			e.m_vSubject2 = h.m_vPos;
			e.m_iValue = house;
			e.SetText("house", h.m_sName);
		}

		RZ_Holding post = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.MILITARY, e.m_vGiver, 1500);
		if (post)
		{
			e.m_vSubject = post.m_vPos;
			e.SetText("subject_place", post.m_sName);
		}

		return true;
	}

	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1 && dealer.GetLocals() && e.m_iValue >= 0)
		{
			dealer.GetLocals().Make(playerId, "whose son ran with the crews", "You brought me his lighter. Nobody else brought me anything.", e.m_iValue, "stove", false, "helped");
			said.Insert(e.Fill("His mother at {house} took it in both hands, I hear. What he owed is in the book against her now, and a lighter won't pay it. She knows."));
			return;
		}

		if (w.m_iIndex == 2)
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.lighter", e.m_sSubject, RZ_EFaction.BANDITS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 4,
				"A crew lad saw a dead man's lighter on a Bastion sergeant's desk. The crews remember who had it last.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Market (the fence): the Zone just made something. A wedding party broke up within 3 km in the
//! last season day, and a ring came off a finger. The fence wants it; the groom's mother at a Haven
//! house wants it too. Give it to her, sell it to her for two of the house's tins, or sell it to him.
//! Inside it: a date, and "for ever". Nobody laughs except the fence.
class RZ_Routine_Market : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "market";
		m_sTitle = "The Market";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		m_sRoles = "fence,toll man";
		Offer("There's a ring loose {subject_place}. A wedding party got broken up there, the infected came through the middle of it, and the ring's somewhere in the grass. Find it before the rain does and I'll give you a fair price. Fairish.",
			"A wedding party broke up {subject_place}, badly. Somewhere in the grass there's a ring with a date in it. I want it. So does the groom's mother, I hear. I want it more.");
		Over("The ring's gone. Somebody's mother got there first. Mothers do.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Find it, and sell it to him";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 40;
		w.m_sStep = "Search the grass where the wedding party was {subject_place} (20 s there), then bring the ring to {giver}.";
		w.Work(RZ_EWork.TAKE, "Search the grass");
		w.m_iFavor = 8;
		w.Done("For ever, it says inside. For ever. Ha. That's worth a tin on its own.");

		w = Way("give");
		w.m_sLabel = "Give it to the groom's mother";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.First(RZ_EErrandPlace.SUBJECT, "Search the grass where the wedding party was {subject_place} for the ring (20 s there).", 20);
		w.m_sStep = "Give the ring to the groom's mother at {house} (5 s at the door), then tell {giver}.";
		w.m_iFame = 2;
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.Done("To the mother? Soft. You're soft. I'll remember that, and so will the price of everything.");

		w = Way("do");
		w.m_sLabel = "Sell it to her for two tins";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.First(RZ_EErrandPlace.SUBJECT, "Search the grass where the wedding party was {subject_place} for the ring (20 s there).", 20);
		w.m_sStep = "Sell the ring to the groom's mother at {house} for two tins (5 s at the door), then tell {giver}.";
		w.m_iFavor = 4;
		w.Done("Two tins from a mother. That's business. Ugly business, but business.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Signal s = RZ_ErrandCard.FreshSignal("wedding", e.m_vGiver, 3000, RZ_SeasonClock.DayMinutes() / 60, -1);
		if (!s)
			return false;

		e.m_vSubject = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
		e.m_sSubject = "ring:" + RZ_SettlementScanner.GridReference(e.m_vSubject);
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		int house = RZ_ErrandCard.HouseNear(gm, e.m_vSubject, 3000, -1, 0);
		if (house >= 0)
		{
			RZ_Safehouse h = gm.GetSafehouses().Get(house);
			e.m_vSubject2 = h.m_vPos;
			e.m_iValue = house;
			e.SetText("house", h.m_sName);
		}

		return true;
	}

	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 2 && e.m_iValue >= 0 && gm.GetSafehouses())
		{
			RZ_Safehouse h = gm.GetSafehouses().Get(e.m_iValue);
			if (h)
				h.m_iFood = Math.Max(0, h.m_iFood - 2);
		}

		if (w.m_iIndex == 1)
			said.Insert(e.Fill("The groom's mother at {house} wears it on her own finger now, I hear, because it's the only safe place. A date inside, and 'for ever'."));
	}
}

//------------------------------------------------------------------------------------------------
//! The Receiver (the tag, re-angled): a tagged pack leader's signal is unread because the station's
//! receiver is down, a cable or the mast 200-400 m out. A battery fixes it, or a Lab Tech re-seats
//! it. Never near the pack or its tag. Down on some days, by the season's seed (the same all day).
class RZ_Routine_Receiver : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "receiver";
		m_sTitle = "The Receiver";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		m_sRoles = "technician";
		Offer("We tagged the one that leads the infected at {lair}, so we know where the group moves at night. The tag is still talking. We aren't listening: the receiver's down, {subject_place}. A small request: get it working.",
			"We put a radio tag in the leader at {lair} at great expense, and now the receiver's dead, {subject_place}. A cable, probably. Or a crow. Without it, we don't know which way they're walking.");
		Over("The receiver's back. Someone kicked it. Science.");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Fit a battery at the mast";
		w.m_eItem = RZ_EErrandItem.BATTERY;
		w.m_iItems = 1;
		w.m_bGiveThere = true;
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 10;
		w.m_fRadius = 25;
		w.m_sStep = "Fit a battery to the receiver's mast {place} (10 s there), then tell {giver}.";
		w.Work(RZ_EWork.REPAIR, "Fit the battery");
		w.m_iFavor = 5;
		w.Done("There he is on the screen. Walking in circles, as usual. Thank you.");

		w = Way("do");
		w.m_sLabel = "Re-seat the cable";
		w.m_sBackground = "labtech";
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 15;
		w.m_fRadius = 25;
		w.m_sStep = "Re-seat the receiver's cable at the mast {place} (15 s there), then tell {giver}.";
		w.Work(RZ_EWork.REPAIR, "Re-seat the cable");
		w.m_iFavor = 5;
		w.Done("Just the cable? I'll be writing that down as a fault in the equipment, not in us.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Tribes tribes = dealer.GetGM().GetTribes();
		if (!tribes)
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (t.m_iTagDay < 0 || RZ_Util.DistXZ(t.m_vLair, e.m_vGiver) > 6000)
				continue;

			// Down today or not, the same all day (the season's seed)
			RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + e.m_iHolding * 97 + RZ_SeasonClock.Day() * 13);
			if (!rng.Chance(0.5))
				return false;

			e.SetText("lair", t.m_sPlace);
			e.SetText("subject_place", "out on the mast");
			e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".receiver";
			return true;
		}

		return false;
	}
}
