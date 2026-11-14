//! The opening's favour (docs/DESIGN.md, "Openings", beat 2): the contact's problem, which fits
//! your background, and one named thing to fetch for him from a place that makes sense, then hand
//! over by a reply on his talk screen. Your background opens a second way through: a [Background]
//! reply on his talk screen while the fetch runs, which either settles it there and then (a reply
//! way) or hands a step of its own (a hold somewhere: treating his son on the spot, hotwiring his
//! car, reading the tracks, knocking as the police, picking the back lock, packing the insulin in
//! cold water, talking the lad out of the rifle). Either way done, the chain goes on to the runner
//! and his case (RZ_OpeningCase). Every step is on the errand engine (RZ_OpeningStepDef), handed to
//! you; the thing named stands at the place (the errands' props and the work sites' looks); doors are
//! real doors. Server only.

//------------------------------------------------------------------------------------------------
//! The contact's problem (the character flag opening.favour, for the conversations).
enum RZ_EFavour
{
	NONE,
	FEVER,		// Nurse, Doctor: his son's fever; the boy's medicine from the chemist's
	CAR,		// Mechanic: his car won't start; a battery from the garage
	HENS,		// Hunter: something's at his hens; his rifle's last box of shells from his shed
	NEIGHBOUR,	// Police Officer, Ex-Con: his neighbour won't answer; the spare key under the mat
	INSULIN,	// Lab Technician: his insulin's gone warm; the cool box from the school's lab
	BROTHER,	// Radio Operator: he can't raise his brother; a battery for his set from the shop
	RIFLE,		// Soldier: a lad's taken his rifle; the rifle, from the lad's house
	TIN,		// Raider: he owes the crews a tin; a tin from the house next door
	WELL,		// Elder: two families want the same well; the well rota from the church board
	SACK		// Unemployed, Drifter: his last sack of potatoes, from the shop's back room
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningFavours
{
	static const string FLAG_FAVOUR = "opening.favour";	// RZ_EFavour, while the favour runs
	static const string KEY = "opening.favour";

	protected static const float FAR_MIN = 90;		// a fetch place this far from the contact at least
	protected static const float FAR_MAX = 450;		// and at most
	protected static const float NEAR_MIN = 35;		// the house next door, the neighbour's, the lad's
	protected static const float NEAR_MAX = 300;
	protected static const int TRIES = 10;
	protected static const float DOOR_STEP = 1.2;	// the doorstep, out from the doorway

	protected ref map<int, ref RZ_OpeningStepDef> m_mFetch = new map<int, ref RZ_OpeningStepDef>();
	protected ref map<int, ref RZ_OpeningStepDef> m_mGive = new map<int, ref RZ_OpeningStepDef>();
	protected ref map<string, ref RZ_OpeningStepDef> m_mWay = new map<string, ref RZ_OpeningStepDef>();	// by background

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningFavours(RZ_OpeningChain chain)
	{
		RZ_ErrandWay w;
		// Nurse, Doctor
		w = Fetch(RZ_EFavour.FEVER, "The Medicine", "Find the boy's medicine at {place}.", RZ_EWork.SEARCH, "Search the shelves for it", 10, RZ_EErrandProp.NONE);
		w.Take("My son's burning up. Since last night. There's {place}: whatever they had for a fever, it'll be on a shelf. Fetch it, would you?",
			"The boy's got a fever and I've got nothing. {place} might still have something on the shelves. I'll sit with him.");
		Give(RZ_EFavour.FEVER, "Give Him the Medicine", "Give {giver} the boy's medicine.");

		// Mechanic
		w = Fetch(RZ_EFavour.CAR, "The Battery", "Find a battery that holds a charge at {place}.", RZ_EWork.SEARCH, "Find a battery", 10, RZ_EErrandProp.PARTS);
		w.Take("It won't turn over. Not a click. It's the battery, it has to be. {place} will have one that still holds a charge.",
			"I've been trying to start it since the roads shut, as if there's anywhere to go. Humour me. {place}. A battery.");
		Give(RZ_EFavour.CAR, "Give Him the Battery", "Give {giver} the battery.");

		// Hunter
		w = Fetch(RZ_EFavour.HENS, "The Last Box", "Find the last box of shells at {place}.", RZ_EWork.SEARCH, "Find the box of shells", 8, RZ_EErrandProp.CRATE);
		w.Take("Something's been at my hens. Three gone this week. My last box of shells is in {place}. I'm not walking over there in the dark.",
			"Fox, or worse. I've got a gun and nothing to put in it. The shells are in {place}.");
		Give(RZ_EFavour.HENS, "Give Him the Shells", "Give {giver} the box of shells.");

		// Police Officer, Ex-Con
		w = Fetch(RZ_EFavour.NEIGHBOUR, "The Spare Key", "Find the spare key under the mat at {place}.", RZ_EWork.HANDS, "Look under the mat", 3, RZ_EErrandProp.NONE);
		w.Take("My neighbour's not answered since yesterday. {place}. He keeps a spare key under the mat. I can't make myself look.",
			"He always answers. Always. {place}, and the key's under the mat. I'd go, but I know what I'd find.");
		Give(RZ_EFavour.NEIGHBOUR, "Give Him the Key", "Give {giver} the spare key.");

		// Lab Technician
		w = Fetch(RZ_EFavour.INSULIN, "The Cool Box", "Find the cool box at {place}.", RZ_EWork.SEARCH, "Find the cool box", 10, RZ_EErrandProp.CRATE);
		w.Take("My insulin's gone warm. Two days without a fridge. {place} had a cool box in the lab. Ice packs, if we're lucky.",
			"I need my insulin cold, and there's no cold left anywhere. The lab at {place} kept a cool box.");
		Give(RZ_EFavour.INSULIN, "Give Him the Cool Box", "Give {giver} the cool box.");

		// Radio Operator
		w = Fetch(RZ_EFavour.BROTHER, "A Battery for the Set", "Find a battery for his set at {place}.", RZ_EWork.SEARCH, "Find a battery", 8, RZ_EErrandProp.PARTS);
		w.Take("I can't raise my brother. The set's fine, it's the battery. {place} might have one. He's two towns over, and he worries.",
			"My brother calls every night at nine. He hasn't. My set's dead. {place} sold batteries, once.");
		Give(RZ_EFavour.BROTHER, "Give Him the Battery", "Give {giver} the battery for the set.");

		// Soldier: the lad is there (a work site's person, standing)
		w = Fetch(RZ_EFavour.RIFLE, "The Rifle", "Take the rifle back from the lad at {place}.", RZ_EWork.HANDS, "Take the rifle off him", 10, RZ_EErrandProp.NONE);
		w.m_bPerson = true;
		w.Take("A lad from the end house took my rifle. Says he's going to the line to shoot his way out. He's at {place}. Get it back before he tries.",
			"The boy's got my rifle and a head full of nonsense. {place}. Get it off him. Gently, if you can.");
		Give(RZ_EFavour.RIFLE, "Give Him the Rifle", "Give {giver} the rifle.");

		// Raider
		w = Fetch(RZ_EFavour.TIN, "A Tin", "Find a tin of anything at {place}.", RZ_EWork.SEARCH, "Find a tin", 8, RZ_EErrandProp.CRATE);
		w.Take("I owe the crews a tin. One tin. They come round at dusk. {place}'s empty: they left in a hurry. There'll be something in a cupboard.",
			"One tin, or they take the door off. {place} is empty now. Anything in a tin.");
		Give(RZ_EFavour.TIN, "Give Him the Tin", "Give {giver} the tin.");

		// Elder
		w = Fetch(RZ_EFavour.WELL, "The Rota", "Take the well rota off the board at {place}.", RZ_EWork.HANDS, "Take the rota off the board", 3, RZ_EErrandProp.PAPERS);
		w.Take("Two families, one well, and they're about to come to blows. The rota's pinned on the board at {place}. They'll listen to paper.",
			"The Kerns say it's their morning at the well. The Orlovs say it's theirs. The rota's at {place}. Fetch it and we'll see.");
		Give(RZ_EFavour.WELL, "Give Him the Rota", "Give {giver} the well rota.");

		// Unemployed, Drifter
		w = Fetch(RZ_EFavour.SACK, "The Last Sack", "Find his sack of potatoes in the back room at {place}.", RZ_EWork.SEARCH, "Find the sack", 8, RZ_EErrandProp.CRATE);
		w.Take("My last sack of potatoes is in the back room at {place}. I hid it there on the second day. Clever, I thought. Then I couldn't face the walk.",
			"I hid food in the back of {place} when it started. Potatoes. A whole sack. Fetch it and we'll both eat tonight.");
		Give(RZ_EFavour.SACK, "Give Him the Sack", "Give {giver} the sack of potatoes.");

		// The background's second ways that are steps of their own
		RZ_OpeningStepDef fever = chain.Def(RZ_EOpeningTask.FEVER);
		if (fever)
		{
			m_mWay.Set("nurse", fever);
			m_mWay.Set("doctor", fever);
		}

		w = WayStep("mechanic", "Hotwire It", "Hotwire his car at {place}.", RZ_EWork.HANDS, "Hotwire it", 12, RZ_EErrandProp.CAR);
		w.Done("It's running. Listen to that. Where did you learn that? No. Don't tell me.",
			"Running, and no battery. I'm not going to ask.");
		w = WayStep("hunter", "The Tracks", "Read the tracks by his hens at {place}.", RZ_EWork.HANDS, "Read the tracks", 8, RZ_EErrandProp.NONE);
		w.m_bQuiet = true;
		w.Done("A man? Boots? Then it's not a fox that wants my hens. It's a neighbour. That's worse.",
			"Size nine, and hungry. I know whose boots those are. I'll have a word.");
		w = WayStep("police", "Open Up", "Knock at {place} as the police.", RZ_EWork.HANDS, "Knock: it's the police", 4, RZ_EErrandProp.NONE);
		w.Done("He opened the door. He actually opened the door. Asleep in his chair, he says. Asleep for a day.",
			"He heard 'police' and came running. Habit. Thank you.");
		w = WayStep("excon", "The Back Door", "Work the back lock at {place}, quietly.", RZ_EWork.HANDS, "Work the lock, quietly", 10, RZ_EErrandProp.NONE);
		w.m_bQuiet = true;
		w.m_bUnseen = true;
		w.Done("He's in there. Alive, asleep in his chair. I won't ask where you learned that.",
			"Open. Quiet as you like. Remind me to put a better lock on mine.");
		w = WayStep("labtech", "Kept Cold", "Pack his insulin in cold water at {place}.", RZ_EWork.FILL, "Pack it in cold water", 8, RZ_EErrandProp.NONE);
		w.Done("Cold water and a wet cloth. That's all it needed? You've saved me a walk and a week.",
			"It's cold to the touch. I could kiss you. I won't.");
		w = WayStep("soldier", "Give It Here", "Talk the lad at {place} out of the rifle.", RZ_EWork.HANDS, "Give it here, son", 3, RZ_EErrandProp.NONE);
		w.m_bPerson = true;
		w.Done("He just handed it over? To you? I've been shouting at him for an hour.",
			"Back on its nail. And the lad's gone home. You've a way with them.");
	}

	//------------------------------------------------------------------------------------------------
	//! A favour's fetch step: one named thing at its place, a hold there (`work`, `label`, `seconds`),
	//! with `prop` standing at the place when the work's own look doesn't name it. Done there.
	protected RZ_ErrandWay Fetch(int favour, string title, string step, RZ_EWork work, string label, float seconds, RZ_EErrandProp prop)
	{
		RZ_OpeningStepDef d = new RZ_OpeningStepDef(KEY + "." + favour.ToString(), title, 1);
		d.m_fLapseMinutes = 45;
		m_mFetch.Set(favour, d);
		RZ_ErrandWay w = d.Way("do");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = seconds;
		w.m_fRadius = 10;
		w.m_bEndsThere = true;
		w.m_sStep = step;
		w.m_eProp = prop;
		w.Work(work, label, seconds);
		return w;
	}

	//------------------------------------------------------------------------------------------------
	//! A favour's hand-over: done only by its reply on his talk screen (RZ_Openings.HandOver), where
	//! he is: he walks with you, so it's never a walk back.
	protected void Give(int favour, string title, string step)
	{
		RZ_OpeningStepDef d = new RZ_OpeningStepDef(KEY + ".give." + favour.ToString(), title, 2);
		d.m_fLapseMinutes = 30;
		m_mGive.Set(favour, d);
		RZ_ErrandWay w = d.Way("give");
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 1;
		w.m_fRadius = 8;
		w.m_bEndsThere = true;
		w.m_sStep = step;
	}

	//------------------------------------------------------------------------------------------------
	//! A background's second way that is a step of its own: a hold at its place.
	protected RZ_ErrandWay WayStep(string bg, string title, string step, RZ_EWork work, string label, float seconds, RZ_EErrandProp prop)
	{
		RZ_OpeningStepDef d = new RZ_OpeningStepDef(KEY + ".way." + bg, title, 1);
		d.m_fLapseMinutes = 30;
		m_mWay.Set(bg, d);
		RZ_ErrandWay w = d.Way("do");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = seconds;
		w.m_fRadius = 10;
		w.m_bEndsThere = true;
		w.m_sStep = step;
		w.m_eProp = prop;
		w.Work(work, label, seconds);
		return w;
	}

	//------------------------------------------------------------------------------------------------
	RZ_OpeningStepDef FetchDef(int favour)
	{
		RZ_OpeningStepDef d;
		m_mFetch.Find(favour, d);
		return d;
	}

	//------------------------------------------------------------------------------------------------
	RZ_OpeningStepDef GiveDef(int favour)
	{
		RZ_OpeningStepDef d;
		m_mGive.Find(favour, d);
		return d;
	}

	//------------------------------------------------------------------------------------------------
	//! The background's way as a step of its own; null for a reply way (or none).
	RZ_OpeningStepDef WayDef(string bg)
	{
		RZ_OpeningStepDef d;
		m_mWay.Find(bg, d);
		return d;
	}

	// ================================================================ Who gets which
	//------------------------------------------------------------------------------------------------
	static int For(string bg)
	{
		switch (bg)
		{
			case "nurse": return RZ_EFavour.FEVER;
			case "doctor": return RZ_EFavour.FEVER;
			case "mechanic": return RZ_EFavour.CAR;
			case "hunter": return RZ_EFavour.HENS;
			case "police": return RZ_EFavour.NEIGHBOUR;
			case "excon": return RZ_EFavour.NEIGHBOUR;
			case "labtech": return RZ_EFavour.INSULIN;
			case "radioham": return RZ_EFavour.BROTHER;
			case "soldier": return RZ_EFavour.RIFLE;
			case "raider": return RZ_EFavour.TIN;
			case "elder": return RZ_EFavour.WELL;
		}

		return RZ_EFavour.SACK;
	}

	//------------------------------------------------------------------------------------------------
	//! A background's way is a reply that settles it there and then (no step of its own).
	static bool WayIsReply(string bg)
	{
		return bg == "radioham" || bg == "raider" || bg == "elder";
	}

	//------------------------------------------------------------------------------------------------
	//! This background has a second way through its favour (the Unemployed and the Drifter haven't).
	static bool HasWay(string bg)
	{
		return bg != "unemployed" && bg != "drifter" && !bg.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Where the background's way is done (a step of its own): his son and his insulin by him, the car
	//! and the hens a short way off, the neighbour's door and the lad's house where the fetch was.
	static vector WayPlace(string bg, vector giverAt, vector fetchAt)
	{
		switch (bg)
		{
			case "mechanic": return RZ_WorkSites.Spot(giverAt, 6, 15);
			case "hunter": return RZ_WorkSites.Spot(giverAt, 15, 40);
			case "police": return fetchAt;
			case "excon": return fetchAt;
			case "soldier": return fetchAt;
		}

		return giverAt;
	}

	// ================================================================ The place
	//------------------------------------------------------------------------------------------------
	//! Where the favour's thing is: a building of the right kind (its words true of it), the spot on
	//! its ground floor (the neighbour's: the doorstep of its real outside door). False: none here.
	static bool PickPlace(int favour, vector from, out vector place, out string text)
	{
		place = vector.Zero;
		text = "";
		array<string> kinds = {};
		Kinds(favour, kinds);
		float lo = FAR_MIN;
		float hi = FAR_MAX;
		if (favour == RZ_EFavour.NEIGHBOUR || favour == RZ_EFavour.RIFLE || favour == RZ_EFavour.TIN)
		{
			lo = NEAR_MIN;
			hi = NEAR_MAX;
		}

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(from, hi, buildings, 400);
		array<IEntity> mine = {};
		array<IEntity> houses = {};
		foreach (IEntity b : buildings)
		{
			if (!b)
				continue;

			float d = RZ_Util.DistXZ(from, b.GetOrigin());
			if (d < lo || d > hi || RZ_LockedSpots.IsLockedBuilding(b))
				continue;

			if (!kinds.IsEmpty() && !RZ_OpeningRoll.KindOf(b, kinds).IsEmpty())
				mine.Insert(b);
			else if (RZ_PlaceCheck.IsHouse(b))
				houses.Insert(b);
		}

		string kind;
		IEntity found = Try(favour, mine, kinds, from, place, kind);
		if (!found && HouseWillDo(favour))
		{
			array<string> none = {};
			found = Try(favour, houses, none, from, place, kind);
			kind = "";
		}

		if (!found)
			return false;

		text = Words(favour, kind);
		string need = "opening.favour";
		if (favour == RZ_EFavour.NEIGHBOUR)
			need = "opening.favour.door";

		string building = kind;
		if (building.IsEmpty())
			building = "house";

		RZ_PlaceNeeds.Verify(need, place, text, -1, building);
		RZ_Log.Info(string.Format("Openings: the favour (%1) is at %2 (%3, %4), %5 m from the contact", typename.EnumToString(RZ_EFavour, favour), text, RZ_ItemHelper.LowerName(found), building, RZ_Util.RoundToInt(RZ_Util.DistXZ(from, place))));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Up to TRIES of `list` at random: the first whose side meets the need and has a floor inside (the
	//! neighbour's: a real outside door, its doorstep).
	protected static IEntity Try(int favour, notnull array<IEntity> list, notnull array<string> kinds, vector from, out vector place, out string kind)
	{
		string key = "opening.favour";
		if (favour == RZ_EFavour.NEIGHBOUR)
			key = "opening.favour.door";

		RZ_PlaceNeed need = RZ_PlaceNeeds.Get(key);
		for (int i = 0; i < TRIES && !list.IsEmpty(); i++)
		{
			int pick = Math.RandomInt(0, list.Count());
			IEntity b = list[pick];
			list.Remove(pick);
			kind = RZ_OpeningRoll.KindOf(b, kinds);
			string building = kind;
			if (building.IsEmpty())
				building = "house";

			vector side = RZ_Util.GroundPos(RZ_DoorGeometry.SidePoint(b, from, 4));
			string why;
			if (!RZ_PlaceCheck.Check(side, need, -1, building, why))
				continue;

			if (favour == RZ_EFavour.NEIGHBOUR)
			{
				IEntity door;
				vector centre;
				vector normal;
				if (!RZ_DoorGeometry.NearestOuterDoor(b.GetOrigin(), 0, 16, true, door, centre, normal))
					continue;

				place = RZ_Util.GroundPos(centre + normal * DOOR_STEP);
				return b;
			}

			vector inside;
			if (RZ_WorkSites.FloorOf(b, inside))
			{
				place = inside;
				return b;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The kinds of building a favour's thing is in, first choice first ("house" alone: any house).
	protected static void Kinds(int favour, notnull array<string> kinds)
	{
		switch (favour)
		{
			case RZ_EFavour.FEVER: kinds.Insert("clinic"); kinds.Insert("aid"); return;
			case RZ_EFavour.CAR: kinds.Insert("garage"); kinds.Insert("fuel"); return;
			case RZ_EFavour.HENS: kinds.Insert("farm"); return;
			case RZ_EFavour.INSULIN: kinds.Insert("school"); kinds.Insert("clinic"); kinds.Insert("aid"); return;
			case RZ_EFavour.BROTHER: kinds.Insert("shop"); return;
			case RZ_EFavour.WELL: kinds.Insert("church"); kinds.Insert("public"); return;
			case RZ_EFavour.SACK: kinds.Insert("shop"); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A plain house is a true place for this favour's words (the doctor's house, his own shed's house,
	//! the neighbour's, the lad's, the house next door).
	protected static bool HouseWillDo(int favour)
	{
		return favour == RZ_EFavour.FEVER || favour == RZ_EFavour.HENS || favour == RZ_EFavour.NEIGHBOUR || favour == RZ_EFavour.RIFLE || favour == RZ_EFavour.TIN;
	}

	//------------------------------------------------------------------------------------------------
	//! What the place is called, true of the building (docs/PLACES.md, "A place is what its words say").
	static string Words(int favour, string kind)
	{
		switch (favour)
		{
			case RZ_EFavour.FEVER:
			{
				if (kind == "clinic")
					return "the chemist's";
				if (kind == "aid")
					return "the old aid post";
				return "the doctor's house";
			}
			case RZ_EFavour.CAR:
			{
				if (kind == "fuel")
					return "the petrol station";
				return "the garage";
			}
			case RZ_EFavour.HENS:
			{
				if (kind == "farm")
					return "his farm's shed";
				return "his old house up the lane";
			}
			case RZ_EFavour.NEIGHBOUR: return "the house over the way";
			case RZ_EFavour.INSULIN:
			{
				if (kind == "school")
					return "the school";
				if (kind == "aid")
					return "the old aid post";
				return "the chemist's";
			}
			case RZ_EFavour.BROTHER: return "the shop";
			case RZ_EFavour.RIFLE: return "the lad's house";
			case RZ_EFavour.TIN: return "the house next door";
			case RZ_EFavour.WELL:
			{
				if (kind == "church")
					return "the church";
				return "the hall";
			}
		}

		return "the old shop";
	}

	// ================================================================ The words
	//------------------------------------------------------------------------------------------------
	//! Beat 1: the contact asks who you were, in passing.
	static string Question(string bg)
	{
		switch (bg)
		{
			case "nurse": return "You're steady with that bag. Still a nurse, under all that?";
			case "doctor": return "You've got the doctor's look. Like you're counting something in me.";
			case "mechanic": return "Those hands have seen engines. Don't tell me they haven't.";
			case "hunter": return "You walk like you're listening. Still hunting, are you?";
			case "police": return "You've still got the walk. You can take off the uniform, but not the walk.";
			case "excon": return "You've done time, haven't you? It's all right. Everyone's doing time now.";
			case "labtech": return "You were at the institute. You've still got the gloves in your pocket.";
			case "radioham": return "You're the one with the aerial on the roof. Half the street heard you at night.";
			case "soldier": return "You stand like a soldier. Even now.";
			case "raider": return "I know your face. Most people on this street do. Most of them wish they didn't.";
			case "elder": return "People listen when you talk. They always did. I'm counting on it.";
			case "drifter": return "You're not from here. Doesn't matter. Nobody's from anywhere now.";
		}

		return "Still out of work? Me too. Everyone is now. We're all equals at last.";
	}

	//------------------------------------------------------------------------------------------------
	//! Your answer that picks a habit (the reply's label).
	static string HabitLabel(int habit)
	{
		switch (habit)
		{
			case RZ_EPlayerHabit.STEADY: return "I keep my hands steady. That's all I've got.";
			case RZ_EPlayerHabit.LIGHT: return "I keep my head down and my feet quiet.";
		}

		return "I tell people what they need to hear.";
	}

	//------------------------------------------------------------------------------------------------
	//! What he says back.
	static string HabitAnswer(int habit)
	{
		switch (habit)
		{
			case RZ_EPlayerHabit.STEADY: return "Good. Steady's worth more than clever, these days.";
			case RZ_EPlayerHabit.LIGHT: return "Sensible. The ones who don't are the ones we hear about.";
		}

		return "Then don't tell me anything. I'd believe it.";
	}

	//------------------------------------------------------------------------------------------------
	//! The background's way, as the reply's label.
	static string WayLabel(string bg)
	{
		switch (bg)
		{
			case "nurse": return "That's not a fever, that's an infection. Hold him still.";
			case "doctor": return "That's not a fever. It's the water he's been drinking. Hold him still.";
			case "mechanic": return "Forget the battery. Show me the car.";
			case "hunter": return "Before anyone fetches shells, let me see the tracks.";
			case "police": return "Leave the key. Knocking is my job.";
			case "excon": return "Back doors are never as good as front ones.";
			case "labtech": return "Insulin doesn't need a fridge. It needs cold. Get me water.";
			case "radioham": return "Never mind your set. I'll raise him on mine.";
			case "soldier": return "Leave the lad to me.";
			case "raider": return "I know who collects on this street. It's forgotten.";
			case "elder": return "Fetch both of them out here. I'll settle it in the street.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! What the way needs that you haven't got ("" for nothing).
	static string WayNeed(string bg, IEntity user)
	{
		if (bg == "nurse" || bg == "doctor")
		{
			if (!user || RZ_Errands.CountItems(user, RZ_EErrandItem.MEDICAL) <= 0)
				return "Something medical in your bag";
		}

		if (bg == "radioham" && !RZ_Radio.CarriesRadio(user))
			return "A radio on you";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A reply way: what's said, there and then.
	static void WayLines(string bg, notnull array<string> lines)
	{
		switch (bg)
		{
			case "radioham":
			{
				lines.Insert("Static, static, then a voice. Yes. Yes, he's alive. He says the bridge is out and to stop worrying.");
				lines.Insert("He never stopped worrying in his life. Thank you. I'll sleep tonight.");
				return;
			}
			case "raider":
			{
				lines.Insert("Forgotten? Just like that? I won't ask what he owes you.");
				lines.Insert("Keep your voice down when you say things like that. Somebody might believe you.");
				return;
			}
			case "elder":
			{
				lines.Insert("Mornings to the Kerns, evenings to the Orlovs, and the bucket stays at the well.");
				lines.Insert("They shook on it. They didn't smile, but they shook. You made it look easy.");
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! His thanks when the thing is handed over.
	static string Thanks(int favour)
	{
		switch (favour)
		{
			case RZ_EFavour.FEVER: return "Two now, two at bedtime, it says. I'll read it to him. Thank you. I mean it.";
			case RZ_EFavour.CAR: return "It'll turn over now. Not that there's anywhere to drive to. It's the principle.";
			case RZ_EFavour.HENS: return "Shells. Now whatever it is had better come back tonight.";
			case RZ_EFavour.NEIGHBOUR: return "I'll go round now. Whatever's in there, I'd rather know.";
			case RZ_EFavour.INSULIN: return "Cold. Properly cold. You've bought me a month. Maybe two.";
			case RZ_EFavour.BROTHER: return "It's on. It's on, and that's his voice. He's alive. He's complaining. That's him.";
			case RZ_EFavour.RIFLE: return "Back on its nail. And the lad's still breathing. Good work, both of you.";
			case RZ_EFavour.TIN: return "Peaches. They'll take peaches. Thank you. That's one more week of having a door.";
			case RZ_EFavour.WELL: return "Mornings to the Kerns. It's in writing. They can argue with the paper now.";
		}

		return "A whole sack. We eat tonight. Both of us, sit down.";
	}

	//------------------------------------------------------------------------------------------------
	//! The hand-over reply's label.
	static string GiveLabel(int favour)
	{
		switch (favour)
		{
			case RZ_EFavour.FEVER: return "Here's the boy's medicine.";
			case RZ_EFavour.CAR: return "Here's your battery.";
			case RZ_EFavour.HENS: return "Here are your shells.";
			case RZ_EFavour.NEIGHBOUR: return "Here's the key.";
			case RZ_EFavour.INSULIN: return "Here's the cool box.";
			case RZ_EFavour.BROTHER: return "Here's a battery for your set.";
			case RZ_EFavour.RIFLE: return "Here's your rifle.";
			case RZ_EFavour.TIN: return "Here's a tin.";
			case RZ_EFavour.WELL: return "Here's the rota.";
		}

		return "Here's your sack.";
	}

	// ================================================================ On the way
	//------------------------------------------------------------------------------------------------
	//! What the walking contact says, once each, at a thing worth knowing.
	static string Remark(int what)
	{
		switch (what)
		{
			case 0: return "See the chalk on that door? A cross means the dead are inside. A circle, someone's still home.";
			case 1: return "There. See it? They hear you before they see you. Keep low.";
			case 2: return "That one's locked. Somebody locked it from inside, or somebody's coming back. Either way, not us.";
		}

		return "Hear that? The broadcast. Aid is on its way. Eleven days it's been saying that.";
	}

	//------------------------------------------------------------------------------------------------
	//! The background's way done as a step of its own: the neighbour's door opens (the police knock,
	//! the back lock). A real door, never one of ours.
	static void WayDone(string bg, vector at)
	{
		if (bg != "police" && bg != "excon")
			return;

		IEntity door;
		vector centre;
		vector normal;
		if (RZ_DoorGeometry.NearestOuterDoor(at, 0, 5, false, door, centre, normal) && door)
		{
			RZ_OpeningRoll.OpenDoor(door);
			RZ_Log.Info(string.Format("Openings: the neighbour's door at %1 opened (%2)", RZ_SettlementScanner.GridReference(at), bg));
		}
	}
}
