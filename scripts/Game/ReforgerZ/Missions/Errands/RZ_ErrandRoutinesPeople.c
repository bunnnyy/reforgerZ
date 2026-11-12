//! The routines phase 4 added (docs/ERRANDS.md, "The routines"): people at the holdings asking
//! about people. Each repeats on the routines' cooldowns like the others; one class each.
//! Server only.

//------------------------------------------------------------------------------------------------
//! Who's at the Gate: someone is waiting at a safehouse's gate (a found hook's bitten stranger was
//! sent here). Let them in, look them over first, or walk them to Bastion's post. The house checks
//! everyone at the door, so nobody is blamed for a bite they didn't know about.
class RZ_Routine_WhosAtTheGate : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "whos_at_gate";
		m_sTitle = "Who's at the Gate";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "lookout";
		Offer("There's someone at our gate, holding one arm close. Says another house sent them. The rule is you declare a bite at the door, and not everyone does. I've let nobody in since Tuesday without a second pair of eyes. Will you look?",
			"Stranger at the gate, holding an arm and saying it's nothing. Could be a fence. Could be a bite. Could be nothing. Will you look them over before I open up?");
		Over("Never mind the gate. They've gone. Or someone else let them in. I'm not asking which.");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Let them in";
		w.Done("In you come, then. Shoes off. Everyone takes their shoes off. It's a rule now.");

		w = Way("talk");
		w.m_sLabel = "Look them over first";
		w.m_sBackground = "nurse,doctor,police";
		w.Done("Right. Now we know. Thank you for doing it properly.");

		w = Way("do");
		w.m_sLabel = "Walk them to Bastion's post";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Walk the stranger to the Bastion post {subject_place} and leave them with the sentry, then come back to {giver}.";
		w.Done("The army can have them. Not a nice thing to say. It's true, though.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		int at = dealer.GetStrangers().AtGate(e.m_iHouse);
		if (at <= 0)
			return false;

		e.m_iValue = at;
		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".gate";
		RZ_Checkpoints posts = dealer.GetGM().GetCheckpoints();
		if (posts)
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(RZ_EFaction.MILITARY, e.m_vGiver, 1500, near);
			if (!near.IsEmpty())
			{
				e.m_vSubject = near[0].m_vPos;
				e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
			}
		}

		return true;
	}

	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		return dealer.GetStrangers().AtGate(e.m_iHouse) > 0;
	}

	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_Safehouses houses = dealer.GetGM().GetSafehouses();
		bool bitten = e.m_iValue == 2;
		dealer.GetStrangers().ClearGate(e.m_iHouse);
		if (w.m_iIndex == 2)
			return;

		if (w.m_iIndex == 1)
		{
			if (bitten)
				said.Insert("You found the bite before anyone else had to. We'll send them on to the lab's gate for the trial dose, with bread for the road.");
			else
				said.Insert("No bite, and the story holds together. They'll be one of the house by supper.");

			if (!bitten && houses)
				houses.AddPopulationAt(e.m_iHouse, 1);

			return;
		}

		if (bitten)
		{
			said.Insert("The house checked them at the door anyway, and found a bite. They'll be walked out at dusk. Nobody blames you. You couldn't have known.");
			return;
		}

		if (houses)
			houses.AddPopulationAt(e.m_iHouse, 1);
	}
}

//------------------------------------------------------------------------------------------------
//! Sit With Him: a bitten resident at the house (a bite declared there, or someone walked out).
//! Sit with him (a Nurse or Doctor eases it). He tells you where his tins are buried, or something
//! true. No pay. A world record, with an epilogue line.
class RZ_Routine_SitWithHim : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "sit_with_him";
		m_sTitle = "Sit With Him";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("There's a man in our back room who's bitten. He declared it himself, at supper, the way the house asks. There's no lab near enough and no dose here. Would you sit with him? Nobody else can face it today.",
			"He's bitten, and he's frightened, and he's very polite about it. Nobody should be alone in the back room. Sit with him a while, that's all I ask.");
		Over("You needn't go in now. It's over. He was polite to the end.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Sit with him";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 120;
		w.m_fRadius = 15;
		w.m_sStep = "Sit with the bitten man in the back room at {here} (2 minutes).";
		w.Done("Thank you. He asked if you'd come back. I said probably.");

		w = Way("do");
		w.m_sLabel = "Sit with him, and see to the bite";
		w.m_sBackground = "nurse,doctor";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 15;
		w.m_sStep = "Sit with the bitten man at {here} and dress the bite properly (a minute).";
		w.Done("He's easier. You can tell when someone knows what they're doing. So can he.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Reason r = RZ_WorldFacts.Get().Memory().FindReason("house.bitten", RZ_WorldFacts.Safehouse(e.m_iHouse));
		if (!r)
			return false;

		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".bitten";
		return true;
	}

	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (Math.RandomFloat01() < 0.5 && gm.GetSafehouses())
		{
			gm.GetSafehouses().AddFood(e.m_iHouse, 1);
			said.Insert("He told me to tell you where he buried his tins: under the third paving stone by the shed. We dug, and they're there. The house eats them tonight.");
		}
		else
		{
			said.Insert("He asked me to pass on one true thing, for you, in his words: " + RZ_Routine_SitWithHim.TrueThing(gm, e.m_vGiver));
		}

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.sat_with", RZ_WorldFacts.Safehouse(e.m_iHouse), e.GetText("here"), RZ_WorldMemory.SOURCE_ERRAND, false, who,
			"Someone sat with a bitten man in the back room, so he wasn't alone. The house remembers that more than the bite.", "voice");
		RZ_WorldFacts.Get().Memory().ClearReason("house.bitten", RZ_WorldFacts.Safehouse(e.m_iHouse));
	}

	//------------------------------------------------------------------------------------------------
	//! A real thing from the world, never made up.
	static string TrueThing(RZ_GameModeComponent gm, vector at)
	{
		RZ_Tribes tribes = gm.GetTribes();
		if (tribes)
		{
			RZ_Tribe t = tribes.NearestTribe(at, 3000);
			if (t)
				return string.Format("The lot from %1 come out at dusk. Always the same way. I watched them from the roof for a week.", t.m_sPlace);
		}

		array<RZ_Reason> towns = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(at, 3000, towns, "town.infested");
		if (!towns.IsEmpty())
			return towns[0].text + " That's where I got it.";

		return "The well at the crossroads is good. The one by the church isn't. Tell them.";
	}
}

//------------------------------------------------------------------------------------------------
//! Fresh Chalk: by day, the edge of the infected's ground within 1.5 km, the Haven's reading of it 2+
//! season days old. Watch the edge 30 s unnoticed and chalk today's date on the Haven's mark; chalk
//! without looking (a lie, x0.6, the reading not refreshed); or a Hunter or Radio Op reads them by
//! their call from 300 m at dusk. Haven chalk only. The only thing it writes is the Haven's reading
//! day, and only by a real reading.
class RZ_Routine_FreshChalk : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "fresh_chalk";
		m_sTitle = "Fresh Chalk";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "lookout";
		Offer("The chalk mark on the gatepost for {lot} is days old: how many, which way, and the date. People going out for water decide by that mark. Watch the edge of their ground a minute, unseen, and write today's date.",
			"Somebody needs to look at {lot} and chalk the date on the mark. If the date's old, people guess. Guessing gets people eaten.");
		Over("Somebody chalked it already, fresh as anything. You can leave it.");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Watch the edge, unseen";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 30;
		w.m_fRadius = 40;
		w.m_bUnseen = true;
		w.m_sStep = "Watch the edge of the ground {lot} keep, {subject_place}, for 30 s without being seen. Then chalk the date on the mark and report to {giver}.";
		w.m_iFavor = 6;
		w.Done("Today's date, in your writing. That's a good thing to have on a gatepost.");

		w = Way("do");
		w.m_sLabel = "Read them by their call at dusk";
		w.m_sBackground = "hunter,radioham";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 20;
		w.m_fRadius = 60;
		w.m_sStep = "From 300 m out {subject_place}, listen for {lot} calling at dusk (20 s). Then chalk the date and report to {giver}.";
		w.m_iFavor = 6;
		w.Done("By ear. I've heard of people who can. I'd never met one.");

		w = Way("lie");
		w.m_sLabel = "Chalk the date without looking";
		w.m_iFavor = 3;
		w.Done("Today's date, without even going? Lovely. That'll do people some good.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Tribes tribes = gm.GetTribes();
		if (!tribes || RZ_GameTime.IsNight())
			return false;

		RZ_Tribe t = tribes.NearestTribe(e.m_vGiver, 4500);
		if (!t)
			return false;

		vector dir = e.m_vGiver - t.m_vLair;
		dir[1] = 0;
		float len = dir.Length();
		if (len < 1 || len - t.m_fRadius > 1500 || len < t.m_fRadius)
			return false;

		if (RZ_SeasonClock.Day() - dealer.Counter(ReadKey(t)) < 2 && dealer.Counter(ReadKey(t)) > 0)
			return false;

		vector edge = t.m_vLair + dir * (t.m_fRadius / len);
		e.m_vSubject = RZ_Mission.LandPointNear(edge, 0, 20);
		e.m_vSubject2 = RZ_Mission.LandPointNear(t.m_vLair + dir * ((t.m_fRadius + 300) / len), 0, 20);
		e.m_iValue = t.m_iId;
		e.m_sSubject = "tribe:" + t.m_iId.ToString() + ".chalk";
		e.SetText("lot", "the lot from " + t.m_sPlace);
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		return true;
	}

	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		// Reading by the call is a dusk thing
		if (w.m_iIndex == 1)
			return RZ_WorldFacts.Get().IsDusk();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static string ReadKey(RZ_Tribe t)
	{
		return "read." + t.m_iId.ToString();
	}

	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		if (w.m_iIndex == 2)
		{
			// The reading isn't refreshed; the lie checks at the next real reading, by anyone
			mem.Lies().Tell(gm, playerId, "errand.chalk", e.m_sSubject, RZ_EFaction.SURVIVORS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.TICK, 2,
				"Someone read the mark properly. The date you chalked was a guess, and the house knows whose.");
			return;
		}

		dealer.SetCounter("read." + e.m_iValue.ToString(), RZ_SeasonClock.Day());
		mem.Lies().Test(gm, "errand.chalk", e.m_sSubject);
	}
}

//------------------------------------------------------------------------------------------------
//! Fell Off the Inventory (Spare Magazine merged in): a private at a Bastion post, the shift change
//! within 20 real minutes and a Red Hand fence within 1.5 km. "It's not lost. I know exactly where it
//! is." Buy it back, or report him. Known with both, and the post's morale low, he asks you to sell
//! his spare magazine to a crew lookout instead.
class RZ_Routine_FellOff : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "fell_off";
		m_sTitle = "Fell Off the Inventory";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_sRoles = "sentry,mechanic";
		m_bTimed = true;
		Offer("A rifle fell off the inventory. It's not lost. I know exactly where it is: at the Red Hand fence's in {subject_place}, swapped for tins when we were hungry. The rack's counted at the shift change, and one short means the stockade.",
			"Shift change soon, and the rifle rack's one short. I sold it to the crews at {subject_place}. Well. Somebody sold it. I need it back before the count, or it's my name in the log.");
		Over("The count's done. It came out right. Don't ask me how. I really don't know.");

		RZ_ErrandWay w = Way("pay");
		w.m_sLabel = "Buy it back from the fence";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iFavorCost = 8;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Buy the rifle back from the Red Hand's fence at {subject_place} (paid already), then bring it to {giver} before the count.";
		w.Done("That's it. That's the one. I'd know that scratch anywhere. I made it.");

		w = Way("report");
		w.m_sLabel = "Tell the sergeant";
		w.m_iFame = 1;
		w.Done("You told the sergeant. Right. Thank you. I suppose. Don't expect me to look at you for a while.");

		w = Way("do");
		w.m_sLabel = "Sell his spare magazine for him";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Sell the soldier's spare magazine to the Red Hand's lookout at {subject_place}, quietly, and bring {giver} what it fetched.";
		w.m_iFavor = 6;
		w.Done("Tins. Good. Don't tell me which crew. I'd rather not know which crew.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_GameTime.IsCalibrated())
			return false;

		float soon = Math.Min(RZ_GameTime.RealMinutesUntil(5.5), RZ_GameTime.RealMinutesUntil(17.5));
		if (soon > 20)
			return false;

		RZ_Territory territory = dealer.GetGM().GetTerritory();
		if (!territory)
			return false;

		RZ_Holding fence = territory.FindHolding(RZ_EFaction.BANDITS, e.m_vGiver, 1500);
		if (!fence)
			return false;

		e.m_vSubject = fence.m_vPos;
		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".rack";
		e.SetText("subject_place", fence.m_sName);
		return true;
	}

	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex != 2)
			return true;

		// The Spare Magazine variant: Known with both, and the post's morale low
		RZ_FactionLife life = gm.GetFactionLife();
		bool low = life && life.IsActive() && life.GetMorale(RZ_EFaction.MILITARY) < 40;
		return low && RZ_Reputation.IsAtLeast(gm.GetTier(e.m_iAsker, RZ_EFaction.MILITARY), RZ_ETier.KNOWN);
	}

	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 0)
		{
			dealer.SmallFavor(playerId, RZ_EFaction.MILITARY, 5);
			return;
		}

		if (w.m_iIndex == 1)
		{
			said.Insert("The fence will hear who came asking for Bastion. Fences always do.");
			return;
		}

		if (RZ_Witness.Seen(gm, playerId, e.m_vSubject, RZ_EFaction.MILITARY))
			gm.AddInfamy(playerId, RZ_EFaction.MILITARY, 5, "sold a Bastion magazine to the crews", 0, "sold a soldier's magazine to a crew lookout");
	}
}

//------------------------------------------------------------------------------------------------
//! Sandbags: the post took a fight lately (a death or shells within 400 m in 3 played hours). Fill
//! and stack sandbags at the gap (2 real minutes at the gate).
class RZ_Routine_Sandbags : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "sandbags";
		m_sTitle = "Sandbags";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_sRoles = "sentry";
		Offer("The wire took a beating last night, and there's a gap where the sandbags went. Two high, before it's dark, or the post sleeps with a hole in it. Shovels by the gate.",
			"We've a gap in the line where the bags went last night. Fill, tie, stack. Two minutes. You'll ache.");
		Over("Gap's filled. The relief did it before you got the shovel.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fill and stack sandbags";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 120;
		w.m_fRadius = 20;
		w.m_sStep = "Fill and stack sandbags at the gap by the gate of {here} (2 minutes).";
		w.m_iFavor = 5;
		w.Done("Regulation height. Near enough. Thank you.", "That'll stop a bullet. Not a big one. A polite one.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		bool hit = RZ_ErrandCard.FreshSignal("death", e.m_vGiver, 400, 3, -1) || RZ_ErrandCard.FreshSignal("shell", e.m_vGiver, 400, 3, -1);
		if (!hit)
			return false;

		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".gap";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Rent's Due: a Red Hand camp within 1.2 km of the infected's ground, dusk within 20 real minutes.
//! "Every crossroads pays somebody. At dusk it's them." Count their pass from 150-250 m off, or make
//! a number up (a lie: x0.6, found if they reach the toll tonight).
class RZ_Routine_RentsDue : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "rents_due";
		m_sTitle = "Rent's Due";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		m_sRoles = "toll man,lookout";
		m_bTimed = true;
		Offer("Every crossroads pays somebody. At dusk, ours pays {lot}: they walk right past the toll. Get up on the lorry roof and count them past, so we know whether to shut the toll early. Don't wave.",
			"Rent's due. Theirs, not ours. Count {lot} going by the toll at dusk so we know how many to not argue with.");
		Over("Never mind the count. Somebody's done it, or they didn't come. Either way, drink up.");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Count them from the lorry roof";
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 120;
		w.m_fRadius = 40;
		w.m_bUnseen = true;
		w.m_sStep = "Count {lot} passing from {place} without being seen (2 minutes), then tell {giver}.";
		w.m_iFavor = 6;
		w.Done("That many. Right. We'll be indoors at dusk, then, toll or no toll.");

		w = Way("lie");
		w.m_sLabel = "Make a number up";
		w.m_iFavor = 3;
		w.Done("That few? Lovely. Toll stays open, then.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Tribes tribes = dealer.GetGM().GetTribes();
		if (!tribes || !RZ_GameTime.IsCalibrated() || RZ_GameTime.RealMinutesUntil(19.5) > 20)
			return false;

		RZ_Tribe t = tribes.NearestTribe(e.m_vGiver, 4000);
		if (!t || RZ_Util.DistXZ(t.m_vLair, e.m_vGiver) - t.m_fRadius > 1200)
			return false;

		e.m_iValue = t.m_iId;
		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".rent";
		e.SetText("lot", "the lot from " + t.m_sPlace);
		return true;
	}

	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 1)
			return;

		RZ_GameModeComponent gm = dealer.GetGM();
		gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.rent", e.m_sSubject, RZ_EFaction.BANDITS, e.m_sGiver, e.m_vGiver, RZ_ELieTest.RUMOUR, 3,
			"They came past the toll in numbers last night. The lad remembers the number you gave him. So does his arm.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Number: night at a Halcyon station or the lab, a trial dose that didn't take today. A quiet
//! hour with the night shift. Never the name onto the paper. No pay. After the cure too.
class RZ_Routine_TheNumber : RZ_ErrandDef
{
	override void Define()
	{
		m_sKey = "the_number";
		m_sTitle = "The Number";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		m_sRoles = "lab assistant,technician";
		Offer("A trial dose didn't take today. One of the other two. We write a number in the notebook, not a name. Would you sit with the night shift? We're not good company. That's rather the point.",
			"Tonight's volunteer didn't walk out. Somebody wrote the time and the number, and then we all went back to work. Sit with us a while. You needn't talk. We don't.");
		Over("It's all right. The shift changed. The new ones didn't see it.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Sit with the night shift";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 90;
		w.m_fRadius = 15;
		w.m_sStep = "Sit with the night shift at {here} a while (90 s).";
		w.Done("Thank you. The number stays a number. But someone sat with it.");
	}

	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_GameTime.IsNight())
			return false;

		RZ_Signal s = RZ_ErrandCard.FreshSignal("dose", e.m_vGiver, 6000, RZ_SeasonClock.DayMinutes() / 60, -1);
		if (!s || s.text.IndexOf("didn't") < 0)
			return false;

		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".number";
		return true;
	}
}
