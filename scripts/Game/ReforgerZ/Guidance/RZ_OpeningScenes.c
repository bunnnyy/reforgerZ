//! An opening's scene (docs/DESIGN.md, "Openings"): what stands by the door you come out of. The
//! reason's composition (three per reason, Guidance > Opening scenes "hiding.1"...), your
//! background's own things on the other side (its id), the reason's thing to read (the calendar,
//! the group's note, the words on the door), and one clue: the six old spawn scenarios' clues,
//! two to a reason (the scratched door and the farmhouse radio for the hiding place, the group's
//! car and Halcyon's flyer for the group gone, the dead soldier and the dead raider for the fever),
//! each only while it's still true. A veteran (level 3) gets the scene with a one-line read and no
//! clue. The door is left open. Everything here goes with the guidance clues. Server only.

//------------------------------------------------------------------------------------------------
//! Why today: why you came out of the door now (the conversations' opening.why).
enum RZ_EOpeningReason
{
	HIDING = 1,		// the last bottle is empty
	GROUP,			// your group went out three days ago and never came back
	FEVER			// the fever broke: the bite is only a scar now
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningScenes
{
	static const int VARIANTS = 3;					// compositions per reason

	protected static const float CLUE_MIN = 3;		// the clue stands this far from the door's point...
	protected static const float CLUE_MAX = 6;		// ...and no further
	protected static const float DOOR_MIN = 6;		// the scratched door: a real door this far from yours...
	protected static const float DOOR_MAX = 80;		// ...and no further

	protected RZ_GameModeComponent m_GM;
	protected RZ_Guidance m_Guide;
	protected RZ_GuidanceSettings m_Settings;

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningScenes(RZ_GameModeComponent gm, RZ_Guidance guide, RZ_GuidanceSettings settings)
	{
		m_GM = gm;
		m_Guide = guide;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! "hiding", "group", "fever": the reason's key in Guidance > Opening scenes.
	static string ReasonKey(int reason)
	{
		switch (reason)
		{
			case RZ_EOpeningReason.GROUP: return "group";
			case RZ_EOpeningReason.FEVER: return "fever";
		}

		return "hiding";
	}

	//------------------------------------------------------------------------------------------------
	//! A composition from Guidance > Opening scenes by its key; "" if none.
	ResourceName SceneFor(string key)
	{
		if (!m_Settings.m_aOpeningScenes)
			return "";

		foreach (RZ_OpeningSceneSlot slot : m_Settings.m_aOpeningScenes)
		{
			if (slot && slot.m_sKey == key)
				return slot.m_sPrefab;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The scene by the door: the reason's composition in front of it to one side (never the one
	//! this player saw last life), your background's things on the other, the reason's read by the
	//! door, the clue a few metres out, and the door left open.
	void Place(RZ_Opening o, int day, RZ_PlayerProfile profile, RZ_Rng rng)
	{
		RZ_OpeningPlace place = o.m_Place;
		RZ_GuidanceWorld world = m_Guide.GetWorld();
		vector side = Vector(place.m_vOut[2], 0, -place.m_vOut[0]);

		// The reason's composition: one of its three, not last life's
		int last = profile.m_iOpeningLastScene;
		int variant = rng.Int(1, VARIANTS + 1);
		if (o.m_iReason * 10 + variant == last)
			variant = variant % VARIANTS + 1;

		profile.m_iOpeningLastScene = o.m_iReason * 10 + variant;
		o.m_iScene = variant;
		ResourceName scene = SceneFor(ReasonKey(o.m_iReason) + "." + variant.ToString());
		if (!scene.IsEmpty())
		{
			vector sceneAt = RZ_Util.GroundPos(place.m_vPos - side * 2.2 + place.m_vOut * 1.5);
			world.TrackProp(RZ_Util.SpawnPrefab(scene, sceneAt, place.m_fYaw));
		}

		ResourceName props = SceneFor(o.m_sBackground);
		if (!props.IsEmpty())
		{
			vector propsAt = RZ_Util.GroundPos(place.m_vPos + side * 1.8 - place.m_vOut * 0.5);
			world.TrackProp(RZ_Util.SpawnPrefab(props, propsAt, place.m_fYaw));
		}

		bool brief = o.m_iLevel >= RZ_Guidance.LEVEL_VETERAN;
		vector noteAt = RZ_Util.GroundPos(place.m_vPos - side * 1.2);
		world.PlaceReadable(noteAt, RZ_OpeningText.ReadLabel(o.m_iReason), RZ_OpeningText.ReadTitle(o.m_iReason), RZ_OpeningText.Read(o.m_iReason, day, Circled(o), brief));
		if (!brief)
			Clue(o, day, rng);

		RZ_OpeningRoll.OpenDoor(place.m_Door);
	}

	//------------------------------------------------------------------------------------------------
	//! The town the group went to see about: the nearest other one.
	protected string Circled(RZ_Opening o)
	{
		string best = "the next town";
		float bestDist = float.MAX;
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			float d = RZ_Util.DistXZ(s.m_vCenter, o.m_Place.m_vPos);
			if (i != o.m_Place.m_iTown && s.m_sName != o.m_sTown && d < bestDist)
			{
				bestDist = d;
				best = s.m_sName;
			}
		}

		return best;
	}

	// ================================================================ The clue
	//------------------------------------------------------------------------------------------------
	//! One of the reason's two clues that's still true, at random; none if neither is.
	protected void Clue(RZ_Opening o, int day, RZ_Rng rng)
	{
		array<int> kinds = {};
		RZ_WorldFacts facts = m_GM.GetFacts();
		RZ_Leaders leaders = m_GM.GetLeaders();
		switch (o.m_iReason)
		{
			case RZ_EOpeningReason.HIDING:
			{
				// "NOBODY GETS OUT" is only true while the line is sealed
				RZ_Perimeter perimeter = m_GM.GetPerimeter();
				if (perimeter && perimeter.IsActive() && facts && facts.PerimeterSealed())
					kinds.Insert(1);

				if (m_GM.GetBroadcast())
					kinds.Insert(2);

				break;
			}

			case RZ_EOpeningReason.GROUP:
			{
				RZ_Safehouses houses = m_GM.GetSafehouses();
				if (houses && houses.Count() > 0)
					kinds.Insert(3);

				// "Samples wanted": not once the cure is out
				if (m_GM.HasLab() && facts && !facts.IsCured())
					kinds.Insert(4);

				break;
			}

			case RZ_EOpeningReason.FEVER:
			{
				if (leaders && leaders.GetStronghold(RZ_EFaction.MILITARY))
					kinds.Insert(5);

				if (leaders && leaders.GetStronghold(RZ_EFaction.BANDITS))
					kinds.Insert(6);

				break;
			}
		}

		if (kinds.IsEmpty())
			return;

		vector at = RZ_Mission.LandPointNear(o.m_Place.m_vPos + o.m_Place.m_vOut * CLUE_MAX, CLUE_MIN, CLUE_MAX);
		if (RZ_Util.IsWater(at))
			return;

		int kind = kinds[rng.Int(0, kinds.Count())];
		switch (kind)
		{
			case 1: Door(o.m_Place.m_vPos, day, rng); break;
			case 2: Radio(at, rng); break;
			case 3: Car(at, rng); break;
			case 4: Flyer(at); break;
			case 5: Soldier(at, day, rng); break;
			case 6: Raider(at, rng); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The scratched door: the roads out end in concrete, nobody gets out. Scratched into a real
	//! door of a building down the street (DOOR_MIN to DOOR_MAX m from yours): the words go on an
	//! invisible readable spot on its leaf (Interactions > Door spot); no door is ever spawned. No
	//! real door near, or no door spot: no clue.
	protected void Door(vector from, int day, RZ_Rng rng)
	{
		IEntity door;
		vector centre;
		vector normal;
		RZ_Interactions inter = m_GM.GetInteractions();
		ResourceName spot;
		if (inter)
			spot = inter.GetDoorSpotPrefab();

		if (spot.IsEmpty() || !RZ_DoorGeometry.NearestOuterDoor(from, DOOR_MIN, DOOR_MAX, false, door, centre, normal))
		{
			RZ_Log.Info(string.Format("Openings: no real door within %1 m of %2 for the scratched door, no clue", DOOR_MAX, RZ_SettlementScanner.GridReference(from)));
			return;
		}

		IEntity building = RZ_DoorGeometry.BuildingOf(door);
		vector at = centre + normal * 0.15;
		if (building)
			at = RZ_DoorGeometry.HandlePoint(building, door);

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		string roadDir = "";
		string edgeDir = "";
		vector checkpoint;
		if (perimeter.GetNearestCheckpoint(at, checkpoint))
		{
			roadDir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(at, checkpoint));
			roadDir.ToUpper();
		}
		else
		{
			vector inward = perimeter.InwardAt(at);
			edgeDir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(at, at - inward * 100));
			edgeDir.ToUpper();
		}

		string text = RZ_GuidanceText.DoorText(rng, day, roadDir, edgeDir);
		m_Guide.GetWorld().PlaceSceneClue(spot, RZ_EPuzzleProp.CLUE, false, at, "Look at the door", "Scratched into the door", text);
		RZ_Log.Info(string.Format("Openings: the scratched door is a real door at %1, %2 m from the opening", RZ_SettlementScanner.GridReference(at), RZ_Util.RoundToInt(RZ_Util.DistXZ(from, at))));
	}

	//------------------------------------------------------------------------------------------------
	//! The farmhouse radio, still talking: the broadcast, then the Haven's open channel.
	protected void Radio(vector at, RZ_Rng rng)
	{
		string channel = RZ_GuidanceText.HavenChannel(rng, m_Guide.SafehouseText(at));
		ResourceName prop = m_Settings.m_sTableRadioPrefab;
		if (prop.IsEmpty())
			prop = m_Settings.m_sRadioPrefab;

		string text = m_GM.GetBroadcast().GetText() + "\nThen static, and a woman's voice, closer, tired:\n" + channel;
		m_Guide.GetWorld().PlaceSceneClue(prop, RZ_EPuzzleProp.CLUE, false, at, "Listen to the radio", RZ_Guidance.RADIO_TITLE, text);
	}

	//------------------------------------------------------------------------------------------------
	//! The group's car in the ditch, the note on its dashboard pointing at the nearest safehouse.
	protected void Car(vector at, RZ_Rng rng)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_Safehouse h = houses.Get(houses.FindNearest(at));
		if (!h)
			return;

		string direction = RZ_Compass.DirectionWord(RZ_Compass.Bearing(at, h.m_vPos));
		string text = RZ_GuidanceText.CarNote(rng, h.m_sName, direction);
		m_Guide.GetWorld().PlaceSceneClue(m_Settings.m_sCarWreckPrefab, RZ_EPuzzleProp.CLUE, true, at, "Read the note on the dashboard", "Note on the dashboard", text);
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's flyer, the one your group went after: samples wanted at the lab.
	protected void Flyer(vector at)
	{
		string labTown = "the lab town";
		RZ_Settlement s = m_GM.GetSettlement(m_GM.GetLabSettlement());
		if (s)
			labTown = s.m_sName;

		ResourceName prop = m_Settings.m_sFlyerPrefab;
		if (prop.IsEmpty())
			prop = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE);

		m_Guide.GetWorld().PlaceSceneClue(prop, RZ_EPuzzleProp.CLUE, true, at, "Read the flyer", "Flyer", RZ_GuidanceText.FlyerText(labTown, false));
	}

	//------------------------------------------------------------------------------------------------
	//! A dead soldier against a barricade: hold the line, await relief.
	protected void Soldier(vector at, int day, RZ_Rng rng)
	{
		RZ_Leader bastion = m_GM.GetLeaders().GetStronghold(RZ_EFaction.MILITARY);
		if (!bastion)
			return;

		if (!m_Settings.m_sBarricadePrefab.IsEmpty())
			m_Guide.GetWorld().TrackProp(RZ_Util.SpawnPrefab(m_Settings.m_sBarricadePrefab, RZ_Mission.LandPointNear(at, 1, 2), Math.RandomFloat(0, 360)));

		string text = RZ_GuidanceText.OrdersText(rng, day, bastion.m_sPlace);
		m_Guide.GetWorld().PlaceSceneClue(m_Settings.m_sSoldierBodyPrefab, RZ_EPuzzleProp.BODY, true, at, "Read his orders", "The soldier's orders", text);
	}

	//------------------------------------------------------------------------------------------------
	//! A dead raider with a red hand on his jacket and a list of toll roads.
	protected void Raider(vector at, RZ_Rng rng)
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		RZ_Leader boss = leaders.GetStronghold(RZ_EFaction.BANDITS);
		if (!boss)
			return;

		// Toll roads: the Red Hand's other seats first, then any towns
		array<string> places = {};
		array<RZ_Leader> seats = {};
		leaders.GetFactionLeaders(RZ_EFaction.BANDITS, seats);
		foreach (RZ_Leader seat : seats)
		{
			if (seat != boss && !places.Contains(seat.m_sPlace))
				places.Insert(seat.m_sPlace);
		}

		int guard = 0;
		array<ref RZ_Settlement> settlements = m_GM.GetSettlements();
		while (places.Count() < 2 && !settlements.IsEmpty() && guard < 20)
		{
			guard++;
			string name = settlements[rng.Int(0, settlements.Count())].m_sName;
			if (name != boss.m_sPlace && !places.Contains(name))
				places.Insert(name);
		}

		while (places.Count() < 2)
		{
			places.Insert("the old mill");
		}

		string text = RZ_GuidanceText.RaiderText(places[0], places[1], boss.m_sPlace, boss.m_sShort);
		m_Guide.GetWorld().PlaceSceneClue(m_Settings.m_sRaiderBodyPrefab, RZ_EPuzzleProp.BODY, true, at, "Search the body", "The dead man", text);
	}
}
