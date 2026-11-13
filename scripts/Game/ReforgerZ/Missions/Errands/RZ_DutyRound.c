//! The Round (docs/ERRANDS.md, "The routines"): a Haven house with no medic and its sick to see to.
//! The sick are real people lying on cots (Missions > Errands > Props > Round bed), indoors where
//! the house's ground floor has room (against the walls), else under a canvas shelter stood by the
//! house wall (Round tent), else in the yard by the wall. Every piece fits by its measured box
//! (tools/camp_kit/footprints.json): clear at knee and chest, on a floor or level ground, nothing
//! over it outdoors, apart from the others; what doesn't fit is dropped, never squeezed, and each
//! fit or drop is logged. Two to four of the sick, drawn from ten conditions, each with a
//! name. At each bed you talk to them (their own conversation, dialogue/duties/round.rzd: what
//! happened to them), look them over with your hands (the brow, any wound, the breath, the eyes,
//! and asking: time for three of the five), then do one thing for them: the right one helps, the
//! wrong one costs. A bite someone hid and you missed turns on the round. Scored like the ward;
//! a perfect round pays a little more (RZ_ErrandRoutine.Resolve). Server only.

//------------------------------------------------------------------------------------------------
class RZ_DutyRound
{
	protected static const float BEDS_APART = 1.8;		// m between two of the sick indoors
	protected static const float FLOOR_BAND = 0.35;		// a floor point within this of the lowest is on the ground floor (not a table top)
	protected static const float SIDE_OUT = 1.0;		// m out from a cot's middle to stand at its long side
	protected static const float FOOT_OUT = 1.7;		// m out from a cot's middle to stand at its foot
	protected static const float LIE_DELAY = 600;		// ms after spawning before a body is laid down
	protected static const float COT_TOP = 0.45;		// m: a cot's top (measured 0.51, less the sag)

	//------------------------------------------------------------------------------------------------
	//! The round's beds and its people, and their beats into the scene. Where it was laid out goes in
	//! the log. False if nowhere would take even one bed.
	static bool Build(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 47);
		int count = 2 + rng.Int(0, 3);
		array<vector> beds = {};
		array<vector> lay = {};
		string where = Indoors(scene, count, beds, lay);
		if (beds.IsEmpty())
			where = Tent(scene, count, beds, lay);

		if (beds.IsEmpty())
			where = Yard(scene, count, beds, lay);

		if (beds.IsEmpty())
		{
			RZ_Log.Info(string.Format("Errands: The Round at %1: nowhere fit to lay out the sick.", RZ_SettlementScanner.GridReference(scene.m_vHome)));
			return false;
		}

		// The sick: distinct conditions, at most one hidden bite
		array<int> conds = {};
		int guard = 0;
		while (conds.Count() < beds.Count() && guard < 60)
		{
			guard++;
			int c = rng.Int(0, KINDS);
			if (!conds.Contains(c))
				conds.Insert(c);
		}

		string town = e.GetText("here");
		if (town.IsEmpty())
			town = "the town";

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("round", "Two minutes at each bed, the clerk said. Talk to them first. Then look, and do what's right by them.", RZ_DutyBeat.AT_SET).m_vAt = Stand(beds[0], lay[0]);
		RZ_DutyScript.O("Start the round", "You go to the first bed. They hear you coming.", "hold:2");
		foreach (int n, int cond : conds)
		{
			string name = RZ_NoteText.FullName(rng);
			int body = Patient(gm, e, scene, beds[n], lay[n][0]);
			Bed(scene, n, cond, name, town, beds[n], body, lay[n][1], lay[n][2]);
		}

		RZ_Log.Info(string.Format("Errands: The Round at %1: %2 of the sick %3.", RZ_SettlementScanner.GridReference(scene.m_vHome), beds.Count(), where));
		return true;
	}

	// ================================================================ Where they lie
	//------------------------------------------------------------------------------------------------
	//! A measured prop's box in its own space (tools/camp_kit/footprints.json: w, d, h, ox, oz).
	//! False for a prop that was never measured: it isn't used.
	static bool Box(ResourceName prefab, out vector fmin, out vector fmax)
	{
		array<float> m = {};
		if (prefab.Contains("CotMilitary_US_01.et"))
			m = {2.308, 0.857, 0.51, 0, -0.004};
		else if (prefab.Contains("CanvasCover_Medium_US.et"))
			m = {4.58, 5.278, 3.552, 0, -0.052};
		else
			return false;

		fmin = Vector(m[3] - m[0] * 0.5, 0, m[4] - m[1] * 0.5);
		fmax = Vector(m[3] + m[0] * 0.5, m[2], m[4] + m[1] * 0.5);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The house's ground floor: against its walls (the loose loot's floor points, the lowest storey,
	//! never a table top), a cot each (Round bed) with a clear box, a floor under every corner and
	//! nothing in the way, kept apart from the other cots. "" if fewer than two fit.
	protected static string Indoors(RZ_DutyScene scene, int count, notnull array<vector> beds, notnull array<vector> lay)
	{
		vector fmin;
		vector fmax;
		ResourceName cot = RZ_ErrandSettings.Get().Props().m_sRoundBed;
		if (cot.IsEmpty() || !Box(cot, fmin, fmax))
			return "";

		IEntity building = RZ_LootAnchors.BuildingAt(scene.m_vHome, 30);
		if (!building)
			return "";

		array<vector> anchors = {};
		RZ_LootAnchors.Find(building, anchors, RZ_LootAnchors.MAX_ANCHORS);
		if (anchors.Count() < 2)
			return "";

		float lowest = anchors[0][1];
		foreach (vector a : anchors)
		{
			lowest = Math.Min(lowest, a[1]);
		}

		vector middle = building.GetOrigin();
		array<vector> taken = {};
		array<vector> spots = {};
		array<float> turns = {};
		array<vector> sides = {};
		foreach (vector p : anchors)
		{
			if (spots.Count() >= count || p[1] - lowest > FLOOR_BAND)
				continue;

			// Off the wall toward the room's middle, the cot's long side along the wall, then its foot
			// end to the room. Whoever sees to them stands on the room side.
			vector inward = middle - p;
			inward[1] = 0;
			inward.Normalize();
			float wallYaw = Bearing(p, middle);
			bool placed = false;
			for (int k = 0; k < 3 && !placed; k++)
			{
				for (int t = 0; t < 2 && !placed; t++)
				{
					float yaw = wallYaw - t * 90;
					float off = 0.55 + k * 0.4;
					float reach = SIDE_OUT;
					if (t == 1)
					{
						off = 1.25 + k * 0.4;
						reach = FOOT_OUT;
					}

					vector pos = p + inward * off;
					vector floor;
					if (!FloorUnder(pos, floor))
						continue;

					if (!RZ_CampLife.FitsIndoors(floor, yaw, fmin, fmax) || Overlaps(floor, yaw, fmin, fmax, taken))
						continue;

					RZ_CampLife.Quad(floor, yaw, fmin, fmax, 0, taken);
					spots.Insert(floor);
					turns.Insert(yaw);
					sides.Insert(Vector(wallYaw, reach, 0));
					placed = true;
				}
			}
		}

		if (spots.Count() < 2)
		{
			RZ_Log.Info(string.Format("Errands: The Round: indoors dropped, %1 of %2 cots had a clear box in the house.", spots.Count(), count));
			return "";
		}

		if (!Spawn(scene, cot, spots, turns, sides, beds, lay, "indoors"))
			return "";

		return "lying indoors in the house";
	}

	//------------------------------------------------------------------------------------------------
	//! The medical tent by the house wall: a canvas shelter (Round tent) with the cots under it laid
	//! out as JTF Bronze Props' casualty point lays them (three side by side, a fourth across the
	//! back). The shelter's box clear at knee and chest, nothing over it, the ground flat enough, every
	//! cot's own box clear; a few spots and headings tried, else none. "" with nothing that fits.
	protected static string Tent(RZ_DutyScene scene, int count, notnull array<vector> beds, notnull array<vector> lay)
	{
		vector tmin;
		vector tmax;
		vector cmin;
		vector cmax;
		ResourceName tent = RZ_ErrandSettings.Get().Props().m_sRoundTent;
		ResourceName cot = RZ_ErrandSettings.Get().Props().m_sRoundBed;
		if (tent.IsEmpty() || !Box(tent, tmin, tmax))
			return "";

		bool cots = !cot.IsEmpty() && Box(cot, cmin, cmax);
		if (!cots)
		{
			cmin = Vector(-1.1, 0, -0.45);
			cmax = Vector(1.1, 0.3, 0.45);
		}

		// The cots' places under the canvas (its own space, the open front ahead): x, z, turn, how far
		// out in front of it its looks stand
		array<float> layout = {-1.2, 0.4, 90, FOOT_OUT, 0, 0.4, 90, FOOT_OUT, 1.2, 0.4, 90, FOOT_OUT, 0, -1.6, 0, 0.75};
		for (int tries = 0; tries < 8; tries++)
		{
			vector at = RZ_WorkSites.Spot(scene.m_vHome, 6, 14);
			// Its open front away from the house, else sideways to it
			int sideways = tries % 2;
			float yaw = Bearing(scene.m_vHome, at) + sideways * 90;
			array<vector> quad = {};
			RZ_CampLife.Quad(at, yaw, tmin, tmax, 0, quad);
			if (RZ_CampLife.Blocked(quad) || !Level(quad, 0.6) || Covered(quad))
				continue;

			array<vector> spots = {};
			array<float> turns = {};
			array<vector> sides = {};
			for (int n = 0; n < count; n++)
			{
				vector c = RZ_SiteGrid.Frame(at, yaw, layout[n * 4], layout[n * 4 + 1]);
				float turn = yaw + layout[n * 4 + 2];
				c = RZ_Util.GroundPos(c);
				array<vector> box = {};
				RZ_CampLife.Quad(c, turn, cmin, cmax, 0, box);
				if (RZ_CampLife.Blocked(box))
				{
					RZ_Log.Info(string.Format("Errands: The Round: a cot under the tent dropped at %1: something in the way.", c.ToString()));
					continue;
				}

				spots.Insert(c);
				turns.Insert(turn);
				sides.Insert(Vector(yaw, layout[n * 4 + 3], 0));
			}

			if (spots.Count() < 2)
				continue;

			IEntity e = RZ_Util.SpawnPrefab(tent, RZ_Util.GroundPos(at), yaw);
			if (!e)
				return "";

			// Nothing on a canvas shelter to use, but strip it as every dressing piece is
			RZ_DressingGuard.Strip(e, tent);
			scene.m_aProps.Insert(e);
			RZ_Log.Info(string.Format("Errands: The Round: the tent fits at %1, yaw %2 (try %3).", at.ToString(), Math.Round(yaw), tries + 1));
			if (cots)
				Spawn(scene, cot, spots, turns, sides, beds, lay, "under the tent");
			else
				Floor(spots, turns, sides, beds, lay);

			return "under the medical tent by the house wall";
		}

		RZ_Log.Info("Errands: The Round: the tent dropped: no clear, level, open spot by the house wall in 8 tries.");
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Neither: cots in a row in the yard by the house wall (Round bed), each box clear and nothing
	//! over it, apart from the others; a few spots tried, a cot that won't fit is left out.
	protected static string Yard(RZ_DutyScene scene, int count, notnull array<vector> beds, notnull array<vector> lay)
	{
		vector fmin;
		vector fmax;
		ResourceName cot = RZ_ErrandSettings.Get().Props().m_sRoundBed;
		bool cots = !cot.IsEmpty() && Box(cot, fmin, fmax);
		if (!cots)
		{
			fmin = Vector(-1.1, 0, -0.45);
			fmax = Vector(1.1, 0.3, 0.45);
		}

		array<vector> taken = {};
		array<vector> spots = {};
		array<float> turns = {};
		array<vector> sides = {};
		for (int tries = 0; tries < 6 * count && spots.Count() < count; tries++)
		{
			vector at = RZ_Util.GroundPos(RZ_WorkSites.Spot(scene.m_vHome, 4, 10));
			// Long side to the wall (a cot's long side is its own x); its looks on the side away from it
			float yaw = Bearing(at, scene.m_vHome);
			array<vector> quad = {};
			RZ_CampLife.Quad(at, yaw, fmin, fmax, 0, quad);
			if (RZ_CampLife.Blocked(quad) || !Level(quad, 0.3) || Covered(quad) || Overlaps(at, yaw, fmin, fmax, taken))
				continue;

			RZ_CampLife.Quad(at, yaw, fmin, fmax, 0, taken);
			spots.Insert(at);
			turns.Insert(yaw);
			sides.Insert(Vector(yaw + 180, SIDE_OUT, 0));
		}

		if (spots.Count() < count)
			RZ_Log.Info(string.Format("Errands: The Round: %1 of %2 cots fit in the yard; the rest dropped.", spots.Count(), count));

		if (spots.IsEmpty())
			return "";

		if (cots)
			Spawn(scene, cot, spots, turns, sides, beds, lay, "in the yard");
		else
			Floor(spots, turns, sides, beds, lay);

		return "on cots in the yard by the house wall (no room inside, no tent)";
	}

	//------------------------------------------------------------------------------------------------
	//! The cots spawned where they fit (each logged), and where the sick lie on them: on top, lying
	//! along the cot. `lay`: per bed, the body's yaw, the bearing of the side to stand on, and how far
	//! out from the bed its looks stand. False if none spawned.
	protected static bool Spawn(RZ_DutyScene scene, ResourceName cot, array<vector> spots, array<float> turns, array<vector> sides, notnull array<vector> beds, notnull array<vector> lay, string where)
	{
		foreach (int n, vector p : spots)
		{
			IEntity e = RZ_Util.SpawnPrefab(cot, p, turns[n]);
			if (!e)
			{
				RZ_Log.Info(string.Format("Errands: The Round: cot %1 %2 dropped: it didn't spawn.", n + 1, where));
				continue;
			}

			scene.m_aProps.Insert(e);
			// VERIFY: a body laid down at the cot's top surface (its measured height, less a hand for the
			// sag) settles on it, not through it or floating over it
			beds.Insert(p + Vector(0, COT_TOP, 0));
			// The cot's long side is its own x: the body lies along it
			lay.Insert(Vector(turns[n] + 90, sides[n][0], sides[n][1]));
			RZ_Log.Info(string.Format("Errands: The Round: cot %1 fits %2 at %3, yaw %4.", n + 1, where, p.ToString(), Math.Round(turns[n])));
		}

		return !beds.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! No cot set: they lie on the ground in the cots' places.
	protected static void Floor(array<vector> spots, array<float> turns, array<vector> sides, notnull array<vector> beds, notnull array<vector> lay)
	{
		foreach (int n, vector p : spots)
		{
			beds.Insert(p);
			lay.Insert(Vector(turns[n] + 90, sides[n][0], sides[n][1]));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The floor straight under `pos` (a downward trace, so a storey's floor and not the ground under
	//! the house).
	protected static bool FloorUnder(vector pos, out vector floor)
	{
		vector from = pos + Vector(0, 0.8, 0);
		vector to = pos + Vector(0, -0.6, 0);
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = to;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		float frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param);
		if (frac >= 0.999)
			return false;

		floor = from + (to - from) * frac;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A box's corners are within `rise` m of each other on the ground.
	protected static bool Level(notnull array<vector> quad, float rise)
	{
		float lo = 99999;
		float hi = -99999;
		foreach (vector c : quad)
		{
			float y = RZ_Util.GroundPos(c)[1];
			lo = Math.Min(lo, y);
			hi = Math.Max(hi, y);
		}

		return hi - lo <= rise;
	}

	//------------------------------------------------------------------------------------------------
	//! Something over a box outdoors (a branch, an eave, a balcony): straight up from its middle and
	//! its corners.
	protected static bool Covered(notnull array<vector> quad)
	{
		vector mid = (quad[0] + quad[2]) * 0.5;
		array<vector> pts = {mid, quad[0], quad[1], quad[2], quad[3]};
		foreach (vector c : pts)
		{
			vector g = RZ_Util.GroundPos(c);
			if (RZ_CampLife.TraceHits(g + Vector(0, 1.5, 0), g + Vector(0, 8, 0), false))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A box at pos comes within a person's width of a box already taken (four corners each in `taken`).
	protected static bool Overlaps(vector pos, float yaw, vector fmin, vector fmax, notnull array<vector> taken)
	{
		array<vector> quad = {};
		RZ_CampLife.Quad(pos, yaw, fmin, fmax, 0.3, quad);
		for (int i = 0; i + 3 < taken.Count(); i += 4)
		{
			if (RZ_CampLife.QuadsOverlap(quad, 0, taken, i))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Where to stand by a bed: out on its open side (`lay`: the body's yaw, the side's bearing, how far).
	protected static vector Stand(vector bed, vector lay)
	{
		float rad = lay[1] * Math.DEG2RAD;
		return bed + Vector(Math.Sin(rad), 0, Math.Cos(rad)) * lay[2];
	}

	//------------------------------------------------------------------------------------------------
	//! Degrees clockwise from north, from a toward b.
	protected static float Bearing(vector a, vector b)
	{
		return Math.Atan2(b[0] - a[0], b[2] - a[2]) * Math.RAD2DEG;
	}

	// ================================================================ The sick
	//------------------------------------------------------------------------------------------------
	//! One of the sick in their bed: a civilian (Duty person, else Puzzles > Civilian), in the body
	//! budget as an errand's, tagged so the talk screen knows them, and laid down. Indoors, a bed
	//! under them (Round bed). The index into the scene's own people, -1 if nobody could be put there
	//! (then the bed is seen to with your hands alone).
	protected static int Patient(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene, vector at, float yaw)
	{
		ResourceName prefab = RZ_ErrandSettings.Get().Props().m_sDutyPerson;
		if (prefab.IsEmpty())
			prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);

		RZ_Shared shared = gm.GetShared();
		if (prefab.IsEmpty() || (shared && !shared.CanSpawnBody(RZ_BodyBudget.ERRAND, at)))
			return -1;

		IEntity body = RZ_Util.SpawnPrefab(prefab, at, yaw);
		if (!body)
			return -1;

		if (shared)
			shared.AddBody(RZ_BodyBudget.ERRAND, body);

		RZ_PuzzleObjectComponent.Tag(body, RZ_DutyScenes.TAG_PERSON, e.m_iId, "", "");
		GetGame().GetCallqueue().CallLater(LieDown, LIE_DELAY, false, body);

		scene.m_aBodies.Insert(body);
		return scene.m_aBodies.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Laid down where they are: vanilla's unconscious state, which lies the body flat and keeps it
	//! there with no AI moving it.
	//! VERIFY: CharacterControllerComponent.SetUnconscious(true) on a freshly spawned AI-less civilian
	//! on the server lays it down for every client, it stays down (nothing wakes it while no damage
	//! comes), and the talk prompt on it still shows. If the prompt vanishes, the sick can't be talked
	//! to and their beds are seen to with your hands alone: say so on the board.
	static void LieDown(IEntity body)
	{
		if (!body)
			return;

		CharacterControllerComponent ctrl = CharacterControllerComponent.Cast(body.FindComponent(CharacterControllerComponent));
		if (ctrl)
			ctrl.SetUnconscious(true);
	}

	// ================================================================ The ten conditions
	static const int KINDS = 10;
	static const int KIND_FEVER = 0;
	static const int KIND_CUT = 1;
	static const int KIND_BITE = 2;
	static const int KIND_HUNGER = 3;
	static const int KIND_FALL = 4;
	static const int KIND_CHILD = 5;
	static const int KIND_BED_OLD = 6;
	static const int KIND_CHEST = 7;
	static const int KIND_BURN = 8;
	static const int KIND_TOO_FAR = 9;

	//------------------------------------------------------------------------------------------------
	//! Their conversation's key (dialogue/duties/round.rzd: duty:round:<key>).
	static string Key(int cond)
	{
		array<string> keys = {"fever", "cut", "bite", "hunger", "fall", "child", "old", "chest", "burn", "toofar"};
		return keys[cond];
	}

	//------------------------------------------------------------------------------------------------
	//! One bed: what's said as you come up, five looks (time for three), three things to do. The
	//! options' order is fixed (the conversation's replies run them by number): 0 the brow, 1 any
	//! wound, 2 the breath, 3 the eyes, 4 asking, 5-7 the calls.
	protected static void Bed(RZ_DutyScene scene, int n, int cond, string name, string town, vector at, int body, float face, float reach)
	{
		array<string> line = {
			string.Format("%1, under two blankets and shivering anyway. The fever that's going round %2.", name, town),
			string.Format("%1, on top of the blanket, one hand held up on their chest like a parcel.", name),
			string.Format("%1, sleeves pulled down to the knuckles, smiling too quickly when you come up.", name),
			string.Format("%1, very still, very polite. The bowl by the bed is clean and was never used.", name),
			string.Format("%1, holding one arm against their chest with the other.", name),
			string.Format("%1, eight or nine, facing the wall.", name),
			string.Format("%1, old, propped on a pillow, watching the door and nothing else.", name),
			string.Format("%1, coughing into a rag, and the rag says it's been a while.", name),
			string.Format("%1, one hand wrapped in a wet tea towel.", name),
			string.Format("%1, grey in the face, breathing in a way you don't want to count.", name)};
		array<string> brow = {"Hot, and dry, all over.", "Warm. Not hot.", "Hot, and wet with it.", "Cool. Too cool.", "Normal.", "Normal.", "Cool and papery.", "Warm.", "Normal.", "Burning."};
		array<string> wound = {"Nothing. Not a mark on them.", "A cut across the palm. Red to the wrist, and the smell of it.", "Under the sleeve, a bandage they didn't put on well. Under the bandage: a bite, two days old.", "No wound. You can count every rib.", "The wrist is swollen and the wrong shape.", "Nothing. A scraped knee, old.", "Nothing new. Old scars, a lot of them.", "Nothing.", "The palm and fingers, red and blistered.", "A wound in the side, dressed, and the dressing's black."};
		array<string> breath = {"Fast and shallow.", "Steady.", "Ragged, and it catches.", "Slow.", "It catches when they move the arm.", "Steady, until you ask about home.", "Slow and even. They've stopped trying.", "Rattling. Worse lying down.", "Fast. They're in pain.", "Short. A little blood in it."};
		array<string> eyes = {"Glassy, but they follow you.", "Clear, and cross.", "Slow to the light, the left slower than the right.", "Sunk in, and clear.", "Clear. Wet.", "Clear. On the door the whole time.", "Clear. Not interested in you.", "Red-rimmed, from the coughing.", "Clear, watering.", "Half closed. They come back when you say the name."};
		array<string> ask = {
			string.Format("Half of %1 has it, they say. It started at the well and went house to house.", town),
			"A tin, they say. Opening a tin with a knife. Four days ago. It was fine yesterday.",
			"Nothing, they say. A cold. They look at the door when they say it.",
			"They gave theirs to the little ones, they say. Every night for a week. Nobody noticed. That was the idea.",
			"The stairs, in the dark. The rail wasn't there any more.",
			"Mama went to get water from the far well. She said wait. They're waiting.",
			"They ask why you'd bother. They had a shop, once, and a family. They've had enough of being had.",
			"Weeks, they say. The roof leaks onto the bed. Lying flat is when it's worst.",
			"The stove, they say. The pot slipped. It was supper. Everyone ate. They're proud of that.",
			"A scavenging run. A fence, and a nail. They dressed it themselves. It's gone bad since."};
		array<string> calls = {
			"Water, and a cool wet cloth", "Clean it and dress it", "Fetch the clerk, quietly",
			"Give them something to eat", "Splint it and sling it", "Sit with them and talk",
			"Sit them up and get them talking", "Sit them up and keep them up", "Cool water, then a loose dressing",
			"Send for the medic from the next house"};
		array<string> follows = {
			"Water from the pail, a cloth on the forehead, the window opened a crack. By the end of the round they've stopped shaking.",
			"You clean it until they swear at you, and dress it, and tell them to keep it dry. They swear at that too, and then thank you.",
			"You find the clerk and say it quietly. The clerk shuts their eyes for a second, then goes for two of the men. Nobody shouts.",
			"A bowl from the pot, and a tin of your own. They eat it as if it might be taken off them.",
			"Two pieces of a crate and a strip of sheet, then a sling. It's not a doctor's work. It'll set straight.",
			"You sit on the end of the bed and ask about the far well, and the mother, and the dog they had. After a while they turn round.",
			"You sit them up and ask about the shop. They tell you what bread cost. Then what everything cost. They're still talking when you go.",
			"A rolled blanket behind them. Sitting up, the cough eases. You tell them to sleep that way, and they say they will.",
			"Water from the pail on the burn for as long as they'll stand it, then a dressing that doesn't stick. They stop holding their breath.",
			"Someone runs to the next house. Their medic comes at a walk, then at a run. It was past what you could do, and you knew it."};
		array<string> codes = {"", "med1", "report", "food1", "med1", "morale+", "regard+", "", "med1", ""};

		RZ_DutyBeat b = RZ_DutyScript.T("bed" + n.ToString(), line[cond], RZ_DutyBeat.AT_SET);
		b.m_vAt = at;
		b.m_iPicks = 3;
		b.m_iBody = body;
		// A bearing in 0..360 (a beat's face below 0 means none)
		float facing = face;
		while (facing < 0)
		{
			facing += 360;
		}

		while (facing >= 360)
		{
			facing -= 360;
		}

		b.m_fFace = facing;
		b.m_sTalk = "round:" + Key(cond);
		b.m_sTalkName = name;
		Look(RZ_DutyScript.O("Feel their brow", brow[cond], "care hold:2"), reach);
		Look(RZ_DutyScript.O("Look them over for a wound", wound[cond], "care hold:3"), reach);
		Look(RZ_DutyScript.O("Listen to their breathing", breath[cond], "care hold:2"), reach);
		Look(RZ_DutyScript.O("Look in their eyes", eyes[cond], "care hold:2"), reach);
		Look(RZ_DutyScript.O("Ask what happened", ask[cond], "hold:2"), reach);
		// Three calls: the right one for this, and two that are right for something else
		array<int> shown = {cond, (cond + 3) % KINDS, (cond + 7) % KINDS};
		if (cond == KIND_BITE)
			shown = {KIND_CUT, KIND_BITE, KIND_FEVER};

		foreach (int k : shown)
		{
			string code = "ends care";
			if (k == cond)
				code += " right " + codes[k];
			else if (cond == KIND_BITE)
				code += " wrong flag:missed " + codes[k];
			else
				code += " wrong " + codes[k];

			if (k == KIND_BED_OLD || k == KIND_CHILD)
				code += " hold:6";
			else if (k == KIND_FALL)
				code += " hold:5";
			else
				code += " hold:4";

			RZ_DutyOption call = RZ_DutyScript.O(calls[k], Wrong(k, cond, follows[k]), code);
			Look(call, reach);
		}

		// A bite missed: it comes out before the round's done
		RZ_DutyBeat turned = RZ_DutyScript.T("turned" + n.ToString(), string.Format("Behind you, the bed creaks. %1 is sitting up, and isn't looking at you the way a person looks.", name), RZ_DutyBeat.AT_SET, "missed");
		turned.m_vAt = at;
		RZ_DutyScript.O("Shout for the house", "The men come with a blanket and a pole. It takes three of them. The house watches it happen, and nobody sleeps tonight.", "hold:3 morale- infect+", "", "", "Someone on your round had a bite under their sleeve, and it got past you. Two of ours are being stitched.");
	}

	//------------------------------------------------------------------------------------------------
	//! A call done for someone it wasn't right for: what follows says so.
	protected static string Wrong(int call, int cond, string right)
	{
		if (call == cond)
			return right;

		if (cond == KIND_BITE)
			return "They let you, and thank you, and pull the sleeve back down the moment you turn away.";

		if (cond == KIND_TOO_FAR)
			return "You do it. It does nothing. Their breathing goes on getting shorter while you do it.";

		return "They let you. It was no help, and you can see they know that.";
	}

	//------------------------------------------------------------------------------------------------
	//! Every look and call stands on the bed's open side, spread along it, not in a row across the room.
	protected static void Look(RZ_DutyOption o, float reach)
	{
		if (o)
			o.m_fOut = reach;
	}
}
