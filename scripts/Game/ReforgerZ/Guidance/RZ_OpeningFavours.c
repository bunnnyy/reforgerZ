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
		w.Take("My son has had a fever since last night, and it's getting worse. I've no medicine in the house. {place} might still have something for a fever on a shelf. Fetch it and bring it back to me, and I'll get it into him tonight.",
			"My boy's burning up with a fever and I've nothing to give him. Look on the shelves at {place} for fever medicine and bring it here. I'll stay and sit with him till you're back.");
		Give(RZ_EFavour.FEVER, "Give Him the Medicine", "Give {giver} the boy's medicine.");

		// Mechanic
		w = Fetch(RZ_EFavour.CAR, "The Battery", "Find a battery that holds a charge at {place}.", RZ_EWork.SEARCH, "Find a battery", 10, RZ_EErrandProp.PARTS);
		w.Take("My car won't start. Not even a click, so it's the battery. If the infected come down this street, I want to drive away, not run. {place} should have a battery that still holds a charge. Bring one back here and I'll fit it.",
			"My car's battery is dead, and that car is my only way off this street if things go bad. There'll be a battery at {place}. Fetch one that still holds a charge and bring it to me.");
		Give(RZ_EFavour.CAR, "Give Him the Battery", "Give {giver} the battery.");

		// Hunter
		w = Fetch(RZ_EFavour.HENS, "The Last Box", "Find the last box of shells at {place}.", RZ_EWork.SEARCH, "Find the box of shells", 8, RZ_EErrandProp.CRATE);
		w.Take("Something has been taking my hens. Three gone this week, and they're all the food I have. I've a gun but no shells: my last box is in {place}. Bring it to me and I'll sit up tonight and deal with it.",
			"A fox, or worse, is eating my hens, and they're what feeds me. My gun's empty and my last box of shells is in {place}. I won't walk there in the dark. Fetch it here for me?");
		Give(RZ_EFavour.HENS, "Give Him the Shells", "Give {giver} the box of shells.");

		// Police Officer, Ex-Con
		w = Fetch(RZ_EFavour.NEIGHBOUR, "The Spare Key", "Find the spare key under the mat at {place}.", RZ_EWork.HANDS, "Look under the mat", 3, RZ_EErrandProp.NONE);
		w.Take("My neighbour in {place} hasn't answered his door since yesterday, and he always answers. He keeps a spare key under the mat. I can't face going in. Fetch me the key, so at least we can find out if he's alive.",
			"The man in {place} has always answered when I knock. Since yesterday, nothing. He could be ill, or worse. His spare key is under the mat. Bring it to me. I'd go, but I'm scared of what I'd find.");
		Give(RZ_EFavour.NEIGHBOUR, "Give Him the Key", "Give {giver} the spare key.");

		// Lab Technician
		w = Fetch(RZ_EFavour.INSULIN, "The Cool Box", "Find the cool box at {place}.", RZ_EWork.SEARCH, "Find the cool box", 10, RZ_EErrandProp.CRATE);
		w.Take("I'm diabetic, and my insulin stops working if it isn't kept cold. Two days without a fridge and it's going warm. There was a cool box with ice packs at {place}. Bring it to me and the insulin will last.",
			"My insulin has to stay cold, and there isn't a working fridge left on this street. Without it I'm in real trouble within weeks. {place} kept a cool box. Fetch it back here for me.");
		Give(RZ_EFavour.INSULIN, "Give Him the Cool Box", "Give {giver} the cool box.");

		// Radio Operator
		w = Fetch(RZ_EFavour.BROTHER, "A Battery for the Set", "Find a battery for {giver}'s radio at {place}.", RZ_EWork.SEARCH, "Find a battery", 8, RZ_EErrandProp.PARTS);
		w.Take("I can't reach my brother. He lives two towns over and we talk on the radio every night at nine, but my set's battery is dead. {place} might have one. Bring me a battery so I can find out if he's alive.",
			"My radio's battery is flat, and the radio is the only way I can talk to my brother two towns over. He hasn't heard from me in days. Look for a battery at {place} and bring it here.");
		Give(RZ_EFavour.BROTHER, "Give Him the Battery", "Give {giver} the battery for the set.");

		// Soldier: the lad is there (a work site's person, standing)
		w = Fetch(RZ_EFavour.RIFLE, "The Rifle", "Take the rifle back from the lad at {place}.", RZ_EWork.HANDS, "Take the rifle off the lad", 10, RZ_EErrandProp.NONE);
		w.m_bPerson = true;
		w.Take("A lad from down the road took my rifle. He says he'll walk to the Perimeter and shoot his way out, and the snipers there will kill him long before he's close. He's at {place}. Get the rifle off him, gently if you can, and bring it back to me.",
			"The boy's taken my rifle and means to fight his way through the line. That's how you die in here. He's at {place}. Take the rifle off him and bring it back, and he'll live to be angry with me.");
		Give(RZ_EFavour.RIFLE, "Give Him the Rifle", "Give {giver} the rifle.");

		// Raider
		w = Fetch(RZ_EFavour.TIN, "A Tin", "Find a tin of anything at {place}.", RZ_EWork.SEARCH, "Find a tin", 8, RZ_EErrandProp.CRATE);
		w.Take("I owe the Red Hand crews one tin of food. They collect at dusk, and if I can't pay they take my front door, and then the infected walk in. {place} is empty: the family left in a hurry. Find a tin of anything in a cupboard and bring it to me.",
			"One tin of food, or the Red Hand take my front door off tonight. That's their price for leaving me be. The family in {place} left in a hurry. Look in their cupboards and bring me a tin.");
		Give(RZ_EFavour.TIN, "Give Him the Tin", "Give {giver} the tin.");

		// Elder
		w = Fetch(RZ_EFavour.WELL, "The Rota", "Take the well rota off the board at {place}.", RZ_EWork.HANDS, "Take the rota off the board", 3, RZ_EErrandProp.PAPERS);
		w.Take("Two families on this street, the Kerns and the Orlovs, both say tomorrow morning at the well is theirs, and they're close to a fight. The written rota is pinned on the board at {place}. Bring it here and the paper can settle whose morning it is.",
			"The Kerns and the Orlovs share one well, and each says the morning turn is theirs. It'll come to blows soon. The rota is pinned on the board at {place}. Fetch it to me and we'll see what it says.");
		Give(RZ_EFavour.WELL, "Give Him the Rota", "Give {giver} the well rota.");

		// Unemployed, Drifter
		w = Fetch(RZ_EFavour.SACK, "The Last Sack", "Find {giver}'s sack of potatoes in the back room at {place}.", RZ_EWork.SEARCH, "Find the sack", 8, RZ_EErrandProp.CRATE);
		w.Take("I hid my last sack of potatoes in the back room at {place} on the second day. It's all the food I've got left, and I couldn't face the walk there on my own. Fetch it back here and we'll both eat tonight.",
			"When it started I hid a whole sack of potatoes in the back of {place}. It's the last food I have. Bring it back to me and you'll eat tonight too.");
		Give(RZ_EFavour.SACK, "Give Him the Sack", "Give {giver} the sack of potatoes.");

		// The background's second ways that are steps of their own
		RZ_OpeningStepDef fever = chain.Def(RZ_EOpeningTask.FEVER);
		if (fever)
		{
			m_mWay.Set("nurse", fever);
			m_mWay.Set("doctor", fever);
		}

		w = WayStep("mechanic", "Hotwire It", "Hotwire {giver}'s car at {place}.", RZ_EWork.HANDS, "Hotwire the car", 12, RZ_EErrandProp.CAR);
		w.Done("The car's running, and without a new battery. If I have to get away now, I can. Where did you learn that? No. Don't tell me.",
			"It's running with the old battery still in it. That's my way off this street sorted. I'm not going to ask how.");
		w = WayStep("hunter", "The Tracks", "Read the tracks by {giver}'s hens at {place}.", RZ_EWork.HANDS, "Read the tracks", 8, RZ_EErrandProp.NONE);
		w.m_bQuiet = true;
		w.Done("Boot prints, not paws? Then it's a man taking my hens, not a fox. A hungry neighbour. That's worse, but I don't need shells for it.",
			"Size nine boots, and hungry. I know whose boots those are. I'll have a word with him, and my hens stay mine.");
		w = WayStep("police", "Open Up", "Knock at {place} as the police.", RZ_EWork.HANDS, "Knock: it's the police", 4, RZ_EErrandProp.NONE);
		w.Done("He opened the door for you. He's alive. Asleep in his chair, he says. Asleep for a whole day. Thank you.",
			"He heard 'police' and came to the door. Habit. He's alive, and that's all I wanted to know.");
		w = WayStep("excon", "The Back Door", "Work the back lock at {place}, quietly.", RZ_EWork.HANDS, "Work the lock, quietly", 10, RZ_EErrandProp.NONE);
		w.m_bQuiet = true;
		w.m_bUnseen = true;
		w.Done("You got in. He's alive, asleep in his chair. I was sure he was dead. I won't ask where you learned that.",
			"The back door's open, quiet as you like, and he's alive in there. Remind me to put a better lock on mine.");
		w = WayStep("labtech", "Kept Cold", "Pack {giver}'s insulin in cold water at {place}.", RZ_EWork.FILL, "Pack the insulin in cold water", 8, RZ_EErrandProp.NONE);
		w.Done("Cold water and a wet cloth round it, and it keeps? Then my insulin lasts, and nobody has to walk anywhere for a cool box.",
			"It's cold to the touch again. My insulin will keep. I could kiss you. I won't.");
		w = WayStep("soldier", "Give It Here", "Talk the lad at {place} out of the rifle.", RZ_EWork.HANDS, "Ask the lad for the rifle", 3, RZ_EErrandProp.NONE);
		w.m_bPerson = true;
		w.Done("He handed the rifle over? To you? I've been shouting at him for an hour. He'll live now.",
			"The rifle's back on its nail, and the lad's gone home instead of to the line. You've a way with them.");
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

	//------------------------------------------------------------------------------------------------
	//! A favour's step by its key (a saved held errand, RZ_Errands.FindDef); null if not a favour's.
	RZ_ErrandDef Find(string key)
	{
		foreach (int fetch, RZ_OpeningStepDef f : m_mFetch)
		{
			if (f && f.m_sKey == key)
				return f;
		}

		foreach (int give, RZ_OpeningStepDef g : m_mGive)
		{
			if (g && g.m_sKey == key)
				return g;
		}

		foreach (string bg, RZ_OpeningStepDef w : m_mWay)
		{
			if (w && w.m_sKey == key)
				return w;
		}

		return null;
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
					return "the shed on the farm";
				return "the old house up the lane";
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
			case "nurse": return "You were a nurse at the clinic before all this. Tell me, how have you kept yourself alive in there since the roads shut?";
			case "doctor": return "You were a doctor before the seal. You still look at people like you're checking them over. How have you kept alive since the roads shut?";
			case "mechanic": return "You used to fix engines at the garage. I'd know those hands anywhere. How have you kept yourself alive since the roads shut?";
			case "hunter": return "You used to hunt the woods round here. You still walk like you're listening for something. How have you stayed alive since the roads shut?";
			case "police": return "You were a police officer before the seal. You still walk like one. How have you kept yourself alive since the roads shut?";
			case "excon": return "You did time before all this. It's all right: we're all locked in now. How have you kept yourself alive since the roads shut?";
			case "labtech": return "You worked in the labs at the institute. You've still got gloves in your pocket. How have you kept yourself alive since the roads shut?";
			case "radioham": return "You're the one with the aerial on the roof. Half the street heard you on the radio at night. How have you kept alive since the roads shut?";
			case "soldier": return "You were a soldier before the seal. You still stand like one. How have you kept yourself alive since the roads shut?";
			case "raider": return "I know your face. You robbed half this street before the seal. How have you kept yourself alive since the roads shut?";
			case "elder": return "People on this street always listened to you, and they still will. How have you kept yourself alive since the roads shut?";
			case "drifter": return "You're not from round here, are you? Doesn't matter: nobody gets to leave now. How have you kept yourself alive since the roads shut?";
		}

		return "You were out of work before all this, like me. Everyone is now. How have you kept yourself alive since the roads shut?";
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
			case RZ_EPlayerHabit.LIGHT: return "Sensible. The loud ones are the ones the infected find first.";
		}

		return "Useful. A good word gets you further in here than a gun. Just don't use it on me.";
	}

	//------------------------------------------------------------------------------------------------
	//! The background's way, as the reply's label.
	static string WayLabel(string bg)
	{
		switch (bg)
		{
			case "nurse": return "That's an infection, not just a fever. Let me treat him now.";
			case "doctor": return "It's not the fever, it's bad water. Let me treat him now.";
			case "mechanic": return "Forget the battery. Show me the car and I'll hotwire it.";
			case "hunter": return "Before you load a gun, let me read the tracks by your hens.";
			case "police": return "Leave the key. I'll knock as the police. He'll answer me.";
			case "excon": return "Forget the key. I'll open his back door myself, quietly.";
			case "labtech": return "Insulin just needs cold. I'll pack it in cold water for you.";
			case "radioham": return "Never mind your set. I'll call your brother on my radio.";
			case "soldier": return "Leave the lad to me. I'll talk the rifle off him.";
			case "raider": return "I know who collects on this street. I'll get your tin forgotten.";
			case "elder": return "Fetch both families out here. I'll settle the well in the street.";
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
				lines.Insert("Static, then a voice. That's him. That's my brother. He's alive. He says the bridge is out, that's why he couldn't come, and I'm to stop worrying.");
				lines.Insert("He never stopped worrying in his life. Thank you. I'll sleep tonight.");
				return;
			}
			case "raider":
			{
				lines.Insert("They'll forget the tin because you said so? Then I keep my door, and I owe them nothing. I won't ask what the collector owes you.");
				lines.Insert("Keep your voice down when you say things like that. Somebody might believe you.");
				return;
			}
			case "elder":
			{
				lines.Insert("So it's agreed: mornings at the well to the Kerns, evenings to the Orlovs, and the bucket stays at the well for both.");
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
			case RZ_EFavour.CAR: return "The car will start now. Not that there's anywhere to drive to in here, but if I have to get off this street, I can.";
			case RZ_EFavour.HENS: return "Shells. Now if whatever's taking my hens comes back tonight, I'm ready for it.";
			case RZ_EFavour.NEIGHBOUR: return "I'll go round now. Whatever's in there, I'd rather know.";
			case RZ_EFavour.INSULIN: return "Cold. Properly cold. You've bought me a month. Maybe two.";
			case RZ_EFavour.BROTHER: return "It's on. It's on, and that's his voice. He's alive. He's complaining. That's him.";
			case RZ_EFavour.RIFLE: return "Back on its nail. And the lad's still breathing. Good work, both of you.";
			case RZ_EFavour.TIN: return "Peaches. The Red Hand will take peaches. Thank you. That's my front door safe for another week.";
			case RZ_EFavour.WELL: return "The rota says mornings to the Kerns. It's in writing, so the Orlovs can argue with the paper now, not with their fists.";
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
			case 1: return "There, see it? One of the infected. They hear you before they see you, so keep low and keep quiet.";
			case 2: return "That door's locked. Somebody locked it from inside, or somebody means to come back. Either way, it's not ours to open.";
		}

		return "Hear that? The government broadcast. It says aid is on its way, every hour, and it has since the roads shut. Nobody has come.";
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
