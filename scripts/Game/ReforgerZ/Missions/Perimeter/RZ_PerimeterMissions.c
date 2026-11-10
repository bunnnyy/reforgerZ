//! Missions at the Perimeter (docs/MISSIONS.md, "Perimeter missions"). No fence assets:
//! they're built from the edge rectangle in RZ_Perimeter, a package item, drop points
//! and puzzle props. Nothing crosses the line in either direction: these are jobs at
//! it, rare and dangerous on purpose. All of them turn themselves off when the Perimeter is off.

//------------------------------------------------------------------------------------------------
class RZ_PerimeterMissionHelper
{
	//------------------------------------------------------------------------------------------------
	//! The Perimeter if it's running, else null.
	static RZ_Perimeter Get(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (!perimeter || !perimeter.IsActive())
			return null;

		return perimeter;
	}

	//------------------------------------------------------------------------------------------------
	//! How far from the edge a spot at this shore point sits: `depth` metres inland of where
	//! the land starts (RZ_ShorePoint), or `minEdge` from the edge if that's further in.
	static float SpotDepth(RZ_ShorePoint p, float minEdge, float depth)
	{
		return Math.Max(minEdge, p.m_fLand + depth);
	}

	//------------------------------------------------------------------------------------------------
	//! The ground of a spot at this shore point (see SpotDepth).
	static vector SpotAt(RZ_Perimeter perimeter, RZ_ShorePoint p, float minEdge, float depth)
	{
		return perimeter.PointInside(p.m_vEdge, p.m_vInward, SpotDepth(p, minEdge, depth));
	}

	//------------------------------------------------------------------------------------------------
	//! Shore points (indexes, in the order to try them) with a dry spot (see SpotDepth), whose
	//! edge point is at least `clearance` metres from any checkpoint. Spots no more than
	//! `preferWithin` from the edge come first, then the rest, each lot shuffled: on a sea edge
	//! the coast can lie further in than the drones reach. Measuring from where the land starts
	//! is what makes these work on island maps, where the edge sits out at sea.
	static void FindSpots(RZ_Perimeter perimeter, float minEdge, float depth, float clearance, float preferWithin, notnull array<int> result)
	{
		array<int> later = {};
		int count = perimeter.GetShoreCount();
		for (int i = 0; i < count; i++)
		{
			RZ_ShorePoint p = perimeter.GetShorePoint(i);
			if (perimeter.GetCheckpointDistance(p.m_vEdge) < clearance)
				continue;

			float at = SpotDepth(p, minEdge, depth);
			if (RZ_Util.IsWater(perimeter.PointInside(p.m_vEdge, p.m_vInward, at)))
				continue;

			if (at <= preferWithin)
				result.Insert(i);
			else
				later.Insert(i);
		}

		Shuffle(result);
		Shuffle(later);
		foreach (int idx : later)
		{
			result.Insert(idx);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void Shuffle(notnull array<int> values)
	{
		for (int i = values.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			int swap = values[i];
			values[i] = values[j];
			values[j] = swap;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Does this map have any spot for the mission `key` (see FindSpots)? Worked out once and
	//! cached on the Perimeter, so a mission that can't fit this map is never drawn.
	static bool HasSpots(RZ_Perimeter perimeter, string key, float minEdge, float depth, float clearance)
	{
		int state = perimeter.GetSpotState(key);
		if (state >= 0)
			return state == 1;

		array<int> spots = {};
		FindSpots(perimeter, minEdge, depth, clearance, 0, spots);
		bool found = !spots.IsEmpty();
		NoteSpots(perimeter, key, found);
		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! Cache the answer (and log a no, once).
	static void NoteSpots(RZ_Perimeter perimeter, string key, bool found)
	{
		if (!found && perimeter.GetSpotState(key) != 0)
			RZ_Log.Info(key + ": nowhere suitable at the line on this map, so it won't be offered.");

		perimeter.SetSpotState(key, found);
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds left until dawn, -1 if unknown.
	static float NightLeft()
	{
		return RZ_GameTime.RealSecondsUntil(RZ_GameTime.NIGHT_END);
	}
}

//------------------------------------------------------------------------------------------------
//! The Blind Spot (Red Hand, Known). The drones run a rotation, and for ten minutes
//! tonight one stretch of the edge isn't covered. In that window the cordon is off in
//! that stretch only. A Red Hand runner waits with a package MEET_DEPTH metres inland of
//! where the land starts: in the kill zone where the edge runs over land (the Red Hand keeps
//! a cache there nobody else can reach), on the shore where it runs out at sea. Reach him,
//! take it, and hand it in at the Red Hand drop before the drones come back. When the
//! window closes the stretch is covered again: anyone still in the kill zone gets the usual
//! warning shot, then the fatal one. Night only.
class RZ_Mission_BlindSpot : RZ_Mission_Delivery
{
	protected vector m_vEdge;
	protected vector m_vMeet;
	protected vector m_vDrop;
	protected IEntity m_Smuggler;
	protected int m_iBlindSpot = -1;
	protected float m_fElapsed;
	protected bool m_bOpen;
	protected bool m_bClosed;
	protected bool m_bClosingWarned;

	protected static const float OPEN_IN = 600;			// real seconds until the window opens
	protected static const float WINDOW = 600;			// how long the stretch is uncovered
	protected static const float STRETCH = 250;			// radius of the uncovered stretch
	protected static const float MEET_DEPTH = 40;		// the smuggler waits this far inland of where the land starts
	protected static const float HANDOVER_RADIUS = 6;
	protected static const int MAX_TRIES = 40;
	protected static const string KEY = "The Blind Spot";

	protected bool m_bShore;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 20, 30);
		m_fTimeLeft = OPEN_IN + WINDOW + 30;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_PerimeterMissionHelper.Get(gm))
			return 0;

		if (gm.GetPerimeterPackagePrefab().IsEmpty() || gm.GetDropPointPrefab(RZ_EFaction.BANDITS).IsEmpty() || gm.GetPuzzlePrefab(RZ_EPuzzleProp.SMUGGLER).IsEmpty())
			return 0;

		// The whole window has to fall inside tonight
		if (!RZ_GameTime.IsNight())
			return 0;

		float left = RZ_PerimeterMissionHelper.NightLeft();
		if (left >= 0 && left < OPEN_IN + WINDOW)
			return 0;

		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!RZ_PerimeterMissionHelper.HasSpots(perimeter, KEY, 0, MeetDepth(perimeter), STRETCH + 400))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	protected static float MeetDepth(RZ_Perimeter perimeter)
	{
		return Math.Min(MEET_DEPTH, perimeter.GetKillDistance() * 0.5);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter)
			return false;

		// Under the drones first (the meet inside the warning zone), else anywhere on the shore
		float meetDepth = MeetDepth(perimeter);
		array<int> spots = {};
		RZ_PerimeterMissionHelper.FindSpots(perimeter, 0, meetDepth, STRETCH + 400, perimeter.GetWarnDistance(), spots);
		RZ_ShorePoint pick;
		int tries = 0;
		foreach (int idx : spots)
		{
			if (tries >= MAX_TRIES)
				break;

			tries++;
			RZ_ShorePoint p = perimeter.GetShorePoint(idx);

			// The drop is inland, a few minutes' walk from the stretch
			vector drop = perimeter.PointInside(p.m_vEdge, p.m_vInward, p.m_fLand + perimeter.GetWarnDistance() + Math.RandomFloat(200, 400));
			if (RZ_Util.IsWater(drop) || perimeter.IsNearLine(drop))
				continue;

			pick = p;
			m_vDrop = drop;
			break;
		}

		if (!pick)
		{
			if (tries == spots.Count())
				RZ_PerimeterMissionHelper.NoteSpots(perimeter, KEY, false);

			return false;
		}

		// The uncovered stretch is centred where the land starts (the edge itself on a land edge)
		m_vEdge = perimeter.PointInside(pick.m_vEdge, pick.m_vInward, pick.m_fLand);
		m_vMeet = RZ_PerimeterMissionHelper.SpotAt(perimeter, pick, 0, meetDepth);
		m_vPos = m_vMeet;
		m_bShore = pick.m_fLand >= perimeter.GetKillDistance();
		if (!SpawnDrop(gm, RZ_EFaction.BANDITS, m_vDrop))
			return false;

		string runner = "One of ours has been lying up in the kill zone since the last gap, sitting on the cache we keep where nobody else can reach it.";
		if (m_bShore)
			runner = "One of ours came in along the water at the last gap and has been lying up on the shore since, sitting on the cache we keep in the rocks.";

		m_sTitle = "The Blind Spot";
		m_sBrief = string.Format("Drones run a rotation, same as everyone. In %1 minutes the stretch of the line at grid %2 goes blind for ten. %3 Meet him, take the package, and get it to our drop at grid %4 before the drones come back. Late, or still standing there when they do? Then you were never ours.",
			RZ_Util.RoundToInt(OPEN_IN / 60), RZ_SettlementScanner.GridReference(m_vMeet), runner, RZ_SettlementScanner.GridReference(m_vDrop));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return "the quarantine line, grid " + RZ_SettlementScanner.GridReference(m_vMeet);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fElapsed += dt;
		if (!m_bOpen && m_fElapsed >= OPEN_IN)
			OpenWindow(gm);

		if (!m_bOpen || m_bClosed)
			return;

		if (!m_bItemSpawned)
			TryHandover(gm, ids, players);

		float left = OPEN_IN + WINDOW - m_fElapsed;
		if (!m_bClosingWarned && left <= 120)
		{
			m_bClosingWarned = true;
			NotifyInterested(gm, m_sTitle, "Two minutes until the drones are back over the line.");
		}

		if (left <= 0)
			CloseWindow(gm);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenWindow(RZ_GameModeComponent gm)
	{
		m_bOpen = true;
		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (perimeter)
			m_iBlindSpot = perimeter.AddBlindSpot(m_vEdge, STRETCH);

		m_Smuggler = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(RZ_EPuzzleProp.SMUGGLER), m_vMeet, RZ_Compass.Bearing(m_vMeet, m_vEdge) + 180);
		NotifyInterested(gm, m_sTitle, string.Format("The drones over grid %1 have gone quiet. Ten minutes. Move.", RZ_SettlementScanner.GridReference(m_vMeet)));
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever reaches the smuggler first gets the package, hired or not.
	protected void TryHandover(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vMeet) > HANDOVER_RADIUS)
				continue;

			if (!SpawnItem(gm, RZ_Util.RandomPointInRing(m_vMeet, 0.5, 1), gm.GetPerimeterPackagePrefab()))
				return;

			if (!m_aParticipants.Contains(ids[i]))
				m_aParticipants.Insert(ids[i]);

			string away = "crawls back into the brush along the line";
			if (m_bShore)
				away = "slips back down among the rocks at the waterline";

			gm.NotifyPlayer(ids[i], m_sTitle, string.Format("He doesn't say a word. He sets a sealed package at your feet and %1, to wait for the next gap.", away));
			RZ_Util.DeleteUnit(m_Smuggler);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseWindow(RZ_GameModeComponent gm)
	{
		m_bClosed = true;
		EndBlindSpot(gm);
		RZ_Util.DeleteUnit(m_Smuggler);
		NotifyInterested(gm, m_sTitle, "The drones are back over the line.");
		if (!m_bDelivered)
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected void EndBlindSpot(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (perimeter && m_iBlindSpot >= 0)
			perimeter.RemoveBlindSpot(m_iBlindSpot);

		m_iBlindSpot = -1;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The package reached the Red Hand before the drones came back.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The drones came back before the package reached the Red Hand. That window won't open again.";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		EndBlindSpot(gm);
		RZ_Util.DeleteUnit(m_Smuggler);
	}
}

//------------------------------------------------------------------------------------------------
//! The Crossing (Haven, Any). At night: a Haven runner carrying medicine along the edge
//! (the one road nobody tolls) was found by a drone and dropped the sealed package just
//! outside the kill zone: at the treeline where the edge runs over land, at the waterline
//! where it runs out at sea (inside the warning zone where the coast comes that close, else
//! just past the drones' reach). Grab it and get out of the warning zone before the
//! drone "locks on": the carrier may spend LOCK_SECONDS in total in the warning zone
//! with it. Lock-on: one shot destroys the package (not the carrier) and the mission
//! fails. Deliver it to the Haven drop at the safehouse in the least infected town (with no
//! safehouses, a drop in the safest town).
class RZ_Mission_TheCrossing : RZ_Mission_Delivery
{
	protected float m_fExposed;
	protected bool m_bHalfWarned;
	protected string m_sSpotKind = "treeline";
	protected bool m_bUnderDrones = true;

	protected static const float LOCK_SECONDS = 45;
	protected static const float CLEARANCE = 500;	// from any checkpoint
	protected static const string KEY = "The Crossing";

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 15, 20);
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_PerimeterMissionHelper.Get(gm))
			return 0;

		if (gm.GetPerimeterPackagePrefab().IsEmpty() || gm.GetDropPointPrefab(RZ_EFaction.SURVIVORS).IsEmpty())
			return 0;

		if (!RZ_GameTime.IsNight())
			return 0;

		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (NearDepth(perimeter) >= FarDepth(perimeter))
			return 0;

		if (!RZ_PerimeterMissionHelper.HasSpots(perimeter, KEY, NearDepth(perimeter), 0, CLEARANCE))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	//! The package is at least this far from the edge (clear of the kill zone)...
	protected static float NearDepth(RZ_Perimeter perimeter)
	{
		return perimeter.GetKillDistance() + 30;
	}

	//------------------------------------------------------------------------------------------------
	//! ...and, where the land allows, no further than this (well inside the warning zone).
	protected static float FarDepth(RZ_Perimeter perimeter)
	{
		return perimeter.GetWarnDistance() - 60;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter)
			return false;

		vector spot;
		if (!FindPackageSpot(perimeter, spot))
			return false;

		// A real Haven safehouse when there is one; otherwise the safest town
		RZ_Safehouse house = RZ_ContractHelp.Safehouse(gm, RZ_SafehousePick.Safest(gm, false));
		int town = -1;
		vector dropAt;
		if (house)
			town = house.m_iSettlement;
		else
			town = PickSafestSettlement(gm);

		RZ_Settlement s = gm.GetSettlement(town);
		if (!s)
			return false;

		if (house)
			dropAt = LandPointNear(house.m_vPos, 6, 14);
		else
			dropAt = LandPointNear(s.m_vCenter, 10, Math.Max(20, s.m_fRadius * 0.5));

		if (!SpawnDrop(gm, RZ_EFaction.SURVIVORS, dropAt))
			return false;

		string destination = "our people in " + s.m_sName;
		if (house)
			destination = "our safehouse in " + s.m_sName;

		if (!SpawnItem(gm, spot, gm.GetPerimeterPackagePrefab()))
			return false;

		m_vPos = spot;

		// Until dawn (at most two hours), or 40 minutes if the clock speed isn't known yet
		float left = RZ_PerimeterMissionHelper.NightLeft();
		if (left < 0)
			m_fTimeLeft = 2400;
		else
			m_fTimeLeft = Math.Clamp(left, 900, 7200);

		m_sTitle = "The Crossing";
		string under = "right under the drones";
		if (!m_bUnderDrones)
			under = "just short of where the drones fly";

		m_sBrief = string.Format("One of our runners was carrying medicine along the line tonight, the only road the Red Hand doesn't toll. A drone found her. She dropped the package at the %1 at grid %2, %3, and ran. She made it. The medicine didn't. Take it and get out of the warning zone quickly: if you carry it too long under a drone, it locks on. Please bring it to %4. And be careful.",
			m_sSpotKind, RZ_SettlementScanner.GridReference(spot), under, destination);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The first dry ground from where the land starts, clear of the kill zone. Spots inside the
	//! warning zone come first; on a sea edge whose coast lies further in, the waterline
	//! there will do. Water just before the spot means it's the waterline, otherwise the treeline.
	protected bool FindPackageSpot(RZ_Perimeter perimeter, out vector spot)
	{
		float nearDepth = NearDepth(perimeter);
		float farDepth = FarDepth(perimeter);
		if (farDepth <= nearDepth)
			return false;

		array<int> spots = {};
		RZ_PerimeterMissionHelper.FindSpots(perimeter, nearDepth, 0, CLEARANCE, farDepth, spots);
		if (spots.IsEmpty())
		{
			RZ_PerimeterMissionHelper.NoteSpots(perimeter, KEY, false);
			return false;
		}

		RZ_ShorePoint p = perimeter.GetShorePoint(spots[0]);
		float at = RZ_PerimeterMissionHelper.SpotDepth(p, nearDepth, 0);
		spot = perimeter.PointInside(p.m_vEdge, p.m_vInward, at);
		m_bUnderDrones = at <= farDepth;
		if (p.m_fLand > 0 && p.m_fLand + 30 >= at)
			m_sSpotKind = "waterline";
		else
			m_sSpotKind = "treeline";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return "the quarantine line, grid " + RZ_SettlementScanner.GridReference(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Item || m_bDelivered)
			return;

		int carrier = RZ_Util.GetCarrierPlayerId(m_Item);
		if (carrier <= 0)
			return;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(carrier);
		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (!ent || !perimeter || perimeter.GetZone(ent.GetOrigin()) == RZ_EPerimeterZone.NONE)
			return;

		m_fExposed += dt;
		if (!m_bHalfWarned && m_fExposed >= LOCK_SECONDS * 0.5)
		{
			m_bHalfWarned = true;
			gm.NotifyPlayer(carrier, RZ_Perimeter.LOUDSPEAKER, "The drone's light swings onto you and stays there. It's tracking the package. Get out of the warning zone.");
		}

		if (m_fExposed < LOCK_SECONDS)
			return;

		// Locked on: one shot, at the package
		perimeter.SpawnEffect(ent.GetOrigin());
		RZ_MissionItemHelper.RemoveItem(ent, m_Item);
		m_Item = null;
		gm.NotifyPlayer(carrier, RZ_Perimeter.LOUDSPEAKER, "\"Contraband identified.\" A single shot. The package bursts open against you. You aren't hit. The medicine is gone.");
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The medicine from the line reached the Haven.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The medicine from the line never reached the Haven.";
	}
}

//------------------------------------------------------------------------------------------------
//! Shot at the Line (any faction, Any). A body lies in the warning zone, just short of
//! the kill zone, with papers on them. Read them there (no need to cross into the kill
//! zone); they say where the dead person left what they couldn't carry. The stash is
//! inland: the reward crate is found there. Where the edge runs out at sea, the kill zone
//! is water: the body has washed up on the shore, BODY_INLAND metres above where the land
//! starts (inside the warning zone where the coast comes that close).
class RZ_Mission_ShotAtTheLine : RZ_PuzzleMission
{
	protected vector m_vBody;
	protected vector m_vStash;
	protected bool m_bRead;
	protected bool m_bShore;

	protected static const float BODY_INLAND = 10;
	protected static const float CLEARANCE = 500;	// from any checkpoint
	protected static const int MAX_TRIES = 40;
	protected static const string KEY = "Shot at the Line";

	//------------------------------------------------------------------------------------------------
	//! Each time this mission is drawn, a different faction wants to know who it was.
	override void Configure(RZ_GameModeComponent gm)
	{
		RZ_EFaction giver = Math.RandomInt(0, RZ_Factions.COUNT);
		SetContract(giver, RZ_ETier.STRANGER, 12, 15);
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter || !HasPrefab(gm, RZ_EPuzzleProp.LINE_BODY))
			return 0;

		if (!RZ_PerimeterMissionHelper.HasSpots(perimeter, KEY, perimeter.GetKillDistance() + 35, BODY_INLAND, CLEARANCE))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter)
			return false;

		// Close enough to the kill zone to make the point, far enough to reach safely
		float depth = perimeter.GetKillDistance() + m_Rng.Range(20, 50);
		if (depth > perimeter.GetWarnDistance() - 30)
			depth = (perimeter.GetKillDistance() + perimeter.GetWarnDistance()) * 0.5;

		// Body in the warning zone first, else (a far coast) on the shore wherever it is
		array<int> spots = {};
		RZ_PerimeterMissionHelper.FindSpots(perimeter, depth, BODY_INLAND, CLEARANCE, perimeter.GetWarnDistance() - 30, spots);
		bool found = false;
		int tries = 0;
		foreach (int idx : spots)
		{
			if (tries >= MAX_TRIES)
				break;

			tries++;
			RZ_ShorePoint p = perimeter.GetShorePoint(idx);
			m_vBody = RZ_PerimeterMissionHelper.SpotAt(perimeter, p, depth, BODY_INLAND);

			// The stash: a town inland, or the countryside behind the line
			array<int> towns = {};
			CollectSettlements(gm, m_vBody, 400, 3000, null, towns);
			RZ_Settlement s = null;
			if (!towns.IsEmpty())
				s = gm.GetSettlement(towns[0]);

			if (s)
				m_vStash = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius + 150);
			else
				m_vStash = LandPointNear(perimeter.PointInside(p.m_vEdge, p.m_vInward, p.m_fLand + perimeter.GetWarnDistance() + 600), 0, 150);

			// The letter gives a grid square: the stash sits in its middle
			m_vStash = RZ_HiddenStash.GridSquareCentre(m_vStash);
			if (RZ_Util.IsWater(m_vStash) || perimeter.IsNearLine(m_vStash))
				continue;

			m_bShore = p.m_fLand + BODY_INLAND >= depth;
			found = true;
			break;
		}

		if (!found)
		{
			if (tries == spots.Count())
				RZ_PerimeterMissionHelper.NoteSpots(perimeter, KEY, false);

			return false;
		}

		m_vPos = m_vBody;

		string name = RZ_NoteText.FullName(m_Rng);
		string first = RZ_NoteText.FirstName(m_Rng);
		int papers = AddClue("Papers on the body", string.Format("In an inside pocket, folded small: a travel permit stamped two days before the gates closed, no good to anyone now. It says %1. With it, a letter:\n\"%2, if this doesn't work, I left what I couldn't carry at %3. Use it. Don't try the line. I was wrong about the drones.\"\n%4",
			name, first, PlaceText(gm, m_vStash), RZ_NoteText.Signoff(m_Rng, name)));

		IEntity body = SpawnClue(gm, RZ_EPuzzleProp.LINE_BODY, m_vBody, papers, "Search the body");
		if (!body)
			return false;

		m_sTitle = "Shot at the Line";
		if (m_bShore)
			m_sBrief = GetShoreBriefFor(m_eGiver, RZ_SettlementScanner.GridReference(m_vBody));
		else
			m_sBrief = GetBriefFor(m_eGiver, RZ_SettlementScanner.GridReference(m_vBody));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the line runs out at sea: they tried it by water and washed up on the shore.
	protected string GetShoreBriefFor(RZ_EFaction giver, string grid)
	{
		switch (giver)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("Someone tried to get out by water last night. They didn't make it. The sea brought them back to the shore at grid %1. Please bring back whatever they were carrying, so we know who it was. Don't go out past them: the drones watch the water.", grid);
			case RZ_EFaction.MILITARY: return string.Format("Unidentified civilian, shot attempting the line by sea. Body washed up on the shore, grid %1. Recover any papers. Do not enter the water. Kill zone is offshore.", grid);
			case RZ_EFaction.BANDITS: return string.Format("Some genius tried to swim the line last night. Guess how that went. The tide dropped him back on the shore at grid %1, pockets still full. Go through them. Stay out of the water, yeah?", grid);
			case RZ_EFaction.INSTITUTE: return string.Format("A body has washed up on the shore at grid %1: someone shot trying the line by water. We need to know who they were and what they were carrying. Recover their papers. Stay out of the water: the kill zone is offshore.", grid);
		}

		return string.Format("A body has washed up on the shore at grid %1, shot trying the line by water. Recover their papers without going out into the kill zone.", grid);
	}

	//------------------------------------------------------------------------------------------------
	//! In the giver's voice (docs/LORE.md, "Faction voices").
	protected string GetBriefFor(RZ_EFaction giver, string grid)
	{
		switch (giver)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("Someone tried to cross last night. They didn't make it. Their body is still in the warning zone at grid %1. Please bring back whatever they were carrying, so we know who it was. Don't go past them: the shooting starts just beyond.", grid);
			case RZ_EFaction.MILITARY: return string.Format("Unidentified civilian, shot at the line, grid %1. Body is inside the warning zone. Recover any papers. Do not proceed beyond the body. Kill zone starts there.", grid);
			case RZ_EFaction.BANDITS: return string.Format("Some genius tried the line last night. Guess how that went. The body's in the warning zone at grid %1, pockets still full. Go through them. Just stop where he stopped, yeah?", grid);
			case RZ_EFaction.INSTITUTE: return string.Format("A body lies in the warning zone at grid %1, shot at the line. We need to know who they were and what they were carrying. Recover their papers. Stay short of the kill zone: it begins just past the body.", grid);
		}

		return string.Format("A body lies in the warning zone at grid %1, shot at the line. Recover their papers without crossing into the kill zone.", grid);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_bRead = true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return "the quarantine line, grid " + RZ_SettlementScanner.GridReference(m_vBody);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bRead && ReachedBy(ids, players, m_vStash, RZ_HiddenStash.FIND_RADIUS))
		{
			m_vStash = RZ_HiddenStash.FoundAt(players, m_vStash);
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vStash);
		NotifyParticipants(gm, m_sTitle, "It's where the letter said. Wrapped in plastic, waiting for someone who's never coming back for it.");
	}
}
