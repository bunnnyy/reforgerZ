//! The stranger routines (docs/ERRANDS.md, "Strangers"): one class each. Every one lists the spot
//! kinds it fits (RZ_StrangerSpot) and a Why read from the world, and is written for a figure who
//! stands still. They pay nothing, a line, one common find, or the house's few Favor; a stranger you
//! help may become a local (World/RZ_Locals.c). Server only.
class RZ_StrangerKind
{
	string m_sKey;					// telemetry and logs: never shown
	int m_iSpots;					// RZ_StrangerSpot bits
	int m_iFaction = RZ_EFaction.SURVIVORS;	// whose people they count as (harm)
	bool m_bDark;					// Missions > Errands > Dark cards
	float m_fWeight = 1;
	string m_sLabels;

	//------------------------------------------------------------------------------------------------
	static void Build(notnull array<ref RZ_StrangerKind> list)
	{
		list.Insert(new RZ_Stranger_Roof());
		list.Insert(new RZ_Stranger_Tin());
		list.Insert(new RZ_Stranger_Funeral());
		list.Insert(new RZ_Stranger_WhichWay());
		list.Insert(new RZ_Stranger_Door());
		list.Insert(new RZ_Stranger_TwoInTheBack());
		list.Insert(new RZ_Stranger_Digger());
		list.Insert(new RZ_Stranger_StillWalking());
		list.Insert(new RZ_Stranger_WarmHands());
		list.Insert(new RZ_Stranger_SeedPotatoes());
		list.Insert(new RZ_Stranger_Manners());
		list.Insert(new RZ_Stranger_Sister());
		list.Insert(new RZ_Stranger_Toll());
		list.Insert(new RZ_Stranger_Line());
		list.Insert(new RZ_Stranger_Notice());
		list.Insert(new RZ_Stranger_Hiding());
		list.Insert(new RZ_Stranger_First());
		list.Insert(new RZ_Stranger_Bitten());
	}

	void Define() {}

	//------------------------------------------------------------------------------------------------
	//! True now at this spot: fill the visit (its house, its place, its text). Spawns nothing.
	bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Still true while they wait (every 30 s). False: they've gone.
	bool Still(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return true;
	}

	string CallOut(RZ_Strangers s, RZ_StrangerVisit v) { return ""; }

	//------------------------------------------------------------------------------------------------
	string Labels(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return m_sLabels;
	}

	void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user) {}

	//------------------------------------------------------------------------------------------------
	//! A way that asked you to go somewhere or stay a while is done.
	void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
	}

	// ================================================================ Shared reads
	//------------------------------------------------------------------------------------------------
	static bool Bg(RZ_Strangers s, int playerId, string ids)
	{
		return RZ_QuestApproach.HasBackground(s.GetGM(), playerId, ids);
	}

	//------------------------------------------------------------------------------------------------
	static bool AtLeast(RZ_Strangers s, int playerId, int faction, RZ_ETier tier)
	{
		return RZ_Reputation.IsAtLeast(s.GetGM().GetTier(playerId, faction), tier);
	}

	//------------------------------------------------------------------------------------------------
	//! Kill on Sight with anyone.
	static bool OnTheNotice(RZ_Strangers s, int playerId)
	{
		RZ_GameModeComponent gm = s.GetGM();
		for (int f = 0; f < 4; f++)
		{
			if (gm.GetTier(playerId, f) == RZ_ETier.KILL_ON_SIGHT)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A Haven-anchored stranger refuses an exile, as Haven wanderers do.
	static bool HavenWillTalk(RZ_Strangers s, int playerId)
	{
		RZ_Safehouses houses = s.GetGM().GetSafehouses();
		return houses && !houses.IsExiled(playerId) && !RZ_Reputation.IsHunted(s.GetGM().GetTier(playerId, RZ_EFaction.SURVIVORS));
	}

	//------------------------------------------------------------------------------------------------
	//! A road post of `faction` within 200 m of the way from a to b (not within 80 m of a): its place.
	static bool PostBetween(RZ_Strangers s, int faction, vector a, vector b, out vector post)
	{
		RZ_Checkpoints posts = s.GetGM().GetCheckpoints();
		if (!posts)
			return false;

		array<RZ_RoadPost> near = {};
		vector mid = (a + b) * 0.5;
		posts.PostsNear(faction, mid, RZ_Util.DistXZ(a, b) * 0.5 + 200, near);
		foreach (RZ_RoadPost p : near)
		{
			if (RZ_Util.DistXZ(p.m_vPos, a) >= 80 && RZ_Strangers.DistToSegment(p.m_vPos, a, b) <= 200)
			{
				post = p.m_vPos;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe's range the way from a to b crosses (its middle within the range): its place, "" if none.
	static string RangeBetween(RZ_Strangers s, vector a, vector b)
	{
		RZ_Tribes tribes = s.GetGM().GetTribes();
		if (!tribes)
			return "";

		vector mid = (a + b) * 0.5;
		RZ_Tribe t = tribes.NearestTribe(mid, 3000);
		if (t && RZ_Util.DistXZ(t.m_vLair, mid) <= t.m_fRadius)
			return t.m_sPlace;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Take food from the player; false (and a line) if they've none.
	static bool TakeFood(RZ_Strangers s, RZ_StrangerVisit v, int playerId, IEntity user, int count)
	{
		if (user && RZ_HavenQuest.TakeFood(s.GetGM(), user, count))
			return true;

		s.Say(v, playerId, "You reach for a tin and there isn't one. They see. \"Never mind. Thanks for looking.\"");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Make the one you helped a local at their house.
	static void Local(RZ_Strangers s, RZ_StrangerVisit v, int playerId, string role, string line, string spot, string memo)
	{
		RZ_Errands errands = s.GetErrands();
		if (errands && v.m_iHouse >= 0)
			errands.GetLocals().Make(playerId, role, line, v.m_iHouse, spot, false, memo);
	}

	//------------------------------------------------------------------------------------------------
	//! A house within 1.5 km for them, the visit's house; false if none.
	static bool AnyHouse(RZ_Strangers s, RZ_StrangerVisit v, float radius)
	{
		v.m_iHouse = s.NearestHouse(v.m_vPos, radius);
		if (v.m_iHouse < 0)
			return false;

		v.SetText("house", s.HouseName(v.m_iHouse));
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Up on the Roof (replaces Rooftop Rescue): a town the infected took, or a fresh shelling or fire,
//! within 800 m; someone up on a roof since.
class RZ_Stranger_Roof : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "roof";
		m_iSpots = RZ_StrangerSpot.FRESH | RZ_StrangerSpot.EDGE;
		m_sLabels = "Talk them down|Leave them";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		array<RZ_Reason> reasons = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(v.m_vPos, 800, reasons, "town.infested");
		bool hit = !reasons.IsEmpty() || RZ_ErrandCard.FreshSignal("shell", v.m_vPos, 800, 2, -1) || RZ_ErrandCard.FreshSignal("fire", v.m_vPos, 800, 2, -1);
		return hit && AnyHouse(s, v, 3000);
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Up here. On the roof. Are they gone? Tell me they're gone. I've been holding on up here since the noise started, and my arm's going.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option != 0)
		{
			s.Finish(v, playerId, "\"Don't go. Please.\" You go. The roof is quiet behind you.", false);
			return;
		}

		RZ_GameModeComponent gm = s.GetGM();
		if (gm.GetSpawner() && gm.GetSpawner().IsZombieNear(v.m_vPos, 25))
		{
			s.Say(v, playerId, "\"Not with them down there.\" Clear the street first.");
			return;
		}

		s.ToHouse(v.m_iHouse);
		s.Finish(v, playerId, "They come down the drainpipe faster than you'd think, and go off toward {house} without looking back.", true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 4);
		Local(s, v, playerId, "who came down off a roof", "You talked me off that roof. I still can't look at a ladder.", "bench", "helped");
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Tin: you carry 2+ food, a house within 1.5 km is on half portions. Never to the face on
//! the notice.
class RZ_Stranger_Tin : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "tin";
		m_iSpots = RZ_StrangerSpot.ROAD | RZ_StrangerSpot.APPROACH | RZ_StrangerSpot.ROUTE;
		m_sLabels = "Give him a tin|Trade two tins for his watch|Not today";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		RZ_GameModeComponent gm = s.GetGM();
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(v.m_iAsker);
		if (!me || RZ_HavenQuest.CountFood(gm, me) < 2 || OnTheNotice(s, v.m_iAsker) || !HavenWillTalk(s, v.m_iAsker))
			return false;

		array<RZ_Reason> reasons = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(v.m_vPos, 1500, reasons, "house.hungry");
		if (reasons.IsEmpty())
			return false;

		v.m_iHouse = RZ_ErrandCard.HouseNear(gm, v.m_vPos, 1500, -1, 1);
		if (v.m_iHouse < 0)
			return false;

		v.SetText("house", s.HouseName(v.m_iHouse));
		// His tip: which way the infected walk, from the world's own facts
		RZ_Tribes tribes = gm.GetTribes();
		RZ_Tribe t;
		if (tribes)
			t = tribes.NearestTribe(v.m_vPos, 3000);

		array<RZ_Reason> towns = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(v.m_vPos, 3000, towns, "town.infested");
		if (t)
			v.SetText("tip", string.Format("The lot from %1 come out %2 of the house at dusk. Go the other way.", t.m_sPlace, RZ_Tribes.CompassWord(gm.GetSafehouses().Get(v.m_iHouse).m_vPos, t.m_vLair)));
		else if (!towns.IsEmpty())
			v.SetText("tip", string.Format("%1 Go round it, not through.", towns[0].text));
		else
			v.SetText("tip", "Keep off the low road after dark. That's all I know, and it's true.");

		return true;
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Got a tin? Just the one. {house} is on half portions and I've a girl there. Look, the sack's empty. I'm not lying to you.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option == 2)
		{
			s.Finish(v, playerId, "\"I'd eat a Bastion ration. That's how bad it is.\" He goes back up the road.", false);
			return;
		}

		if (option == 1)
		{
			if (!TakeFood(s, v, playerId, user, 2))
				return;

			// A real watch when Missions > Errands > Props > Watch is set, else a common find
			ResourceName watch;
			RZ_ErrandSettings es = RZ_ErrandSettings.Get();
			if (es.m_Props)
				watch = es.m_Props.m_sWatch;

			IEntity item;
			if (!watch.IsEmpty() && user)
				item = RZ_Util.SpawnPrefab(watch, RZ_Util.GroundPos(user.GetOrigin()) + Vector(0, 0.3, 0));

			if (item)
				RZ_Util.InsertIntoInventory(user, item);
			else
				s.Gift(playerId, user);

			s.Finish(v, playerId, "He unbuckles the watch slowly, like it's the last thing he owns. It might be. \"It runs fast. So does everything.\"", true);
			return;
		}

		if (!TakeFood(s, v, playerId, user, 1))
			return;

		s.Finish(v, playerId, "He holds the tin with both hands. Then, quietly: {tip}", true);
		Local(s, v, playerId, "who had the empty sack", "You gave me a tin on the road. The girl had it with bread. Well, with water.", "gate", "helped");
	}
}

//------------------------------------------------------------------------------------------------
//! The Funeral for the Car: by day, a wreck on a road outside a town nobody lives in. His wife's car.
class RZ_Stranger_Funeral : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "funeral";
		m_iSpots = RZ_StrangerSpot.CAR;
		m_sLabels = "Push it into the ditch|Look under the bonnet|Leave him to it";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (here.m_bOwnCar || here.m_vCar == vector.Zero || RZ_GameTime.IsNight())
			return false;

		int town = gm.FindNearestSettlement(here.m_vCar);
		RZ_Settlement st = gm.GetSettlement(town);
		if (!st || RZ_WorldFacts.Get().IsLivedIn(town) || RZ_Util.DistXZ(st.m_vCenter, here.m_vCar) > st.m_fRadius + 600)
			return false;

		v.m_vSubject = here.m_vCar;
		v.m_vPos = RZ_Mission.LandPointNear(here.m_vCar, 4, 8);
		v.SetText("town", st.m_sName);
		return true;
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "This was my wife's car. She drove it to {town} every Thursday. Take what's in the boot if you want, she'd want it used. Then help me put it in the ditch, would you? Out of the road.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option == 2)
		{
			s.Finish(v, playerId, "You leave him with his hand on the roof. He doesn't look up.", false);
			return;
		}

		if (option == 1)
		{
			if (!Bg(s, playerId, "mechanic"))
			{
				s.Say(v, playerId, "You look. It's an engine. He waits for you to say something, and you don't.");
				return;
			}

			s.Finish(v, playerId, "\"It's not dead,\" you tell him. \"It needs a battery.\" He goes very still. \"There's a mechanic on the roads. I'll tell him.\" He walks off fast, the first time in weeks, by the look of it.", true);
			return;
		}

		s.StartStep(v, 0, v.m_vSubject, 10, 10, false, "Put your shoulder to the car with him (10 s at the car).");
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		s.GetGM().GetHeat().AddHeat(v.m_vSubject, 8);
		s.Finish(v, playerId, "It goes over the edge with a sound like furniture. He stands there a while. \"Thursdays,\" he says, to nobody.", true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
	}
}

//------------------------------------------------------------------------------------------------
//! Which Way's Morton: she's walking to a house 600 m-1.5 km on, and a road post or a range sits
//! between.
class RZ_Stranger_WhichWay : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "which_way";
		m_iSpots = RZ_StrangerSpot.ROAD | RZ_StrangerSpot.ROUTE;
		m_sLabels = "Point her round it|Walk her past it|Tell her the toll's cheap";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (!HavenWillTalk(s, v.m_iAsker) || !AnyHouse(s, v, 1500))
			return false;

		RZ_Safehouse h = s.GetGM().GetSafehouses().Get(v.m_iHouse);
		if (RZ_Util.DistXZ(h.m_vPos, v.m_vPos) < 600)
			return false;

		vector post;
		string range = RangeBetween(s, v.m_vPos, h.m_vPos);
		if (PostBetween(s, -1, v.m_vPos, h.m_vPos, post))
		{
			v.m_vSubject = post;
			v.SetText("trouble", "the post on the road");
		}
		else if (!range.IsEmpty())
		{
			v.SetText("trouble", "the woods where the lot from " + range + " sleep");
		}
		else
		{
			return false;
		}

		v.SetText("town", h.m_sName);
		return true;
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Which way's {town}? The straight way goes past {trouble}, and I'd rather not. It's only pans in the pram. Everything I could carry.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (option == 0)
		{
			s.ToHouse(v.m_iHouse);
			s.Finish(v, playerId, "You point out the long way round. She repeats it back to you twice, then sets off pushing the pans.", true);
			return;
		}

		if (option == 1)
		{
			RZ_Safehouse h = gm.GetSafehouses().Get(v.m_iHouse);
			if (!h)
				return;

			s.StartStep(v, 1, RZ_Strangers.Toward(v.m_vPos, h.m_vPos, 500), 5, 25, false, "Walk with her past {trouble}. She'll go on alone from about 500 m.");
			return;
		}

		// A lie: found if a post really stands there when someone who knows her passes it
		if (v.m_vSubject != vector.Zero && gm.GetFacts())
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.which_way", "post:" + RZ_SettlementScanner.GridReference(v.m_vSubject), RZ_EFaction.SURVIVORS, "",
				v.m_vSubject, RZ_ELieTest.VISIT, 2, "Word came back from {town}: the toll wasn't cheap, and someone on the road said it was.");

		s.Finish(v, playerId, "\"Cheap. Right.\" She doesn't sound sure. She goes the straight way anyway.", false);
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		s.ToHouse(v.m_iHouse);
		s.Finish(v, playerId, "\"I can see the smoke from here.\" She goes on with the pans clattering, and waves once without turning round.", true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
		Local(s, v, playerId, "with the pram full of pans", "You walked me past it. I still have the pans. Want one? I've too many pans.", "stove", "helped");
	}
}

//------------------------------------------------------------------------------------------------
//! Behind the Door (dialogue's "hiding"): a town edge at dusk (or in fog), in a town the infected
//! hold. Someone wants out before they stir.
class RZ_Stranger_Door : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "door";
		m_iSpots = RZ_StrangerSpot.EDGE;
		m_sLabels = "Walk them out now|Come back for them at dawn|Leave them some food";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		RZ_GameModeComponent gm = s.GetGM();
		int town = gm.FindNearestSettlement(v.m_vPos);
		RZ_Settlement st = gm.GetSettlement(town);
		if (!st || !RZ_WorldFacts.Get().Memory().FindReason("town.infested", RZ_WorldFacts.Town(town)))
			return false;

		v.m_vSubject = st.m_vCenter;
		v.SetText("town", st.m_sName);
		return AnyHouse(s, v, 1500);
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Are you real? Don't come closer, just talk. They come out when it's dark. I've been behind this door since Tuesday. Is there anywhere left?";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (option == 0)
		{
			RZ_Safehouse h = gm.GetSafehouses().Get(v.m_iHouse);
			if (!h)
				return;

			// A noise on the far side of town draws them the other way, under the built heat rules
			vector far = v.m_vSubject + (v.m_vSubject - v.m_vPos);
			gm.GetHeat().AddHeat(RZ_Util.GroundPos(far), 20);
			s.StartStep(v, 0, RZ_Strangers.Toward(v.m_vPos, h.m_vPos, 600), 5, 25, false, "Walk them out toward {house}, quickly. Somewhere on the far side of {town} a gate bangs and keeps banging.");
			return;
		}

		if (option == 1)
		{
			s.Later(v, playerId);
			s.Finish(v, playerId, "\"Dawn. You'll come at dawn.\" The door closes. You hear something heavy pushed back against it.", false);
			return;
		}

		if (!TakeFood(s, v, playerId, user, 1))
			return;

		s.Finish(v, playerId, "A hand takes the tin through the gap. \"Thank you. I'll make it last. I'm good at that now.\"", true);
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		s.ToHouse(v.m_iHouse);
		s.Finish(v, playerId, "They don't stop shaking until they can see {house}'s fence. Then they walk the last of it on their own.", true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
		Local(s, v, playerId, "who hid behind the door", "You came and got me out. I sleep with the light on. They let me.", "bench", "saved");
	}
}

//------------------------------------------------------------------------------------------------
//! Two in the Back (dialogue's "roadside"): by day, a family's broken car on a road people use.
class RZ_Stranger_TwoInTheBack : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "two_in_back";
		m_iSpots = RZ_StrangerSpot.CAR;
		m_sLabels = "Fix it|Give them a battery|Push-start it downhill";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (here.m_bOwnCar || here.m_vCar == vector.Zero || RZ_GameTime.IsNight() || !HavenWillTalk(s, v.m_iAsker))
			return false;

		v.m_vSubject = here.m_vCar;
		v.m_vPos = RZ_Mission.LandPointNear(here.m_vCar, 3, 6);
		return AnyHouse(s, v, 1500);
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "It was going. Then it wasn't. I've two kids in the back who think this is an adventure. Don't tell them otherwise.";
	}

	override string Labels(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Fix it [Mechanic]|Give them a battery [1 battery]|Push-start it downhill";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option == 0)
		{
			if (!Bg(s, playerId, "mechanic"))
			{
				s.Say(v, playerId, "No offence, but you're looking at it the way I was.");
				return;
			}

			s.StartStep(v, 0, v.m_vSubject, 60, 10, false, "Fix the car (a minute at the bonnet).");
			return;
		}

		if (option == 1)
		{
			if (RZ_Errands.TakeItems(user, RZ_EErrandItem.BATTERY, 1) < 1)
			{
				s.Say(v, playerId, "\"A battery, you said?\" You haven't one on you.");
				return;
			}

			Done(s, v, playerId);
			return;
		}

		s.StartStep(v, 2, v.m_vSubject, 30, 10, false, "Push with him while he lets the clutch out (30 s at the car).");
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		if (option == 2)
			s.GetGM().GetHeat().AddHeat(v.m_vSubject, 6);

		Done(s, v, playerId);
	}

	protected void Done(RZ_Strangers s, RZ_StrangerVisit v, int playerId)
	{
		s.Finish(v, playerId, "It catches. The kids cheer like it's a football match. They drive on to {house} with the windows down.", true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
		Local(s, v, playerId, "with the car and the two kids", "The kids still call you the car man. I've stopped correcting them.", "gate", "helped");
	}
}

//------------------------------------------------------------------------------------------------
//! The Digger (dialogue's "grave"): a death on a road within 400 m in the last 2 played hours.
class RZ_Stranger_Digger : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "digger";
		m_iSpots = RZ_StrangerSpot.FRESH;
		m_sLabels = "Help her dig|Tell her who it was|Take the dead's things";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (here.m_sFresh != "death" || !HavenWillTalk(s, v.m_iAsker))
			return false;

		v.m_vSubject = here.m_vFresh;
		v.m_vPos = RZ_Mission.LandPointNear(here.m_vFresh, 4, 10);
		v.SetText("who", here.m_sFreshText);
		return AnyHouse(s, v, 3000);
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Nobody should lie by a road. Not even a road this quiet. It's a coal shovel, it's all I had, and it'll do.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (option == 0)
		{
			s.StartStep(v, 0, v.m_vPos, 30, 10, false, "Dig with her (30 s).");
			return;
		}

		if (option == 1)
		{
			string who = v.m_mText.Get("who");
			if (who.IsEmpty())
			{
				s.Say(v, playerId, "You don't know who it was. She nods. \"Nobody does. That's the worst of it.\"");
				return;
			}

			s.Finish(v, playerId, "You tell her what you know: " + who + " She says it back to herself, so she'll remember it for the cross.", true);
			return;
		}

		// She's right there: she always sees
		gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 3, "robbed the dead in front of the digger", 0, "went through a dead man's pockets at a graveside");
		s.Gift(playerId, user);
		s.Finish(v, playerId, "She watches you go through the pockets and doesn't say a word. She keeps digging. She'll tell the house, though.", false);
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		s.GetGM().GetHeat().AddHeat(v.m_vPos, 4);
		// Traces' P3: the grave stays by the road (World/Traces/RZ_PlayerTraces.c)
		RZ_Traces traces = RZ_Traces.Get();
		if (traces && traces.IsOn())
			traces.GetPlayers().DiggerGrave(v.m_vPos, v.m_mText.Get("who"));

		string end = "\"There. That's someone's.\" She knocks the soil off the shovel.";
		if (Bg(s, playerId, "elder"))
			end = "You say the words. She mouths them with you, a beat behind, the way people do at funerals they didn't plan to go to.";

		s.Finish(v, playerId, end, true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
		Local(s, v, playerId, "who digs by the road", "You helped me with the one by the road. I go back and tidy it. Someone should.", "bench", "helped");
	}
}

//------------------------------------------------------------------------------------------------
//! Still Walking (dialogue's "walk_out"): an approach at dusk, the day after a man was walked out of
//! a house and let walk on (Walk Him Out's record `walk.out`). He wants the way to the lab.
class RZ_Stranger_StillWalking : RZ_StrangerKind
{
	protected ref array<string> m_aUsed = {};

	override void Define()
	{
		m_sKey = "still_walking";
		m_iSpots = RZ_StrangerSpot.APPROACH;
		m_sLabels = "Point him to the lab|Walk him toward it|Sit with him till dark";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (!RZ_WorldFacts.Get().IsDusk() || !gm.HasLab() || (gm.GetCure() && gm.GetCure().IsCured()))
			return false;

		// Never while the Edge of Town questline runs (it owns the walked)
		if (gm.GetQuestlines() && gm.GetQuestlines().IsRunning("walk"))
			return false;

		RZ_WorldRecord r = RZ_WorldFacts.Get().Memory().NewestRecord("walk.out", 1);
		if (!r || r.value != "walked on" || m_aUsed.Contains(r.subject))
			return false;

		v.m_sSubject = r.subject;
		v.SetText("name", r.subject);
		v.m_vSubject = gm.GetLabPosition();
		return true;
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "They said there's a lab that gives a shot. For, you know. Is that true, or is it the sort of thing people say?";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		m_aUsed.Insert(v.m_sSubject);
		if (option == 0)
		{
			s.Finish(v, playerId, "You point. He looks the way you point for a long time, then goes. The lab's gate lets a bitten, unarmed man through for the trial dose. He knows that now.", true);
			Dosed(s, v);
			return;
		}

		if (option == 1)
		{
			s.StartStep(v, 1, RZ_Strangers.Toward(v.m_vPos, v.m_vSubject, 600), 5, 25, false, "Walk him toward the lab. He'll go on from about 600 m.");
			return;
		}

		float sit = 120;
		if (Bg(s, playerId, "nurse,doctor"))
			sit = 60;

		s.StartStep(v, 2, v.m_vPos, sit, 10, false, "Sit with him until it's properly dark.");
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		if (option == 1)
		{
			s.Finish(v, playerId, "\"I can find it from here.\" He shakes your hand like a man at a station. He goes on.", true);
			Dosed(s, v);
			return;
		}

		s.Finish(v, playerId, "You sit. He talks about his garden, mostly. When it's dark he gets up, says \"Right,\" and walks toward the lab.", true);
		Dosed(s, v);
	}

	//------------------------------------------------------------------------------------------------
	//! He got the trial dose at the lab's gate: a note on his house's board, in his hand.
	protected void Dosed(RZ_Strangers s, RZ_StrangerVisit v)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int house = s.NearestHouse(v.m_vPos, 4000);
		if (!gm || house < 0)
			return;

		string name = v.m_sSubject;
		if (name.IsEmpty())
			name = "the man with his sleeve rolled down";

		RZ_StoryKit.BoardThanks(gm, gm.GetSafehouses().Get(house).m_vPos, "Got to the lab gate. They gave me the shot. I'm still walking, and I'm told that's the point.", name);
	}
}

//------------------------------------------------------------------------------------------------
//! Warm Hands: a campfire after dark, nobody else within 300 m. A voice from the dark.
class RZ_Stranger_WarmHands : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "warm_hands";
		m_iSpots = RZ_StrangerSpot.FIRE;
		m_sLabels = "Let him sit|Give him a tin|Send him off";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (!RZ_GameTime.IsNight() || here.m_vFire == vector.Zero)
			return false;

		v.m_vSubject = here.m_vFire;
		v.m_vPos = RZ_Mission.LandPointNear(here.m_vFire, 3, 6);
		v.m_iHouse = s.NearestHouse(v.m_vPos, 3000);
		return true;
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Don't shoot. My hands are up. Mind if I sit by you? I'm not armed. I'm just cold.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option == 2)
		{
			s.Finish(v, playerId, "\"Fair.\" He backs off into the dark the way he came.", false);
			return;
		}

		if (option == 1)
		{
			if (v.m_iHouse < 0 || !TakeFood(s, v, playerId, user, 1))
				return;

			v.SetText("house", s.HouseName(v.m_iHouse));
			s.Finish(v, playerId, "He eats it cold, with his fingers, by your fire. \"I'm at {house}, if you're ever by. Ask for the man who ate your tin.\"", true);
			Local(s, v, playerId, "who sat at your fire", "You let me sit at your fire. I still think about that fire.", "stove", "helped");
			return;
		}

		s.StartStep(v, 0, v.m_vSubject, 300, 15, false, "Let him sit with you by the fire a while (5 real minutes).");
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		s.Finish(v, playerId, "Before he goes, one true thing from the road: " + TrueThing(s, v.m_vSubject), true);
	}

	//------------------------------------------------------------------------------------------------
	//! A real habit or a real rumour, never made up.
	protected static string TrueThing(RZ_Strangers s, vector at)
	{
		RZ_Tribes tribes = s.GetGM().GetTribes();
		if (tribes)
		{
			RZ_Tribe t = tribes.NearestTribe(at, 3000);
			if (t && t.m_bFed)
				return string.Format("The lot from %1 ate last night. Tomorrow they'll sleep. Go then, if you're going.", t.m_sPlace);

			if (t)
				return string.Format("The lot from %1 go out at dusk and come back before the light. Every night. You could set a watch by it.", t.m_sPlace);
		}

		array<RZ_Reason> towns = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(at, 3000, towns, "town.infested");
		if (!towns.IsEmpty())
			return towns[0].text + " I came through it. Don't.";

		return "The road east is quieter than the road west. That's all I know for sure.";
	}
}

//------------------------------------------------------------------------------------------------
//! Seed Potatoes: their own place, by day: an empty village near the infected's ground. He buried a
//! sack on Day 2.
class RZ_Stranger_SeedPotatoes : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "seed_potatoes";
		m_iSpots = RZ_StrangerSpot.OWN;
		m_sLabels = "Dig it up for him|Say it was gone|Not my business";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (RZ_GameTime.IsNight() || !HavenWillTalk(s, v.m_iAsker))
			return false;

		int town = gm.FindNearestSettlement(v.m_vPos);
		RZ_Settlement st = gm.GetSettlement(town);
		if (!st || RZ_WorldFacts.Get().IsLivedIn(town) || RZ_Util.DistXZ(st.m_vCenter, v.m_vPos) > st.m_fRadius + 150)
			return false;

		// Near the infected's ground: a range within 1 km, or a town they hold
		RZ_Tribes tribes = gm.GetTribes();
		RZ_Tribe t;
		if (tribes)
			t = tribes.NearestTribe(v.m_vPos, 3000);

		bool near = t && RZ_Util.DistXZ(t.m_vLair, v.m_vPos) <= t.m_fRadius + 1000;
		array<RZ_Reason> towns = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(v.m_vPos, 1000, towns, "town.infested");
		if (!near && towns.IsEmpty())
			return false;

		v.m_vSubject = RZ_Mission.LandPointNear(v.m_vPos, 40, 80);
		v.SetText("town", st.m_sName);
		return AnyHouse(s, v, 1500);
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "This was mine. The farm, I mean, not the mess. I buried a sack of seed potatoes along that field edge on Day 2. Go the day after they've fed. Nobody's out then.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (option == 2)
		{
			s.Finish(v, playerId, "\"No. Course not.\" He goes back to looking at the field.", false);
			return;
		}

		if (option == 1)
		{
			// The sack goes to the fence for you; he believes you, because people always say that now
			s.HouseFavor(playerId, RZ_EFaction.BANDITS, 4);
			if (gm.GetFacts())
				gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.seed_potatoes", "farm:" + RZ_SettlementScanner.GridReference(v.m_vSubject), RZ_EFaction.SURVIVORS, "",
					v.m_vSubject, RZ_ELieTest.RUMOUR, 2, "He went and looked. The hole was fresh. {house} knows who said it was gone.");

			s.Finish(v, playerId, "\"Gone. Course it is.\" He believes you. People always say that now.", false);
			return;
		}

		s.StartStep(v, 0, v.m_vSubject, 10, 15, false, "Dig along the field edge where he says (10 s).");
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		gm.GetHeat().AddHeat(v.m_vSubject, 6);
		gm.GetSafehouses().AddFood(v.m_iHouse, 2);
		s.Finish(v, playerId, "A sack, damp, and the potatoes inside have eyes on them like they're looking for something. He takes it to {house}.", true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 4);
	}
}

//------------------------------------------------------------------------------------------------
//! Checkpoint Manners: at a Bastion post's crossroads, a Haven family with the post between them
//! and their house, carrying a wrapped bundle (their father's rifle part, "for the rabbits").
class RZ_Stranger_Manners : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "manners";
		m_iSpots = RZ_StrangerSpot.CROSSROADS;
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		RZ_Checkpoints posts = s.GetGM().GetCheckpoints();
		if (!posts || here.m_iPost < 0 || !HavenWillTalk(s, v.m_iAsker))
			return false;

		RZ_RoadPost post = posts.FindPost(here.m_iPost);
		if (!post || post.m_eFaction != RZ_EFaction.MILITARY || !AnyHouse(s, v, 1500))
			return false;

		RZ_Safehouse h = s.GetGM().GetSafehouses().Get(v.m_iHouse);
		if (RZ_Strangers.DistToSegment(post.m_vPos, v.m_vPos, h.m_vPos) > 200)
			return false;

		v.m_vSubject = post.m_vPos;
		return true;
	}

	override string Labels(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Say they're with you [Known: Bastion]|Walk them round the post|Hand the bundle in at the barrier";
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "The bundle's their grandfather's. In the tea towel. It's for the rabbits. They search everything at that post now, don't they?";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option == 0)
		{
			if (!AtLeast(s, playerId, RZ_EFaction.MILITARY, RZ_ETier.KNOWN))
			{
				s.Say(v, playerId, "You'd have to be someone the post knows. You aren't, yet.");
				return;
			}

			s.ToHouse(v.m_iHouse);
			s.Finish(v, playerId, "The sentry knows your face and waves the lot of you through. They still search the family, on their own terms. Somehow the tea towel never comes up.", true);
			s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
			Rumour(s);
			return;
		}

		if (option == 1)
		{
			s.StartStep(v, 1, RZ_Mission.LandPointNear(v.m_vSubject, 180, 230), 5, 30, false, "Lead them round the post, well off the road.");
			return;
		}

		s.ToHouse(v.m_iHouse);
		s.HouseFavor(playerId, RZ_EFaction.MILITARY, 2);
		s.Finish(v, playerId, "The sergeant unwraps it, looks at it, and writes something down. The family goes home without it, and without a word to you.", false);
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		s.ToHouse(v.m_iHouse);
		s.Finish(v, playerId, "Past the post, the boys run ahead. The mother says thank you twice, once for each of them.", true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
		Rumour(s);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Rumour(RZ_Strangers s)
	{
		RZ_WorldFacts facts = s.GetGM().GetFacts();
		if (facts)
			facts.AddRumour("Somebody sold the Red Hand a rifle part \"for the rabbits\". Got a week of tins for it, they say.", 6);
	}
}

//------------------------------------------------------------------------------------------------
//! My Sister at Morton: a house lost people lately, and a man on the road asks after his sister. Half
//! the time he's her brother; half the time a crew lad (a red hand on the wrist), named to a Police or
//! Ex-Con background, or Known with the Red Hand. Knowingly sending a crew lad to her is selling her.
class RZ_Stranger_Sister : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "sister";
		m_iSpots = RZ_StrangerSpot.ROAD | RZ_StrangerSpot.ROUTE;
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (!HavenWillTalk(s, v.m_iAsker) || !AnyHouse(s, v, 1500))
			return false;

		RZ_HouseFacts facts = RZ_WorldFacts.Get().House(v.m_iHouse);
		if (!facts.m_bValid || facts.m_iLost <= 0)
			return false;

		v.m_iValue = RZ_ErrandCard.HouseNear(gm, v.m_vPos, 2500, v.m_iHouse, 0);
		array<string> truths = {"brother", "crew"};
		array<float> weights = {0.5, 0.5};
		string subject = RZ_WorldFacts.Safehouse(v.m_iHouse);
		v.m_bTwist = RZ_WorldFacts.Get().Memory().DrawTruth("sister", subject, truths, weights) == "crew";
		v.m_vSubject = RZ_Mission.LandPointNear(v.m_vPos, 600, 1200);
		int barnTown = gm.FindNearestSettlement(v.m_vSubject);
		RZ_Settlement st = gm.GetSettlement(barnTown);
		string barn = "a barn " + RZ_Util.DirectionText(v.m_vPos, v.m_vSubject);
		if (st)
			barn = "a barn near " + st.m_sName;

		v.SetText("barn", barn);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Knows(RZ_Strangers s, int playerId)
	{
		return Bg(s, playerId, "police,excon") || AtLeast(s, playerId, RZ_EFaction.BANDITS, RZ_ETier.KNOWN);
	}

	override string Labels(RZ_Strangers s, RZ_StrangerVisit v)
	{
		// The same replies show whether or not he's lying; the tell is named only to those who'd see it
		if (v.m_bTwist && Knows(s, v.m_iAsker))
			return "Find her and tell him|Warn her on to another house|Call him out [a red hand on his wrist]";

		return "Find her and tell him|Warn her on to another house|Call him out";
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		if (v.m_bTwist && Knows(s, v.m_iAsker))
			return "My sister was put out of {house}. Upkeep, they called it. You'll have seen her on the road? I just want to know where she's got to.";

		return "My sister was put out of {house}. Upkeep, they called it. You'll have seen her on the road? She'll be in some barn. She always liked barns.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option == 2)
		{
			if (v.m_bTwist)
			{
				s.Finish(v, playerId, "\"Your sister has a lot of brothers.\" He looks at you, works out you're not worth it, and walks off the way he came.", true);
				return;
			}

			s.GetGM().AddInfamy(playerId, RZ_EFaction.SURVIVORS, 1, "called a grieving man a liar", 0, "");
			s.Finish(v, playerId, "\"Calling a grieving man a liar. Nice.\" He goes on looking on his own.", false);
			return;
		}

		s.StartStep(v, option, v.m_vSubject, 15, 40, false, "Look for her in {barn} (15 s there).");
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (option == 1)
		{
			// Warned on: she goes to another house
			int other = v.m_iValue;
			if (other < 0)
				other = v.m_iHouse;

			v.m_iHouse = other;
			v.SetText("house", s.HouseName(other));
			s.ToHouse(other);
			s.Finish(v, playerId, "She's there, asleep on a feed sack. You tell her someone's asking. She doesn't ask who. She goes to {house} before dark.", true);
			Local(s, v, playerId, "who was asleep in the barn", "You warned me on. I never asked who was looking. I didn't want to know.", "stove", "saved");
			return;
		}

		if (!v.m_bTwist)
		{
			s.Finish(v, playerId, "She's there. You go back and tell him which barn. Later you see them on the road together, arguing about something from when they were children.", true);
			s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
			return;
		}

		// He was a crew lad. Knowing, it's selling her; tricked, it costs you nothing
		RZ_ErrandSettings settings = RZ_ErrandSettings.Get();
		if (Knows(s, playerId) && settings.m_bDarkCards)
		{
			s.HouseFavor(playerId, RZ_EFaction.BANDITS, 6);
			if (RZ_Witness.Seen(gm, playerId, v.m_vSubject, RZ_EFaction.SURVIVORS))
				gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 12, "sold a woman to the crews", 0, "told a crew lad where a woman was sleeping");

			s.Finish(v, playerId, "You tell him which barn. He says thanks like he's paying for something, and in a way he is. She never arrives anywhere.", false);
			return;
		}

		s.Finish(v, playerId, "You tell him which barn. He goes off quick. Later you hear she never arrived anywhere, and you don't know why.", false);
	}
}

//------------------------------------------------------------------------------------------------
//! Wrong Side of the Toll (a dark card): a Haven woman with a Red Hand roadblock between her and her
//! house. Sell her to the toll, knowing: she never arrives.
class RZ_Stranger_Toll : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "wrong_side";
		m_iSpots = RZ_StrangerSpot.ROAD | RZ_StrangerSpot.CROSSROADS;
		m_bDark = true;
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (!HavenWillTalk(s, v.m_iAsker) || !AnyHouse(s, v, 1500))
			return false;

		RZ_Safehouse h = s.GetGM().GetSafehouses().Get(v.m_iHouse);
		vector post;
		if (!PostBetween(s, RZ_EFaction.BANDITS, v.m_vPos, h.m_vPos, post))
			return false;

		v.m_vSubject = post;
		return true;
	}

	override string Labels(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Pay her toll (10 Favor)|Say she's with you [Trusted: Red Hand]|Walk her round the toll|Sell her to the toll";
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Is that their toll? I've nothing to pay it with. Everything I own, I'm wearing. {house} is just past it.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (option == 0)
		{
			if (!gm.SpendFavor(playerId, RZ_EFaction.BANDITS, 10))
			{
				s.Say(v, playerId, "You haven't 10 Favor with the Red Hand to spend.");
				return;
			}

			s.ToHouse(v.m_iHouse);
			s.Finish(v, playerId, "The toll lad counts it twice and waves her through like he's doing her a favour. Technically, he is.", true);
			return;
		}

		if (option == 1)
		{
			if (!AtLeast(s, playerId, RZ_EFaction.BANDITS, RZ_ETier.TRUSTED))
			{
				s.Say(v, playerId, "The crews would need to know you a lot better than they do.");
				return;
			}

			s.ToHouse(v.m_iHouse);
			s.Finish(v, playerId, "\"She's with me.\" The lad shrugs. \"Your funeral. Hers, I mean.\" He lets her by.", true);
			return;
		}

		if (option == 2)
		{
			s.StartStep(v, 2, RZ_Mission.LandPointNear(v.m_vSubject, 200, 260), 5, 30, false, "Walk her round the toll, well off the road.");
			return;
		}

		// Sold, knowing: she never arrives. Seen by any of the Haven's walking with her, it's known
		s.HouseFavor(playerId, RZ_EFaction.BANDITS, 6);
		if (RZ_Witness.Seen(gm, playerId, v.m_vPos, RZ_EFaction.SURVIVORS))
			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 12, "sold a woman to the toll", 0, "walked a woman to the toll and left her there");

		s.GetErrands().HouseShy(playerId, v.m_iHouse);
		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.toll_sold", RZ_WorldFacts.Safehouse(v.m_iHouse), "sold", RZ_WorldMemory.SOURCE_ERRAND, false, who);
		s.Finish(v, playerId, "You walk her to the barrier and keep walking without her. Nobody says where she went.", false);
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		s.ToHouse(v.m_iHouse);
		s.Finish(v, playerId, "Past it, she takes one coat off and gives it to you. It doesn't fit. You keep it anyway.", true);
		Local(s, v, playerId, "in the two coats", "You walked me round the toll. I've still got your coat. Well, my coat.", "gate", "saved");
	}
}

//------------------------------------------------------------------------------------------------
//! She Waves at the Line (a dark card): the line, while the Zone is sealed. She stops at the warning
//! zone's edge to wave at a husband she thinks is outside. She never goes in.
class RZ_Stranger_Line : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "waves_at_line";
		m_iSpots = RZ_StrangerSpot.LINE;
		m_bDark = true;
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (here.m_vLine == vector.Zero)
			return false;

		v.m_vSubject = here.m_vLine;
		v.m_vPos = RZ_Mission.LandPointNear(here.m_vLine, 2, 8);
		return AnyHouse(s, v, 3000);
	}

	override string Labels(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Talk her back [Known: Haven, or Nurse]|Walk out there with her|Leave her be";
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "He's just there, past the line. Behind the trees. He waves back, sometimes. Can you see him?";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (option == 0)
		{
			if (!AtLeast(s, playerId, RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN) && !Bg(s, playerId, "nurse"))
			{
				s.Say(v, playerId, "She doesn't hear you. You're nobody she knows.");
				return;
			}

			s.ToHouse(v.m_iHouse);
			s.Finish(v, playerId, "You talk about nothing until she's walking with you. At {house} she lets them take her coat. She says she'll go back tomorrow. Maybe.", true);
			Local(s, v, playerId, "who waves at the line", "You walked me back from the line. I still go some days. I don't go as close.", "gate", "saved");
			return;
		}

		if (option == 1)
		{
			s.Finish(v, playerId, "You step past her into the warning zone. She stays where she is. The line's rules are for whoever crosses it, and that's you, alone.", false);
			return;
		}

		RZ_Leaders leaders = gm.GetLeaders();
		if (leaders && gm.GetErrands().PayKinds().MayHeadline())
			leaders.AddHeadline("A woman from the north road waves at the line every evening. The Haven leaves a chair there now.", false);

		s.Finish(v, playerId, "You leave her waving. When you look back she's still at it, steady, like a lighthouse.", false);
	}
}

//------------------------------------------------------------------------------------------------
//! The Face on the Notice: you're Kill on Sight with someone, on foot and alone. A desperate man wants
//! his landlord's debt book taken from a crew's hut within 1 km. The only stranger who doesn't run.
class RZ_Stranger_Notice : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "face_on_notice";
		m_iSpots = RZ_StrangerSpot.APPROACH | RZ_StrangerSpot.ROAD;
		m_sLabels = "Take the book, unseen|Burn the hut's books|Turn him down";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (!OnTheNotice(s, v.m_iAsker))
			return false;

		// Alone: nobody of your group within 300 m
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != v.m_iAsker && RZ_Util.DistXZ(p.GetOrigin(), v.m_vPos) <= 300)
				return false;
		}

		RZ_Territory territory = gm.GetTerritory();
		if (!territory || !territory.IsActive())
			return false;

		RZ_Holding hold = territory.FindHolding(RZ_EFaction.BANDITS, v.m_vPos, 1000);
		if (!hold || hold.m_bStronghold)
			return false;

		v.m_vSubject = RZ_Mission.LandPointNear(hold.m_vPos, 40, 70);
		v.SetText("hut", hold.m_sName);
		return true;
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "You're the face on the notice. Good. I'm not running. I need someone with nothing to lose. My landlord's book is in the crew hut at {hut}. What I owe is in it. All of it.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option == 2)
		{
			s.Finish(v, playerId, "\"Course. Even you.\" He sits down on the verge where he is.", false);
			return;
		}

		if (option == 0)
		{
			s.StartStep(v, 0, v.m_vSubject, 20, 25, true, "Get into the crew hut at {hut} unseen and take the book (20 s hidden there).");
			return;
		}

		s.StartStep(v, 1, v.m_vSubject, 15, 25, false, "Burn the books at the crew hut at {hut} (15 s there).");
	}

	override void StepDone(RZ_Strangers s, RZ_StrangerVisit v, int option, int playerId, IEntity user)
	{
		RZ_GameModeComponent gm = s.GetGM();
		if (option == 1)
		{
			gm.GetHeat().AddHeat(v.m_vSubject, 10);
			if (RZ_Witness.Seen(gm, playerId, v.m_vSubject, RZ_EFaction.BANDITS))
				gm.AddInfamy(playerId, RZ_EFaction.BANDITS, 8, "burned a crew's books", 0, "burned the books at a crew hut");
		}

		s.Gift(playerId, user);
		s.Finish(v, playerId, "When you come back he's still on the verge. He gives you what he has in his pocket, which is one thing, and walks off lighter.", true);
	}
}

//------------------------------------------------------------------------------------------------
//! A found hook: someone hiding at a job's site, at ground level. Talk them out to the nearest house.
class RZ_Stranger_Hiding : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "hook_hiding";
		m_iSpots = RZ_StrangerSpot.FRESH;
		m_sLabels = "Talk them out|Leave them";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (here.m_sFresh != "hook:hiding")
			return false;

		v.m_vSubject = here.m_vFresh;
		v.m_vPos = RZ_Mission.LandPointNear(here.m_vFresh, 10, 30);
		return AnyHouse(s, v, 3000);
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Is it over? I'm under the car. All that noise. I didn't come out. I'm sorry. I didn't come out.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		if (option != 0)
		{
			s.Finish(v, playerId, "You leave them where they are. It's what they asked for, nearly.", false);
			return;
		}

		s.UseHook(s.HookNear("hiding", v.m_vSubject, 50));
		s.ToHouse(v.m_iHouse);
		s.Finish(v, playerId, "They come out a bit at a time, like a tortoise. You point them toward {house}. They go.", true);
		s.HouseFavor(playerId, RZ_EFaction.SURVIVORS, 3);
	}
}

//------------------------------------------------------------------------------------------------
//! A found hook: someone got there first. Talk only: share, trade or leave.
class RZ_Stranger_First : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "hook_first";
		m_iSpots = RZ_StrangerSpot.FRESH;
		m_sLabels = "Share what's left|Trade a tin for something|Leave it to them";
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (here.m_sFresh != "hook:first")
			return false;

		v.m_vSubject = here.m_vFresh;
		v.m_vPos = RZ_Mission.LandPointNear(here.m_vFresh, 10, 30);
		return true;
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Hands where you can see them, look. I got here first. Didn't think anyone else would. We can be civil about it.";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		s.UseHook(s.HookNear("first", v.m_vSubject, 50));
		if (option == 0)
		{
			s.Finish(v, playerId, "You split it down the middle, which is to say there's nothing for either of you. You both laugh, which helps.", true);
			return;
		}

		if (option == 1)
		{
			if (!TakeFood(s, v, playerId, user, 1))
				return;

			s.Gift(playerId, user);
			s.Finish(v, playerId, "A tin for something from the bag. Fair, by today's prices. Nobody's are fair.", true);
			return;
		}

		s.Finish(v, playerId, "\"Decent of you.\" They go off with the bag, not hurrying.", false);
	}
}

//------------------------------------------------------------------------------------------------
//! A found hook: a bite on a stranger at a job's site. Send them to the nearest house, where the gate
//! checks everyone (Who's at the Gate).
class RZ_Stranger_Bitten : RZ_StrangerKind
{
	override void Define()
	{
		m_sKey = "hook_bitten";
		m_iSpots = RZ_StrangerSpot.FRESH;
	}

	override bool Why(RZ_Strangers s, RZ_StrangerVisit v, RZ_StrangerHere here)
	{
		if (here.m_sFresh != "hook:bitten")
			return false;

		v.m_vSubject = here.m_vFresh;
		v.m_vPos = RZ_Mission.LandPointNear(here.m_vFresh, 10, 30);
		return AnyHouse(s, v, 3000);
	}

	override string Labels(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "Send them to {house}|Look at the arm [Nurse or Doctor]|Leave them";
	}

	override string CallOut(RZ_Strangers s, RZ_StrangerVisit v)
	{
		return "It's a scratch. Off a fence. I'm just holding it so it stops throbbing. Is there a house near?";
	}

	override void Choose(RZ_Strangers s, RZ_StrangerVisit v, int playerId, int option, IEntity user)
	{
		s.UseHook(s.HookNear("bitten", v.m_vSubject, 50));
		if (option == 2)
		{
			s.Finish(v, playerId, "You leave them against the wall. They don't call after you.", false);
			return;
		}

		if (option == 1 && !Bg(s, playerId, "nurse,doctor"))
		{
			s.Say(v, playerId, "You'd need to know what you're looking at.");
			return;
		}

		s.SendToGate(v.m_iHouse, true);
		if (option == 1)
		{
			s.Finish(v, playerId, "It isn't a fence. It's a day old. You tell them straight, and send them to {house} anyway: the gate checks everyone, and someone there will sit with them.", true);
			return;
		}

		s.Finish(v, playerId, "They go off toward {house}, holding the arm. The gate checks everyone at the door. Nobody will blame you for what they find.", true);
	}
}
