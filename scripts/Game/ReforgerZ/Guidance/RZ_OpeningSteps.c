//! An opening's step 2 by background (docs/DESIGN.md, "Openings"): each background's own system,
//! on the errand engine like the chain's other steps. Which one a life gets is the background's
//! (Preferred), if it can run here now (Fits: a pump in reach, a locked door, a wanderer on the
//! road...); else the chain's plain step 2, water from the well or a hand to dress; else none, and
//! the chain goes on to its walk. The Drifter gets up to three small asks in a row, each another
//! background's. While a step runs, RZ_ErrandDef.StepCounts asks Counts whether its clock may run
//! (fuel in the can, inside the locked building, hidden, no shots near the toll...). Server only.

//------------------------------------------------------------------------------------------------
//! What step 2 is (the character flag opening.task, for the conversations).
enum RZ_EOpeningTask
{
	NONE,
	WATER,		// anyone: fill what you can carry at the nearest well
	CUT,		// anyone with something medical: dress the contact's hand
	FEVER,		// Nurse: treat the contact's boy
	BOILED,		// Nurse with nothing medical: water for the fever, to boil
	PUMP,		// Mechanic: fuel from the nearest petrol station's pump
	DOWNWIND,	// Hunter: watch a pack's ground, unseen
	KEYS,		// Police Officer: into a locked building with its key
	TOLL,		// Ex-Con: up to a Red Hand toll and away again, without a fight
	SAMPLE,		// Lab Technician: a sample from what's left of a big one
	CHANNEL,	// Radio Operator: answer someone on the open channel
	SWAP,		// Unemployed: a swap with a wanderer on the road
	GATE,		// Soldier: hold the gate for a minute
	CLOCK,		// Doctor: the antidote for someone bitten, against the clock
	CLOCK_MED,	// Doctor without an antidote: keep them going with a dressing
	DEBT,		// Raider: collect what a house owes
	SETTLE		// Elder: settle two families' argument (talk, a check)
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningTasks
{
	static const string FLAG_SETTLED = "opening.settled";	// SETTLE: 1 settled, 2 half settled (the conversation's check)
	static const string FLAG_ANSWERED = "opening.answered";	// CHANNEL: 1 answered on the open channel

	protected static const float NEAR = 1500;			// a pump, a toll or a wanderer this close to the contact
	protected static const float PUMP_NEAR = 800;
	protected static const float LAIR_NEAR = 1600;
	protected static const float LAIR_WATCH = 150;		// the watch point stands this far short of a lair
	protected static const float KEYS_MIN = 40;
	protected static const float KEYS_MAX = 300;
	protected static const float FUEL_LITRES = 5;
	protected static const float GATE_HEAT = 12;		// a noise at the gate (the infected come to it)...
	protected static const float GATE_PULSE = 20;		// ...this often while you hold it
	protected static const float TOLL_SHOTS = 80;		// no shot within this of the toll...
	protected static const float TOLL_SECONDS = 60;		// ...in this long
	protected static const int SWAP_FAVOR = 3;			// Haven Favor for a tin swapped with a wanderer (small pay)
	protected static const int DEBT_INFAMY = 1;			// the Haven hears how you asked
	protected static const int DRIFTER_ASKS = 3;
	protected static const float SAMPLE_MIN = 80;		// A Sample's remains: this far from the contact...
	protected static const float SAMPLE_MAX = 250;		// ...and no further
	protected static const string SAMPLE_ROAD = "what's left of a big one by the road";
	protected static const string SAMPLE_YARD = "what's left of a big one in a yard over the way";

	protected RZ_GameModeComponent m_GM;

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningTasks(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! The background's own step 2, in the order tried (the second when the first can't run).
	static void Preferred(string bg, notnull array<int> result)
	{
		switch (bg)
		{
			case "nurse": result.Insert(RZ_EOpeningTask.FEVER); result.Insert(RZ_EOpeningTask.BOILED); return;
			case "doctor": result.Insert(RZ_EOpeningTask.CLOCK); result.Insert(RZ_EOpeningTask.CLOCK_MED); return;
			case "mechanic": result.Insert(RZ_EOpeningTask.PUMP); return;
			case "hunter": result.Insert(RZ_EOpeningTask.DOWNWIND); return;
			case "police": result.Insert(RZ_EOpeningTask.KEYS); return;
			case "excon": result.Insert(RZ_EOpeningTask.TOLL); return;
			case "labtech": result.Insert(RZ_EOpeningTask.SAMPLE); return;
			case "radioham": result.Insert(RZ_EOpeningTask.CHANNEL); return;
			case "unemployed": result.Insert(RZ_EOpeningTask.SWAP); return;
			case "soldier": result.Insert(RZ_EOpeningTask.GATE); return;
			case "raider": result.Insert(RZ_EOpeningTask.DEBT); return;
			case "elder": result.Insert(RZ_EOpeningTask.SETTLE); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Drifter's asks: a sample of everything, shuffled (up to three that can run, in a row).
	static void DrifterPool(RZ_Rng rng, notnull array<int> result)
	{
		result.Insert(RZ_EOpeningTask.GATE);
		result.Insert(RZ_EOpeningTask.DOWNWIND);
		result.Insert(RZ_EOpeningTask.SWAP);
		result.Insert(RZ_EOpeningTask.DEBT);
		result.Insert(RZ_EOpeningTask.SETTLE);
		result.Insert(RZ_EOpeningTask.WATER);
		result.Insert(RZ_EOpeningTask.CUT);
		rng.ShuffleInts(result);
	}

	//------------------------------------------------------------------------------------------------
	static int DrifterAsks()
	{
		return DRIFTER_ASKS;
	}

	//------------------------------------------------------------------------------------------------
	//! Step 2 for this opening now: the next of its plan that can run (the Drifter's asks, or the
	//! background's own), else water, else the hand. NONE: no step 2 here. Fills where it is.
	int Pick(RZ_Opening o, IEntity me, vector from, out vector place, out string text)
	{
		place = vector.Zero;
		text = "";
		while (!o.m_aTasks.IsEmpty())
		{
			int next = o.m_aTasks[0];
			o.m_aTasks.RemoveOrdered(0);
			if (Fits(o, me, next, from, place, text))
				return next;
		}

		// The Drifter's later asks never fall back: two of three is still a sample
		if (o.m_iAsks > 0)
			return RZ_EOpeningTask.NONE;

		if (Fits(o, me, RZ_EOpeningTask.WATER, from, place, text))
			return RZ_EOpeningTask.WATER;

		if (Fits(o, me, RZ_EOpeningTask.CUT, from, place, text))
			return RZ_EOpeningTask.CUT;

		return RZ_EOpeningTask.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! Can this task run now, from the contact at `from`? Fills its place, the text {place} says,
	//! and the opening's target (the building, the wanderer).
	bool Fits(RZ_Opening o, IEntity me, int task, vector from, out vector place, out string text)
	{
		o.m_Target = null;
		o.m_iTargetId = -1;
		switch (task)
		{
			case RZ_EOpeningTask.WATER: return Well(from, place, text);
			case RZ_EOpeningTask.BOILED: return Well(from, place, text);
			case RZ_EOpeningTask.CUT: return Here(o, me, RZ_EErrandItem.MEDICAL, from, place, text);
			case RZ_EOpeningTask.FEVER: return Here(o, me, RZ_EErrandItem.MEDICAL, from, place, text);
			case RZ_EOpeningTask.CLOCK: return Here(o, me, RZ_EErrandItem.ANTIDOTE, from, place, text);
			case RZ_EOpeningTask.CLOCK_MED: return Here(o, me, RZ_EErrandItem.MEDICAL, from, place, text);
			case RZ_EOpeningTask.PUMP: return Pump(from, place, text);
			case RZ_EOpeningTask.DOWNWIND: return Downwind(from, place, text);
			case RZ_EOpeningTask.KEYS: return Keys(o, from, place, text);
			case RZ_EOpeningTask.TOLL: return Toll(o, from, place, text);
			case RZ_EOpeningTask.SAMPLE: return Sample(from, place, text);
			case RZ_EOpeningTask.CHANNEL: return Channel(o, me, from, place, text);
			case RZ_EOpeningTask.SWAP: return Swap(o, from, place, text);
			case RZ_EOpeningTask.GATE: return Gate(from, place, text);
			case RZ_EOpeningTask.DEBT: return Debt(from, place, text);
			case RZ_EOpeningTask.SETTLE: return Settle(o, from, place, text);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Done at the contact, with something you carry.
	protected bool Here(RZ_Opening o, IEntity me, RZ_EErrandItem item, vector from, out vector place, out string text)
	{
		if (RZ_Errands.CountItems(me, item) <= 0)
			return false;

		place = from;
		text = o.m_sName;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest well or water tower within 400 m ("the well"): a dry spot by it.
	protected bool Well(vector from, out vector place, out string text)
	{
		array<string> kinds = {"well", "watertower"};
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(from, 400, kinds);
		if (!l)
			return false;

		place = RZ_PlaceFind.Near(l.m_vPos, 0, 6, RZ_PlaceNeeds.Get("opening.water"));
		text = l.m_sName;
		return place != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest petrol station within 800 m: its pump fills a can (RZ_FuelPumps, or the hand pump).
	protected bool Pump(vector from, out vector place, out string text)
	{
		array<string> kinds = {"fuel"};
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(from, PUMP_NEAR, kinds);
		if (!l)
			return false;

		// A dry spot by the nearest; else by any other in reach (one by a river may have none)
		RZ_PlaceNeed need = RZ_PlaceNeeds.Get("opening.pump");
		place = RZ_PlaceFind.Near(l.m_vPos, 0, 15, need);
		if (place == vector.Zero)
			place = RZ_PlaceFind.Near(from, 0, PUMP_NEAR, need);

		text = l.m_sName;
		return place != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! Short of the nearest pack's lair within 1.6 km, on the side of the contact; with no pack near,
	//! the tree line 150-250 m out, where they cross.
	protected bool Downwind(vector from, out vector place, out string text)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		RZ_Tribe best;
		float bestDist = LAIR_NEAR;
		if (tribes && tribes.IsEnabled())
		{
			foreach (RZ_Tribe t : tribes.GetTribeList())
			{
				float d = RZ_Util.DistXZ(t.m_vLair, from);
				if (d < bestDist)
				{
					bestDist = d;
					best = t;
				}
			}
		}

		if (best)
		{
			vector toward = from - best.m_vLair;
			toward[1] = 0;
			toward.Normalize();
			place = RZ_PlaceFind.Near(best.m_vLair + toward * LAIR_WATCH, 0, 30, RZ_PlaceNeeds.Get("opening.downwind"));
			text = best.m_sPlace;
			if (place != vector.Zero)
				return true;
		}

		place = RZ_PlaceFind.Near(from, 150, 250, RZ_PlaceNeeds.Get("opening.treeline"));
		text = "the tree line where they cross";
		return place != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! A building with a real lock 40-300 m from the contact, whose key the contact has: you go in.
	protected bool Keys(RZ_Opening o, vector from, out vector place, out string text)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !hub.GetSearch() || !hub.GetDoors() || hub.GetSearch().GetKeyPrefab().IsEmpty())
			return false;

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(from, KEYS_MAX, buildings, 60);
		foreach (IEntity b : buildings)
		{
			if (!b || RZ_Util.DistXZ(from, b.GetOrigin()) < KEYS_MIN || !RZ_LockedSpots.IsLockedBuilding(b) || !hub.GetDoors().HasDoorLock(b))
				continue;

			// The tracker points at the building: never one standing in a river or a pond
			if (!RZ_PlaceCheck.Fits(b.GetOrigin(), RZ_PlaceNeeds.Get("opening.keys")))
				continue;

			string type = hub.GetSearch().BuildingType(RZ_ItemHelper.LowerName(b));
			o.m_Target = b;
			place = b.GetOrigin();
			text = "the " + KeyWord(type);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static string KeyWord(string type)
	{
		switch (type)
		{
			case "pharmacy": return "chemist's";
			case "garage": return "garage";
			case "police": return "police post";
			case "shop": return "shop";
			case "farm": return "farmhouse";
		}

		return "house";
	}

	//------------------------------------------------------------------------------------------------
	//! A Red Hand roadblock within 1.5 km, else a Red Hand camp's gate within 1.5 km; not while
	//! they hunt you.
	protected bool Toll(RZ_Opening o, vector from, out vector place, out string text)
	{
		if (RZ_Reputation.IsHunted(m_GM.GetTier(o.m_iPlayer, RZ_EFaction.BANDITS)))
			return false;

		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (posts)
		{
			array<RZ_RoadPost> list = {};
			posts.PostsNear(RZ_EFaction.BANDITS, from, NEAR, list);
			foreach (RZ_RoadPost post : list)
			{
				o.m_iTargetId = post.m_iId;
				place = post.m_vPos;
				text = "the Red Hand's toll on the road by " + post.m_sNear;
				return true;
			}
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory)
			return false;

		RZ_Holding h = territory.FindHolding(RZ_EFaction.BANDITS, from, NEAR);
		if (!h)
			return false;

		// A dry spot by the camp (a riverside camp's middle can stand in the water)
		place = RZ_PlaceFind.Near(h.m_vPos, 0, 40, RZ_PlaceNeeds.Get("opening.toll"));
		text = "the Red Hand's gate at " + h.m_sName;
		return place != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! What's left of a big one 80-250 m out: on a road's verge, else in a yard by a house (docs/PLACES.md:
	//! never an empty field). Its things are the remains (a dead special, RemainsPrefab) and the sample
	//! by it (Cure > Sample prefab), made when the step starts (SpawnSampleScene); without either slot,
	//! or once the cure is out, it isn't offered.
	protected bool Sample(vector from, out vector place, out string text)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (m_GM.GetSamplePrefab().IsEmpty() || RemainsPrefab(m_GM).IsEmpty() || (facts && facts.IsCured()))
			return false;

		place = RZ_PlaceFind.Near(from, SAMPLE_MIN, SAMPLE_MAX, RZ_PlaceNeeds.Get("opening.sample"));
		text = SAMPLE_ROAD;
		if (place != vector.Zero)
			return true;

		place = RZ_PlaceFind.Near(from, SAMPLE_MIN, SAMPLE_MAX, RZ_PlaceNeeds.Get("opening.sample.yard"));
		text = SAMPLE_YARD;
		return place != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! The place need a task's place makes (World/RZ_PlaceNeeds.c); "" for one done at the contact.
	static string NeedKey(int task, string text)
	{
		switch (task)
		{
			case RZ_EOpeningTask.WATER: return "opening.water";
			case RZ_EOpeningTask.BOILED: return "opening.water";
			case RZ_EOpeningTask.PUMP: return "opening.pump";
			case RZ_EOpeningTask.KEYS: return "opening.keys";
			case RZ_EOpeningTask.SAMPLE: return SampleNeed(text);
			case RZ_EOpeningTask.GATE: return "opening.gate";
			case RZ_EOpeningTask.DEBT: return "opening.debt";
			case RZ_EOpeningTask.SWAP: return "opening.swap";
			case RZ_EOpeningTask.TOLL: return "opening.toll";
			case RZ_EOpeningTask.DOWNWIND:
			{
				if (text.Contains("tree line"))
					return "opening.treeline";

				return "opening.downwind";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The place need A Sample's text names: by the road, or in a yard.
	static string SampleNeed(string text)
	{
		if (text == SAMPLE_YARD)
			return "opening.sample.yard";

		return "opening.sample";
	}

	//------------------------------------------------------------------------------------------------
	//! What's left of a big one: a special infected's body (Zombies > Special prefabs), one that
	//! doesn't burst when it dies (no exploder, bomber, bloater, spitter or screamer); else a common
	//! one that is a single character, not a group. "" without one: A Sample isn't offered.
	static ResourceName RemainsPrefab(RZ_GameModeComponent gm)
	{
		if (!gm || !gm.GetSpawner() || !gm.GetSpawner().GetSettings())
			return "";

		RZ_ZombieSettings zs = gm.GetSpawner().GetSettings();
		if (zs.m_aSpecialPrefabs)
		{
			foreach (ResourceName special : zs.m_aSpecialPrefabs)
			{
				if (!Bursts(special))
					return special;
			}
		}

		if (!zs.m_aCommonPrefabs)
			return "";

		foreach (ResourceName common : zs.m_aCommonPrefabs)
		{
			string name = common;
			name.ToLower();
			if (!name.Contains("group") && !Bursts(common))
				return common;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Bursts(ResourceName prefab)
	{
		string name = prefab;
		name.ToLower();
		return name.Contains("explod") || name.Contains("bomber") || name.Contains("bloater") || name.Contains("spitter") || name.Contains("scream");
	}

	//------------------------------------------------------------------------------------------------
	//! A radio and someone on the open channel to answer: Dialogue > Trees and Radio calls on.
	protected bool Channel(RZ_Opening o, IEntity me, vector from, out vector place, out string text)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		RZ_DialogueSettings ds = RZ_DialogueSettings.Get();
		if (!trees || !trees.IsOn() || !ds || !ds.m_bRadioCalls || !RZ_Radio.CarriesRadio(me))
			return false;

		place = from;
		text = "the open channel";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest wanderer on the road within 1.5 km (not held, not bitten): you find them.
	protected bool Swap(RZ_Opening o, vector from, out vector place, out string text)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (!wanderers)
			return false;

		array<int> ids = {};
		wanderers.GetLiving(RZ_Wanderers.AllRoles(), ids);
		float best = NEAR;
		foreach (int id : ids)
		{
			RZ_Wanderer w = wanderers.FindById(id);
			vector at;
			if (!w || w.IsBitten() || !wanderers.GetPosition(id, at))
				continue;

			// Never one the words can't reach: in water, on an island
			if (!RZ_PlaceCheck.Fits(at, RZ_PlaceNeeds.Get("opening.swap")))
				continue;

			float d = RZ_Util.DistXZ(at, from);
			if (d < best)
			{
				best = d;
				o.m_iTargetId = id;
				place = at;
				text = w.GetTitle();
			}
		}

		return o.m_iTargetId >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The gate: 15-25 m out from the contact.
	protected bool Gate(vector from, out vector place, out string text)
	{
		place = RZ_PlaceFind.Near(from, 15, 25, RZ_PlaceNeeds.Get("opening.gate"));
		text = "the gate";
		return place != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! A house 60-250 m off whose people owe the contact.
	protected bool Debt(vector from, out vector place, out string text)
	{
		place = RZ_PlaceFind.Near(from, 60, 250, RZ_PlaceNeeds.Get("opening.debt"));
		text = "the house with the owing in it";
		return place != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! Settled in talk: Dialogue > Trees on.
	protected bool Settle(RZ_Opening o, vector from, out vector place, out string text)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!trees || !trees.IsOn())
			return false;

		place = from;
		text = o.m_sName;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A Sample's things at its place: what's left of a big one (a special made and killed where it
	//! lies, kept from the engine's corpse cleanup until the opening lets it go, KeepBody) and the
	//! sample beside it. The place audit makes them too (RZ_PlaceAudit). The first of `made` is the body.
	void SpawnSampleScene(vector at, notnull array<IEntity> made)
	{
		vector ground = RZ_Util.GroundPos(at);
		IEntity body = RZ_Util.SpawnPrefab(RemainsPrefab(m_GM), ground + Vector(0, 0.1, 0), Math.RandomFloat(0, 360));
		if (body)
		{
			// VERIFY: a character killed the moment it's spawned on the server falls where it stands
			RZ_Util.KillCharacter(body);
			KeepBody(body, true);
			made.Insert(body);
		}

		IEntity sample = RZ_Util.SpawnPrefab(m_GM.GetSamplePrefab(), RZ_Util.RandomPointInRing(ground, 0.6, 1.2) + Vector(0, 0.3, 0), Math.RandomFloat(0, 360));
		if (!sample)
			return;

		RZ_GameModeComponent.TrackLooseItem(sample);
		made.Insert(sample);
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps a body from the engine's corpse cleanup (the garbage system), or hands it back to it.
	//! VERIFY: as RZ_TribeRest.VigilKeep, a dead character withdrawn and blacklisted stays put.
	static void KeepBody(IEntity body, bool keep)
	{
		if (!body)
			return;

		SCR_GarbageSystem garbage = SCR_GarbageSystem.GetByEntityWorld(body);
		if (!garbage)
			return;

		if (keep)
		{
			garbage.Withdraw(body);
			garbage.UpdateBlacklist(body, true);
			return;
		}

		garbage.UpdateBlacklist(body, false);
		garbage.Insert(body);
	}

	// ================================================================ While it runs
	//------------------------------------------------------------------------------------------------
	//! The step has just been handed: what it puts in the world (the key, the sample, the first
	//! noise at the gate), and the flags the conversations read.
	void Start(RZ_Opening o, IEntity me, RZ_Errand e)
	{
		o.m_bTold = false;
		o.m_bTended = false;
		o.m_fPulseAt = 0;
		switch (o.m_iTask)
		{
			case RZ_EOpeningTask.KEYS:
			{
				RZ_Interactions hub = RZ_Interactions.Get();
				if (hub && hub.GetSearch() && o.m_Target)
				{
					vector at = o.m_Target.GetOrigin();
					string type = hub.GetSearch().BuildingType(RZ_ItemHelper.LowerName(o.m_Target));
					hub.GetSearch().GiveKey(o.m_iPlayer, hub.GetSearch().KeyForBuilding(at, type), hub.GetSearch().KeyTagForBuilding(at, type));
				}

				break;
			}

			case RZ_EOpeningTask.SAMPLE:
			{
				array<IEntity> made = {};
				SpawnSampleScene(e.m_vPlace, made);
				if (!made.IsEmpty() && ChimeraCharacter.Cast(made[0]))
					o.m_Remains = made[0];

				RZ_PlaceNeeds.VerifyThings(SampleNeed(e.GetText("place")), e.m_vPlace, e.GetText("place"));
				break;
			}

			case RZ_EOpeningTask.GATE:
			{
				if (m_GM.GetHeat())
					m_GM.GetHeat().AddHeat(e.m_vPlace, GATE_HEAT);

				break;
			}

			case RZ_EOpeningTask.SETTLE: SetFlag(o, FLAG_SETTLED, 0); break;
			case RZ_EOpeningTask.CHANNEL: SetFlag(o, FLAG_ANSWERED, 0); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! May the step's clock run (RZ_ErrandDef.StepCounts)? Says once what's missing when you're at
	//! the place without it.
	bool Counts(RZ_Errands dealer, RZ_Opening o, RZ_Errand e, IEntity me)
	{
		vector at = me.GetOrigin();
		RZ_ErrandWay w = e.GetWay();
		bool there = w && w.IsAtPlace(me, e.m_vPlace);
		switch (o.m_iTask)
		{
			case RZ_EOpeningTask.PUMP:
			{
				if (RZ_FuelPumps.FindFuel(me, "jerrycan,canister,fuelcan", FUEL_LITRES))
					return true;

				if (there)
					Tell(o, e, "Fill a can at the pump first. Nothing to carry it in, and it's a long walk for nothing.");

				return false;
			}

			case RZ_EOpeningTask.KEYS:
			{
				if (!o.m_Target)
					return true;

				return RZ_DoorGeometry.InBuilding(o.m_Target, at);
			}

			case RZ_EOpeningTask.TOLL:
			{
				if (!dealer.RecentShot(e.m_vPlace, TOLL_SHOTS, TOLL_SECONDS))
					return true;

				if (there)
					Tell(o, e, "Not like this. Wait for the shooting to stop, then walk up slowly.");

				return false;
			}

			case RZ_EOpeningTask.SAMPLE:
			{
				array<IEntity> samples = {};
				RZ_LabHelper.FindSamples(me, samples);
				if (!samples.IsEmpty())
					return true;

				if (there)
					Tell(o, e, "It's by the remains. Pick it up.");

				return false;
			}

			case RZ_EOpeningTask.CHANNEL:
			{
				// Anywhere: the place follows you, and the answer is on the radio
				e.m_vPlace = at;
				return Flag(o, FLAG_ANSWERED) > 0;
			}

			case RZ_EOpeningTask.SWAP:
			{
				// They walk: the place follows them
				RZ_Wanderers wanderers = m_GM.GetWanderers();
				vector them;
				if (wanderers && o.m_iTargetId >= 0 && wanderers.GetPosition(o.m_iTargetId, them))
					e.m_vPlace = them;

				return true;
			}

			case RZ_EOpeningTask.GATE:
			{
				if (there && m_GM.GetHeat() && e.m_fProgress >= o.m_fPulseAt)
				{
					o.m_fPulseAt = e.m_fProgress + GATE_PULSE;
					m_GM.GetHeat().AddHeat(e.m_vPlace, GATE_HEAT);
				}

				return true;
			}

			case RZ_EOpeningTask.SETTLE: return Flag(o, FLAG_SETTLED) > 0;

			// Handed over in person: never just for standing next to them with a bandage in the pack
			case RZ_EOpeningTask.CUT:
			case RZ_EOpeningTask.FEVER:
			case RZ_EOpeningTask.CLOCK:
			case RZ_EOpeningTask.CLOCK_MED:
			{
				if (Tended(o))
					return true;

				if (there)
					Tell(o, e, e.Fill("Talk to {giver} and hand it over yourself."));

				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Tell(RZ_Opening o, RZ_Errand e, string text)
	{
		if (o.m_bTold)
			return;

		o.m_bTold = true;
		m_GM.NotifyPlayer(o.m_iPlayer, "Errand: " + e.Fill(e.m_Def.m_sTitle), text);
	}

	//------------------------------------------------------------------------------------------------
	//! Done: the swap's Favor (a tin for a little Haven Favor, small pay), the debt's word getting
	//! round (1 Infamy with the Haven), and the lines the contact adds.
	void Resolved(RZ_Opening o, RZ_Errand e, notnull array<string> said)
	{
		switch (o.m_iTask)
		{
			case RZ_EOpeningTask.SWAP:
			{
				IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(o.m_iPlayer);
				RZ_Errands errands = m_GM.GetErrands();
				if (me && errands && RZ_Errands.TakeItems(me, RZ_EErrandItem.FOOD, 1) > 0)
				{
					int favor = errands.SmallFavor(o.m_iPlayer, RZ_EFaction.SURVIVORS, SWAP_FAVOR);
					if (favor > 0)
						m_GM.NotifyPlayerAs(o.m_iPlayer, "Errand: " + e.Fill(e.m_Def.m_sTitle), string.Format("A tin for the road, and their word for you at the Haven.\n+%1 Haven Favor.", favor), RZ_ENoticeTag.MISSION);
				}

				break;
			}

			case RZ_EOpeningTask.DEBT:
			{
				m_GM.AddInfamy(o.m_iPlayer, RZ_EFaction.SURVIVORS, DEBT_INFAMY, "collected a debt the Red Hand way", RZ_RepFlag.SMALL);
				break;
			}

			case RZ_EOpeningTask.SETTLE:
			{
				if (Flag(o, FLAG_SETTLED) == 2)
					said.Insert(RZ_OpeningText.SettledHalf());

				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static int Flag(RZ_Opening o, string key)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!trees || !trees.GetMemory())
			return 0;

		return trees.GetMemory().CharFlag(o.m_iPlayer, key);
	}

	//------------------------------------------------------------------------------------------------
	//! Did they choose to see to them on the talk screen? With conversations off (no trees), the old
	//! way: being there with the item is enough.
	protected static bool Tended(RZ_Opening o)
	{
		return o.m_bTended;
	}

	//------------------------------------------------------------------------------------------------
	//! A step done by handing something over to the giver in person (a reply on the talk screen).
	static bool HandsOver(int task)
	{
		return task == RZ_EOpeningTask.CUT || task == RZ_EOpeningTask.FEVER || task == RZ_EOpeningTask.CLOCK || task == RZ_EOpeningTask.CLOCK_MED;
	}

	//------------------------------------------------------------------------------------------------
	//! What a hand-over step takes.
	static RZ_EErrandItem HandItem(int task)
	{
		if (task == RZ_EOpeningTask.CLOCK)
			return RZ_EErrandItem.ANTIDOTE;

		return RZ_EErrandItem.MEDICAL;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetFlag(RZ_Opening o, string key, int value)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees && trees.GetMemory())
			trees.GetMemory().SetCharFlag(o.m_iPlayer, key, value);
	}
}
