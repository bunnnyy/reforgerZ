//! Missions at the Perimeter (docs/MISSIONS.md, "Perimeter missions"). No fence assets:
//! they're built from the edge rectangle in RZ_Perimeter, a package item, drop points
//! and puzzle props. They're the only links to the outside, so they're rare and
//! dangerous on purpose. All of them turn themselves off when the Perimeter is off.

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
	//! A random edge point whose ground `depth` metres inside the line is dry, at least
	//! `clearance` metres from any checkpoint. False after a few tries.
	static bool FindEdgeSpot(RZ_Perimeter perimeter, float depth, float clearance, out vector edge, out vector inward)
	{
		for (int i = 0; i < 25; i++)
		{
			if (!perimeter.RandomEdgePoint(edge, inward))
				return false;

			if (perimeter.GetCheckpointDistance(edge) < clearance)
				continue;

			if (RZ_Util.IsWater(perimeter.PointInside(edge, inward, depth)))
				continue;

			return true;
		}

		return false;
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
//! that stretch only. A smuggler waits just inside the line (in the kill zone) with a
//! package; reach him, take it, and hand it in at the Red Hand drop before the drones
//! come back. When the window closes the stretch is covered again: anyone still in the
//! kill zone gets the usual warning shot, then the fatal one. Night only.
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
	protected static const float MEET_DEPTH = 40;		// the smuggler waits this far inside the line
	protected static const float HANDOVER_RADIUS = 6;

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

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter)
			return false;

		float meetDepth = Math.Min(MEET_DEPTH, perimeter.GetKillDistance() * 0.5);
		vector edge, inward;
		bool found = false;
		for (int i = 0; i < 6 && !found; i++)
		{
			if (!RZ_PerimeterMissionHelper.FindEdgeSpot(perimeter, meetDepth, STRETCH + 400, edge, inward))
				return false;

			// The drop is inland, a few minutes' walk from the stretch
			m_vDrop = perimeter.PointInside(edge, inward, perimeter.GetWarnDistance() + Math.RandomFloat(200, 400));
			if (!RZ_Util.IsWater(m_vDrop))
				found = true;
		}

		if (!found)
			return false;

		m_vEdge = edge;
		m_vMeet = perimeter.PointInside(edge, inward, meetDepth);
		m_vPos = m_vMeet;
		if (!SpawnDrop(gm, RZ_EFaction.BANDITS, m_vDrop))
			return false;

		m_sTitle = "The blind spot";
		m_sBrief = string.Format("Drones run a rotation, same as everyone. In %1 minutes the stretch of the line at grid %2 goes blind for ten. A friend from the other side waits just inside the line with a package. Get it to our drop at grid %3 before the drones come back. Late, or still standing there when they do? Then you were never ours.",
			RZ_Util.RoundToInt(OPEN_IN / 60), RZ_SettlementScanner.GridReference(m_vMeet), RZ_SettlementScanner.GridReference(m_vDrop));
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

			gm.NotifyPlayer(ids[i], m_sTitle, "He doesn't say a word. He sets a sealed package at your feet and walks back over the line, into the dark.");
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
//! The Crossing (Haven, Any). At night, someone outside leaves a sealed medicine
//! package right at the edge, inside the warning zone: at the waterline where the edge
//! is coast, else at the treeline. Grab it and get out of the warning zone before the
//! drone "locks on": the carrier may spend LOCK_SECONDS in total in the warning zone
//! with it. Lock-on: one shot destroys the package (not the carrier) and the mission
//! fails. Deliver it to the Haven drop in the safest town.
class RZ_Mission_TheCrossing : RZ_Mission_Delivery
{
	protected float m_fExposed;
	protected bool m_bHalfWarned;
	protected string m_sSpotKind = "treeline";

	protected static const float LOCK_SECONDS = 45;

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

		return 0.6;
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

		int safehouse = PickSafestSettlement(gm);
		RZ_Settlement s = gm.GetSettlement(safehouse);
		if (!s)
			return false;

		if (!SpawnDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(s.m_vCenter, 10, Math.Max(20, s.m_fRadius * 0.5))))
			return false;

		if (!SpawnItem(gm, spot, gm.GetPerimeterPackagePrefab()))
			return false;

		m_vPos = spot;

		// Until dawn (at most two hours), or 40 minutes if the clock speed isn't known yet
		float left = RZ_PerimeterMissionHelper.NightLeft();
		if (left < 0)
			m_fTimeLeft = 2400;
		else
			m_fTimeLeft = Math.Clamp(left, 900, 7200);

		m_sTitle = "The crossing";
		m_sBrief = string.Format("Someone on the outside is still trying to help. Tonight they left a sealed medicine package at the %1 at grid %2, right under the drones. Take it and get out of the warning zone quickly: if you carry it too long under a drone, it locks on. Please bring it to our safehouse in %3. And be careful.",
			m_sSpotKind, RZ_SettlementScanner.GridReference(spot), s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Walk in from a random edge point to the first dry ground within the warning zone (and
	//! outside the kill zone). Water first means it's the waterline, otherwise the treeline.
	protected bool FindPackageSpot(RZ_Perimeter perimeter, out vector spot)
	{
		float nearDepth = perimeter.GetKillDistance() + 30;
		float farDepth = perimeter.GetWarnDistance() - 60;
		if (farDepth <= nearDepth)
			return false;

		for (int tries = 0; tries < 20; tries++)
		{
			vector edge, inward;
			if (!perimeter.RandomEdgePoint(edge, inward))
				return false;

			if (perimeter.GetCheckpointDistance(edge) < 500)
				continue;

			bool sawWater = false;
			for (float depth = nearDepth; depth <= farDepth; depth += 10)
			{
				vector p = perimeter.PointInside(edge, inward, depth);
				if (RZ_Util.IsWater(p))
				{
					sawWater = true;
					continue;
				}

				spot = p;
				if (sawWater)
					m_sSpotKind = "waterline";
				else
					m_sSpotKind = "treeline";

				return true;
			}
		}

		return false;
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
		return "The medicine from across the line reached the Haven.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The medicine from across the line never reached the Haven.";
	}
}

//------------------------------------------------------------------------------------------------
//! Shot at the Line (any faction, Any). A body lies in the warning zone, just short of
//! the kill zone, with papers on them. Read them there (no need to cross into the kill
//! zone); they say where the dead person left what they couldn't carry. The stash is
//! inland: the reward crate is found there.
class RZ_Mission_ShotAtTheLine : RZ_PuzzleMission
{
	protected vector m_vBody;
	protected vector m_vStash;
	protected bool m_bRead;

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
		if (!RZ_PerimeterMissionHelper.Get(gm) || !HasPrefab(gm, RZ_EPuzzleProp.LINE_BODY))
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

		vector edge, inward;
		if (!RZ_PerimeterMissionHelper.FindEdgeSpot(perimeter, depth, 500, edge, inward))
			return false;

		m_vBody = perimeter.PointInside(edge, inward, depth);
		m_vPos = m_vBody;

		// The stash: a town inland, or the countryside behind the line
		array<int> towns = {};
		CollectSettlements(gm, m_vBody, 400, 3000, null, towns);
		RZ_Settlement s;
		if (!towns.IsEmpty())
			s = gm.GetSettlement(towns[0]);

		if (s)
			m_vStash = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius + 150);
		else
			m_vStash = LandPointNear(perimeter.PointInside(edge, inward, perimeter.GetWarnDistance() + 600), 0, 150);

		if (RZ_Util.IsWater(m_vStash) || perimeter.IsNearLine(m_vStash))
			return false;

		string name = RZ_NoteText.FullName(m_Rng);
		string first = RZ_NoteText.FirstName(m_Rng);
		int papers = AddClue("Papers on the body", string.Format("In an inside pocket, folded small: a travel permit stamped two days before the gates closed, no good to anyone now. It says %1. With it, a letter:\n\"%2, if this doesn't work, I left what I couldn't carry at %3. Use it. Don't try the line. I was wrong about the drones.\"\n%4",
			name, first, PlaceText(gm, m_vStash), RZ_NoteText.Signoff(m_Rng, name)));

		IEntity body = SpawnClue(gm, RZ_EPuzzleProp.LINE_BODY, m_vBody, papers, "Search the body");
		if (!body)
			return false;

		m_sTitle = "Shot at the line";
		m_sBrief = GetBriefFor(m_eGiver, RZ_SettlementScanner.GridReference(m_vBody));
		return true;
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
			case RZ_EFaction.INSTITUTE: return string.Format("A body lies in the warning zone at grid %1, shot at the line. We need to know who they were and what they carried out of the Zone. Recover their papers. Stay short of the kill zone: it begins just past the body.", grid);
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
		if (m_bRead && ReachedBy(ids, players, m_vStash, 10))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vStash);
		NotifyParticipants(gm, m_sTitle, "It's where the letter said. Wrapped in plastic, waiting for someone who's never coming back for it.");
	}
}
