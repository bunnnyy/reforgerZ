//! Reusable building blocks for mission types. Each concrete mission picks a
//! base, sets its numbers in Configure()/Setup(), and overrides a hook or two.

//------------------------------------------------------------------------------------------------
//! Hold an area: at least one player within m_fHoldRadius for m_fHoldNeeded
//! seconds in total. Optional heat while held, optional waves of hordes.
class RZ_Mission_Hold : RZ_Mission
{
	protected float m_fHoldNeeded = 60;
	protected float m_fHoldRadius = 25;
	protected float m_fHeld;
	protected float m_fHeatPerSecond;
	protected float m_fBonusHoldSpeed = 1;	// ★ background near: hold time runs this much faster
	protected float m_fWaveInterval;		// 0 = no waves
	protected int m_iMaxWaves;
	protected float m_fWaveDistance = 250;
	protected float m_fWaveTimer;
	protected int m_iWaves;
	protected bool m_bStarted;
	protected bool m_bAnnounceProgress = true;
	protected int m_iProgressStep;

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!CanHold(gm))
			return;

		if (CountPlayersNear(players, m_vPos, m_fHoldRadius) == 0)
			return;

		if (!m_bStarted)
		{
			m_bStarted = true;
			OnHoldStarted(gm);
		}

		float rate = 1;
		if (m_fBonusHoldSpeed != 1 && AnyBonusNear(gm, ids, players, m_vPos, m_fHoldRadius))
			rate = m_fBonusHoldSpeed;

		m_fHeld += dt * rate;
		if (m_fHeatPerSecond > 0)
			gm.GetHeat().AddHeat(m_vPos, m_fHeatPerSecond * dt);

		if (m_fWaveInterval > 0 && m_iWaves < m_iMaxWaves)
		{
			m_fWaveTimer += dt;
			if (m_fWaveTimer >= m_fWaveInterval)
			{
				m_fWaveTimer = 0;
				m_iWaves++;
				gm.GetSpawner().SpawnHorde(m_vPos, m_fWaveDistance, m_iId);
				OnWave(gm, m_iWaves);
			}
		}

		OnHoldTick(gm, dt, ids, players);

		// "Halfway there" style progress, once per quarter
		if (m_bAnnounceProgress && m_fHoldNeeded >= 120)
		{
			int step = Math.Floor(4 * m_fHeld / m_fHoldNeeded);
			if (step > m_iProgressStep && step < 4)
			{
				m_iProgressStep = step;
				NotifyParticipants(gm, m_sTitle, string.Format("Holding: %1 percent.", step * 25));
			}
		}

		if (m_fHeld >= m_fHoldNeeded && m_eState == RZ_EMissionState.ACTIVE)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! The ground to hold, within its radius (the rough area until you know the spot).
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (IsRoughFor(gm, playerId, from))
			return AreaPlace(gm, playerId, from);

		return RZ_ObjPlace.Spot(m_vPos, m_fHoldRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! Override to pause holding (e.g. until something else happened).
	protected bool CanHold(RZ_GameModeComponent gm)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoldStarted(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoldTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWave(RZ_GameModeComponent gm, int wave)
	{
	}
}

//------------------------------------------------------------------------------------------------
//! Kill everything the mission spawned (tagged with its id). Works with zombies
//! and with AI groups spawned through the spawner.
class RZ_Mission_KillTagged : RZ_Mission
{
	protected bool m_bSpawned;
	protected float m_fGrace = 10;	// groups spawn their members a frame later; wait before checking

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSpawned)
			return;

		m_fGrace -= dt;
		if (m_fGrace > 0)
			return;

		if (gm.GetSpawner().CountForMission(m_iId) == 0 && AllTargetsDead(gm))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! Extra targets that aren't spawner zombies (e.g. an AI group). Default: none.
	protected bool AllTargetsDead(RZ_GameModeComponent gm)
	{
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Visit a set of points. Each point needs a player at it (RZ_Arrive: m_fVisitRadius, widened to the
//! Arrive radius) for m_fVisitSeconds (0 = just reach it).
class RZ_Mission_VisitPoints : RZ_Mission
{
	protected ref array<vector> m_aPoints = {};
	protected ref array<float> m_aDwell = {};
	protected ref array<bool> m_aDone = {};
	protected float m_fVisitRadius = 20;
	protected float m_fVisitSeconds;
	protected float m_fBonusVisitSpeed = 1;
	protected int m_iVisited;
	protected string m_sPointName = "point";
	protected bool m_bHandsOn;		// the points are things you work at (a corpse pile): m_fVisitRadius as it is
	// Work at each point (docs/DESIGN.md, Work site): a hold prompt there, not a stay; off: the stay
	protected RZ_EWork m_eWork = RZ_EWork.NONE;
	protected string m_sWorkLabel;
	protected float m_fWorkHold;
	protected ResourceName m_sWorkBefore;	// this mission's own before look (empty: the kind's)
	protected ResourceName m_sWorkAfter;

	//------------------------------------------------------------------------------------------------
	//! The points are work you do with your hands at a thing there (with work sites on).
	protected void PointWork(RZ_EWork kind, string label, float hold)
	{
		m_eWork = kind;
		m_sWorkLabel = label;
		m_fWorkHold = hold;
	}

	//------------------------------------------------------------------------------------------------
	protected string PointKey(int idx)
	{
		return string.Format("mission.%1.point.%2", m_iId, idx);
	}

	//------------------------------------------------------------------------------------------------
	//! The point is held at a work site (open now, or opened here). False: work sites are off.
	protected bool PointWorked(RZ_GameModeComponent gm, int idx, array<int> ids, array<IEntity> players)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (m_eWork == RZ_EWork.NONE || !sites || !RZ_WorkSites.Available())
			return false;

		string key = PointKey(idx);
		if (!sites.Has(key))
		{
			if (sites.Open(key, m_eWork, m_sWorkLabel, m_fWorkHold, m_aPoints[idx], 0, m_sWorkBefore, m_sWorkAfter) < 0)
				return false;

			// The site may stand a little off a point on a slope or the shore (RZ_WorkSites.Steady):
			// the point follows it, so the marker and who's at it are where the work is
			vector at = sites.PosOf(key);
			if (at != vector.Zero)
				m_aPoints[idx] = at;
		}

		// Whoever is at it takes part; a background's bonus holds faster
		array<int> near = {};
		GetPlayersNear(ids, players, m_aPoints[idx], 6, near);
		foreach (int playerId : near)
		{
			if (!m_aParticipants.Contains(playerId))
				m_aParticipants.Insert(playerId);

			if (m_fBonusVisitSpeed > 1 && HasBonus(gm, playerId))
				sites.SetFast(key, playerId, m_fBonusVisitSpeed);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Its work sites go with the mission (a done one stays until nobody is near).
	protected void CloseWork()
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites)
			return;

		foreach (int i, vector p : m_aPoints)
		{
			sites.Close(PointKey(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		CloseWork();
		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! How close counts at a point.
	protected float VisitRadius()
	{
		if (m_bHandsOn)
			return m_fVisitRadius;

		return RZ_Arrive.Spot(m_fVisitRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! The points to visit are the job: their grids are given: never a rough area.
	override bool NeedsExactLocation()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The next point to visit: the nearest one not done yet (their grids are given).
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		int best = NextPoint(from);
		if (best < 0)
			return super.ObjectivePlace(gm, playerId, from);

		return RZ_ObjPlace.Spot(m_aPoints[best], VisitRadius());
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest point not done yet from `from` (the first without a position), -1 for none.
	int NextPoint(vector from)
	{
		int best = -1;
		float bestDist = 0;
		foreach (int i, vector p : m_aPoints)
		{
			if (m_aDone[i])
				continue;

			if (from == vector.Zero)
				return i;

			float d = RZ_Util.DistXZ(from, p);
			if (best < 0 || d < bestDist)
			{
				best = i;
				bestDist = d;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The points not done yet, and how close counts (the runtime test's check).
	float GetOpenPoints(notnull array<vector> result)
	{
		foreach (int i, vector p : m_aPoints)
		{
			if (!m_aDone[i])
				result.Insert(p);
		}

		return VisitRadius();
	}

	//------------------------------------------------------------------------------------------------
	protected void AddPoint(vector p)
	{
		m_aPoints.Insert(p);
		m_aDwell.Insert(0);
		m_aDone.Insert(false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		float radius = VisitRadius();
		foreach (int i, vector p : m_aPoints)
		{
			if (m_aDone[i])
				continue;

			// Work at it: done when its hold is done
			if (PointWorked(gm, i, ids, players))
			{
				if (!RZ_WorkSites.Get().IsDone(PointKey(i)))
					continue;

				m_aDone[i] = true;
				m_iVisited++;
				OnPointDone(gm, i);
				if (m_iVisited < m_aPoints.Count())
					NotifyParticipants(gm, m_sTitle, string.Format("%1 %2 of %3 done.", m_sPointName, m_iVisited, m_aPoints.Count()));

				continue;
			}

			// Anyone at a point takes part, even if it's far from the mission centre
			array<int> near = {};
			if (m_bHandsOn)
				GetPlayersNear(ids, players, p, radius, near);
			else
				RZ_Arrive.GetAtSpot(ids, players, p, radius, near);

			if (near.IsEmpty())
				continue;

			foreach (int playerId : near)
			{
				if (!m_aParticipants.Contains(playerId))
					m_aParticipants.Insert(playerId);
			}

			float rate = 1;
			if (m_fBonusVisitSpeed != 1 && AnyBonusNear(gm, ids, players, p, radius))
				rate = m_fBonusVisitSpeed;

			m_aDwell[i] = m_aDwell[i] + dt * rate;
			if (m_aDwell[i] < m_fVisitSeconds)
				continue;

			m_aDone[i] = true;
			m_iVisited++;
			OnPointDone(gm, i);
			if (m_iVisited < m_aPoints.Count())
				NotifyParticipants(gm, m_sTitle, string.Format("%1 %2 of %3 done.", m_sPointName, m_iVisited, m_aPoints.Count()));
		}

		if (m_iVisited >= m_aPoints.Count() && !m_aPoints.IsEmpty() && m_eState == RZ_EMissionState.ACTIVE)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		foreach (vector p : m_aPoints)
		{
			result.Insert(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPointDone(RZ_GameModeComponent gm, int idx)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! "Morton (1.2 km NE), Lamentin (800 m S)" from the player at `from`.
	string DescribePoints(RZ_GameModeComponent gm, vector from)
	{
		string text = "";
		foreach (int i, vector p : m_aPoints)
		{
			if (i > 0)
				text += ", ";

			text += RZ_SettlementScanner.GridReference(p);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nPoints (grid): " + DescribePoints(gm, from);
	}
}

//------------------------------------------------------------------------------------------------
//! Destroy a spawned prop (nest, fuel depot, relay). Needs a damage manager on the prefab.
class RZ_Mission_DestroyTarget : RZ_Mission
{
	protected IEntity m_Target;

	//------------------------------------------------------------------------------------------------
	protected bool SpawnTarget(ResourceName prefab, vector pos)
	{
		m_Target = RZ_Util.SpawnPrefab(prefab, pos, Math.RandomFloat(0, 360));
		return m_Target != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Target || IsDestroyed(m_Target))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! The target itself (the rough area until you know the spot).
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (!m_Target || IsRoughFor(gm, playerId, from))
			return super.ObjectivePlace(gm, playerId, from);

		return RZ_ObjPlace.Spot(m_Target.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	static bool IsDestroyed(IEntity e)
	{
		SCR_DamageManagerComponent dmg = SCR_DamageManagerComponent.Cast(e.FindComponent(SCR_DamageManagerComponent));
		if (!dmg)
			return false;

		return dmg.GetState() == EDamageState.DESTROYED; // VERIFY: the target prefab has an SCR_DamageManagerComponent (else the mission ends only when the target is deleted)
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_eState == RZ_EMissionState.FAILED)
			RZ_Util.DeleteUnit(m_Target);
	}
}

//------------------------------------------------------------------------------------------------
//! An item to carry to a faction drop point. Subclasses spawn the item (now or
//! later) and one or more drop points. Handing the item in at one of those drop
//! points succeeds; losing the item (deleted, destroyed) fails.
//! Only the player who hands it in, and anyone with them at the drop, is paid.
class RZ_Mission_Delivery : RZ_Mission
{
	protected IEntity m_Item;
	protected bool m_bItemSpawned;
	protected bool m_bDelivered;
	protected int m_iCarrier = -1;
	protected int m_iDeliveredTo = -1;
	protected ref array<IEntity> m_aDrops = {};
	protected ref array<int> m_aDropFactions = {};
	protected ResourceName m_sItemPrefab;	// what SpawnItem made, to put it back if a carrier leaves
	protected vector m_vItemLastPos;		// where the item (or whoever carried it) was last seen
	protected bool m_bItemWasCarried;		// last seen in an inventory (a player, a body, a vehicle)
	protected int m_iItemReturns;			// times it was put back after a carrier left the server
	protected vector m_vInside;				// PlaceInsideLater: the item lies in the building here
	protected bool m_bInsideLater;

	protected static const float DROP_GROUP_RADIUS = 50;
	protected static const int MAX_ITEM_RETURNS = 3;
	protected static const float INSIDE_PLACE_RADIUS = 60;	// an item inside a building is placed once a player is this close

	//------------------------------------------------------------------------------------------------
	static bool CanRun(RZ_GameModeComponent gm, RZ_EFaction dropFaction)
	{
		return !gm.GetMissionItemPrefab().IsEmpty() && !gm.GetDropPointPrefab(dropFaction).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! The mission item on the ground at pos, or `prefab` if given (it needs RZ_MissionItemComponent too).
	protected bool SpawnItem(RZ_GameModeComponent gm, vector pos, ResourceName prefab = "")
	{
		return PlaceItem(gm, RZ_Util.GroundPos(pos) + Vector(0, 0.3, 0), prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! The mission item lying in the building nearest pos (within 15 m): on a piece of furniture or
	//! the floor, like loose loot (RZ_LootAnchors). On the ground at pos if there's no building or
	//! no anchor in it.
	protected bool SpawnItemInside(RZ_GameModeComponent gm, vector pos, ResourceName prefab = "")
	{
		IEntity building = RZ_LootAnchors.BuildingAt(pos, 15);
		vector at;
		if (building && RZ_LootAnchors.PickOne(building, at))
			return PlaceItem(gm, at + Vector(0, 0.02, 0), prefab);

		return SpawnItem(gm, pos, prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! The item lies in the building nearest pos (SpawnItemInside), placed once a player comes
	//! within 60 m (items only near players). Its spot counts as the mission's (no lock there).
	protected void PlaceInsideLater(vector pos)
	{
		m_vInside = pos;
		m_bInsideLater = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone hands it over (a talk scene's way): the item is put on the ground at pos, or moved
	//! there if it lies loose. Not while someone carries it.
	protected void PutItemAt(RZ_GameModeComponent gm, vector pos)
	{
		vector at = RZ_Util.GroundPos(pos) + Vector(0, 0.3, 0);
		if (!m_bItemSpawned)
		{
			m_bInsideLater = false;
			PlaceItem(gm, at, "");
			return;
		}

		if (m_Item && m_iCarrier <= 0 && m_Item.GetRootParent() == m_Item)
			m_Item.SetOrigin(at);
	}

	//------------------------------------------------------------------------------------------------
	//! Exactly at pos.
	protected bool PlaceItem(RZ_GameModeComponent gm, vector pos, ResourceName prefab)
	{
		if (prefab.IsEmpty())
			prefab = gm.GetMissionItemPrefab();

		m_Item = RZ_Util.SpawnPrefab(prefab, pos, Math.RandomFloat(0, 360));
		if (!m_Item)
			return false;

		RZ_MissionItemComponent.Tag(m_Item, m_iId);
		// A delivery's food is the giver's goods: nobody else takes it for pay
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && RZ_FoodHelper.IsFood(m_Item, houses.GetFoodKeywords()))
			RZ_MissionGoods.Tag(m_Item, RZ_Factions.GetNameInText(m_eGiver));

		m_bItemSpawned = true;
		m_sItemPrefab = prefab;
		m_vItemLastPos = m_Item.GetOrigin();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the work is (docs/MISSIONS.md, "Objectives"): the thing where it lies (the search area
	//! while you know only that), then, once someone carries it, the drop it goes to (ObjectiveDrop).
	//! Before the thing is out (a scene hands it over), the mission's own place.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_bDelivered)
			return super.ObjectivePlace(gm, playerId, from);

		if (IsCarried())
		{
			int idx = ObjectiveDrop(playerId, from);
			if (idx >= 0 && idx < m_aDrops.Count() && m_aDrops[idx])
				return RZ_ObjPlace.Spot(m_aDrops[idx].GetOrigin(), DropRadius());
		}

		vector lies = vector.Zero;
		if (m_bItemSpawned && m_Item)
			lies = m_vItemLastPos;
		else if (m_bInsideLater)
			lies = m_vInside;

		if (lies == vector.Zero)
			return super.ObjectivePlace(gm, playerId, from);

		// The rough rule: a thing lying at the contract's hidden place is a search area until you're close
		if (IsRoughFor(gm, playerId, from) && RZ_Util.DistXZ(lies, m_vPos) <= RoughRadius(gm))
			return AreaPlace(gm, playerId, from);

		return RZ_ObjPlace.Spot(lies);
	}

	//------------------------------------------------------------------------------------------------
	//! Picking the thing up counts you as there: "Get to" points where it lies.
	override RZ_ObjPlace GetTherePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (!m_bDelivered && !IsCarried() && ((m_bItemSpawned && m_Item) || m_bInsideLater))
			return ObjectivePlace(gm, playerId, from);

		return super.GetTherePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone has the thing (a player, a body, a vehicle): it goes to a drop now.
	bool IsCarried()
	{
		return m_bItemSpawned && m_Item && !m_bDelivered && (m_iCarrier > 0 || m_bItemWasCarried);
	}

	//------------------------------------------------------------------------------------------------
	//! Which drop the carried thing is pointed to: the first, the one the job names (a mission with a
	//! second drop that's only a way out, "or turn it in at the stronghold", keeps the first). -1: none.
	int ObjectiveDrop(int playerId, vector from)
	{
		if (m_aDrops.IsEmpty())
			return -1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! How close to a drop counts for the tracker's arrow: handing in is done at the drop itself.
	protected float DropRadius()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The drops' places, in order (the runtime test reads where a delivery can be done).
	void GetDropSpots(notnull array<vector> result)
	{
		foreach (IEntity drop : m_aDrops)
		{
			if (drop)
				result.Insert(drop.GetOrigin());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity SpawnDrop(RZ_GameModeComponent gm, RZ_EFaction faction, vector pos)
	{
		IEntity drop = RZ_Util.SpawnPrefab(gm.GetDropPointPrefab(faction), RZ_Util.GroundPos(pos), Math.RandomFloat(0, 360));
		if (!drop)
			return null;

		m_aDrops.Insert(drop);
		m_aDropFactions.Insert(faction);
		return drop;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetDropPos(int idx)
	{
		if (idx < 0 || idx >= m_aDrops.Count() || !m_aDrops[idx])
			return m_vPos;

		return m_aDrops[idx].GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	//! Found: someone carries the thing (ObjThen's second line points at the drop).
	override protected bool ObjFirstDone()
	{
		return IsCarried() || m_bDelivered;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bInsideLater && !m_bItemSpawned && CountPlayersNear(players, m_vInside, INSIDE_PLACE_RADIUS) > 0 && SpawnItemInside(gm, m_vInside))
			m_bInsideLater = false;

		OnDeliveryTick(gm, dt, ids, players);

		if (!m_bItemSpawned || m_bDelivered)
			return;

		if (!m_Item)
		{
			// The carrier left the server (or their body was cleared away) and the item went with
			// it: put it back where it was, so one disconnect can't sink the job for everyone
			if (!TryReturnItem(gm))
				m_eState = RZ_EMissionState.FAILED;

			return;
		}

		IEntity root = m_Item.GetRootParent(); // the carrier, or the vehicle they sit in
		m_bItemWasCarried = root && root != m_Item;
		if (m_bItemWasCarried)
			m_vItemLastPos = root.GetOrigin();
		else
			m_vItemLastPos = m_Item.GetOrigin();

		int carrier = RZ_Util.GetCarrierPlayerId(m_Item);
		if (carrier != m_iCarrier)
		{
			int previous = m_iCarrier;
			m_iCarrier = carrier;
			OnCarrierChanged(gm, previous, carrier);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The item is gone. If it was last in an inventory (a carrier who left the server takes their
	//! character and pack with them; a body is cleared away after a while), a new one is put on the
	//! ground where it was. A few times at most. Lost from the ground, it stays lost.
	protected bool TryReturnItem(RZ_GameModeComponent gm)
	{
		if (!m_bItemWasCarried || m_sItemPrefab.IsEmpty() || m_iItemReturns >= MAX_ITEM_RETURNS)
			return false;

		int left = m_iCarrier;
		m_iCarrier = -1;
		m_bItemWasCarried = false;
		if (!SpawnItem(gm, LandPointNear(m_vItemLastPos, 0, 2), m_sItemPrefab))
			return false;

		m_iItemReturns++;
		string who = "";
		if (left > 0)
			who = RZ_Names.Public(left);

		if (who.IsEmpty())
			who = "Whoever had it";

		NotifyInterested(gm, m_sTitle, string.Format("%1 is gone. What they carried for this job is lying where they were last seen (%2).", who, RZ_SettlementScanner.GridReference(m_vItemLastPos)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCarrierChanged(RZ_GameModeComponent gm, int previous, int carrier)
	{
		if (carrier > 0 && !m_aParticipants.Contains(carrier))
			m_aParticipants.Insert(carrier);
	}

	//------------------------------------------------------------------------------------------------
	//! Only at one of this mission's own drops (never another mission's drop of the same faction).
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		if (m_bDelivered || !drop)
			return false;

		int dropIdx = m_aDrops.Find(drop);
		if (dropIdx < 0 || m_aDropFactions[dropIdx] != faction)
			return false;

		m_bDelivered = true;
		m_iDeliveredTo = faction;

		// The deliverer and whoever came with them
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		vector at = drop.GetOrigin();
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != playerId && RZ_Util.DistXZ(p.GetOrigin(), at) <= DROP_GROUP_RADIUS)
				m_aParticipants.Insert(ids[i]);
		}

		OnDelivered(gm, playerId, faction);
		m_eState = RZ_EMissionState.SUCCEEDED;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		foreach (IEntity drop : m_aDrops)
		{
			if (drop)
				result.Insert(drop.GetOrigin());
		}

		if (m_Item && !m_bItemWasCarried)
			result.Insert(m_Item.GetOrigin());

		if (m_bInsideLater)
			result.Insert(m_vInside);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		foreach (IEntity drop : m_aDrops)
		{
			RZ_Util.DeleteUnit(drop);
		}

		m_aDrops.Clear();
		if (m_Item && !m_bDelivered)
			RZ_Util.DeleteUnit(m_Item);
	}
}

//------------------------------------------------------------------------------------------------
//! A choice mission: the item can go to m_eGiver or to m_eChoiceOther. Built on who benefits
//! (Missions/Story/RZ_MissionStory.c): whoever gets it pays those with the decider (the deliverer,
//! their group within 150 m) whom it doesn't hunt; the side you crossed adds Infamy. Taken to the
//! other side, it's a switch: it pays at most the base, its Fame doesn't count toward season Fame,
//! the leader who gave the job neither counts it as help nor scales its pay, and everyone else who
//! did the work is paid half the Favor by the giver.
class RZ_Mission_Choice : RZ_Mission_Delivery
{
	//------------------------------------------------------------------------------------------------
	void RZ_Mission_Choice()
	{
		string key = Type().ToString();
		key.ToLower();
		key.Replace("rz_mission_", "");
		m_Story = new RZ_MissionStory(key);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (m_eGiver != m_Req.m_eGiver && m_eState != RZ_EMissionState.ACTIVE)
			return !RZ_Reputation.IsHunted(gm.GetTier(playerId, m_eGiver));

		return super.IsEligible(gm, playerId);
	}

	protected RZ_EFaction m_eChoiceOther = RZ_EFaction.BANDITS;
	protected int m_iDoubleCrossInfamy = 15;

	//------------------------------------------------------------------------------------------------
	static bool CanRunChoice(RZ_GameModeComponent gm, RZ_EFaction a, RZ_EFaction b)
	{
		return CanRun(gm, a) && !gm.GetDropPointPrefab(b).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		RZ_EFaction original = m_Req.m_eGiver;
		RZ_EFaction crossed = m_eChoiceOther;
		if (faction == m_eChoiceOther)
			crossed = original;

		// Who benefits: the side it went to pays those with the decider; a switch pays at most the base
		RZ_Beneficiaries parties = m_Story.m_Parties;
		if (parties.m_aParties.IsEmpty())
		{
			parties.Add(original, RZ_Factions.GetName(original), m_iDoubleCrossInfamy);
			parties.Add(m_eChoiceOther, RZ_Factions.GetName(m_eChoiceOther), m_iDoubleCrossInfamy);
		}

		vector at = m_vPos;
		IEntity decider = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (decider)
			at = decider.GetOrigin();

		m_Story.m_Deciding.Step(gm, playerId);
		parties.Choose(gm, this, playerId, parties.Find(faction), at);

		// The side that didn't get it remembers (the giver too, when it was a switch)
		m_eGiver = faction;
		SetRival(crossed, m_iDoubleCrossInfamy);
		OnChoiceMade(gm, playerId, faction);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnChoiceMade(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
	}
}
