//! Wildlife: deer in the countryside (docs/DESIGN.md, "Wildlife"). The deer themselves come from
//! the Workshop mod WildlifeExpansion (docs/MOD_STACK.md): its doe and buck prefabs graze, flee and
//! can be shot. The mod puts none on a map by itself, so this does, and nothing of the mod is
//! named in code: the two prefabs are slots on the game mode, and with the doe's slot empty the
//! whole thing is off.
//!
//! - Herds are data until a player is out in the country: now and then one is put out 150 to 300 m
//!   from a player, out of sight where it can be, never in or by a town, a faction's place, the lab
//!   or the line, and never on an island. A few does standing together and, most times, a buck off
//!   to one side. It's put away when every player is far from where it was put out.
//! - Hard caps: herds on the server, does a herd, one herd a player per cooldown. The deer are
//!   plain entities driven by the mod's own script every frame, not AI agents: they take nothing
//!   from the infected's agent budget (docs/INFECTED.md), which is why the caps here are their own.
//! - Meat. The mod decides a kill on the shooter's own machine (their fire key and a camera
//!   trace), and tells nobody: the server never learns a deer died. So the shooter's client does
//!   the same check (RZ_WildlifeClient) and reports it; the server believes it only from a player
//!   who really fired a moment ago, close enough, at a herd that still has deer to give, and lays
//!   one or two raw meat legs where the deer fell (loose items, cleaned up like any other).
//!
//! Not saved: after a restart there are no herds until one is put out again.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Wildlife.
[BaseContainerProps()]
class RZ_WildlifeSettings
{
	[Attribute("1", desc: "Deer herds out in the country near players (needs the doe prefab below)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Doe: WildlifeExpansion's RedStagDoe.et. Empty = no wildlife at all", "et")]
	ResourceName m_sDoePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Buck: WildlifeExpansion's RedStag_v2.et. Empty = herds of does only", "et")]
	ResourceName m_sBuckPrefab;

	[Attribute("{128A19AA9F731095}Prefabs/Items/Food/MeatLeg_01.et", UIWidgets.ResourcePickerThumbnail, "Meat a shot deer leaves: the raw meat leg the stove's Cook meat takes. Empty = no meat", "et")]
	ResourceName m_sMeatPrefab;

	[Attribute("2", desc: "Most herds out on the server at once")]
	int m_iMaxHerds;

	[Attribute("3", desc: "Does in a herd, at most (2 to this many)")]
	int m_iMaxDoes;

	[Attribute("0.7", desc: "Chance a herd has a buck (needs the buck prefab)")]
	float m_fBuckChance;

	[Attribute("0.25", desc: "Chance each minute that a player out in the country gets a herd near them (while under the caps)")]
	float m_fChancePerMinute;

	[Attribute("0.5", desc: "That chance at night is multiplied by this")]
	float m_fNightMult;

	[Attribute("20", desc: "Minutes before the same player can get another herd")]
	float m_fPlayerMinutes;

	[Attribute("150", desc: "A herd is put out at least this far (m) from every player")]
	float m_fNearMin;

	[Attribute("300", desc: "and at most this far (m) from the player it's for")]
	float m_fNearMax;

	[Attribute("450", desc: "A herd is put away once every player is this far (m) from where it was put out")]
	float m_fAwayRadius;

	[Attribute("150", desc: "No herd this close (m) to a town's edge, a faction's place or the lab")]
	float m_fTownMargin;

	[Attribute("1", desc: "Meat legs a shot deer leaves, at least")]
	int m_iMeatMin;

	[Attribute("2", desc: "and at most")]
	int m_iMeatMax;

	[Attribute("260", desc: "A deer counts as shot only if it fell within this many metres of the shooter (the mod's own kill range is 250)")]
	float m_fShotRange;

	[Attribute("4", desc: "Most deer a minute that leave meat, on the whole server")]
	int m_iMeatPerMinute;
}

//------------------------------------------------------------------------------------------------
//! One herd that's out. Its deer are the mod's entities; how many gave meat is counted here.
class RZ_Herd
{
	vector m_vPos;
	int m_iForPlayer;
	int m_iTaken;
	ref array<IEntity> m_aDeer = {};
}

//------------------------------------------------------------------------------------------------
class RZ_Wildlife
{
	protected static const float CHECK_SECONDS = 60;
	protected static const int TRIES = 8;
	protected static const float HERD_APART = 400;		// two herds are never closer than this (m)
	protected static const float HERD_REACH = 650;		// a shot deer belongs to a herd put out within this (m): a deer flees up to 300 m, twice
	protected static const float SHOT_SECONDS = 4;		// a report counts only this soon after the player's own shot
	protected static const float FAR_ENOUGH = 220;		// a spot in plain sight is still taken past this (m)
	protected static const float EYE = 1.6;

	protected RZ_GameModeComponent m_GM;
	protected RZ_WildlifeSettings m_Settings;
	protected ref array<ref RZ_Herd> m_aHerds = {};
	protected ref map<int, float> m_mNextFor = new map<int, float>();	// player -> clock time of their next herd
	protected ref map<int, float> m_mShotAt = new map<int, float>();	// player -> clock time of their last shot
	protected float m_fClock;
	protected float m_fCheckIn = CHECK_SECONDS;
	protected float m_fMeatMinuteAt = -1000;
	protected int m_iMeatThisMinute;
	protected bool m_bOn;
	protected bool m_bFailed;	// the doe prefab didn't spawn: off for the session, said once

	//------------------------------------------------------------------------------------------------
	void RZ_Wildlife(RZ_GameModeComponent gm, RZ_WildlifeSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		m_bOn = m_Settings.m_bEnabled && !m_Settings.m_sDoePrefab.IsEmpty();
		if (!m_bOn)
		{
			RZ_Log.Info("Wildlife: off (no doe prefab, or turned off).");
			return;
		}

		string buck = "no buck";
		if (!m_Settings.m_sBuckPrefab.IsEmpty())
			buck = string.Format("a buck %1 times in 10", RZ_Util.RoundToInt(Math.Clamp(m_Settings.m_fBuckChance, 0, 1) * 10));

		string meat = "no meat";
		if (!m_Settings.m_sMeatPrefab.IsEmpty())
			meat = string.Format("%1 to %2 meat a deer, %3 deer a minute", MeatMin(), MeatMax(), m_Settings.m_iMeatPerMinute);

		RZ_Log.Info(string.Format("Wildlife: on. At most %1 herds of up to %2 does (%3), %4 to %5 m from a player, put away past %6 m; %7.",
			m_Settings.m_iMaxHerds, MaxDoes(), buck, RZ_Util.RoundToInt(m_Settings.m_fNearMin), RZ_Util.RoundToInt(m_Settings.m_fNearMax), RZ_Util.RoundToInt(m_Settings.m_fAwayRadius), meat));
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn() { return m_bOn && !m_bFailed; }
	int CountHerds() { return m_aHerds.Count(); }

	//------------------------------------------------------------------------------------------------
	protected int MaxDoes() { return Math.ClampInt(m_Settings.m_iMaxDoes, 1, 6); }
	protected int MeatMin() { return Math.ClampInt(m_Settings.m_iMeatMin, 0, 4); }
	protected int MeatMax() { return Math.ClampInt(m_Settings.m_iMeatMax, MeatMin(), 4); }

	//------------------------------------------------------------------------------------------------
	//! A player left: their timers go.
	void Forget(int playerId)
	{
		m_mNextFor.Remove(playerId);
		m_mShotAt.Remove(playerId);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
		{
			if (!m_aHerds.IsEmpty())
				PutAwayAll();

			return;
		}

		m_fClock += dt;
		PutAwayFar(players);

		m_fCheckIn -= dt;
		if (m_fCheckIn > 0)
			return;

		m_fCheckIn = CHECK_SECONDS;
		if (m_aHerds.Count() >= Math.Max(0, m_Settings.m_iMaxHerds))
			return;

		float chance = Math.Clamp(m_Settings.m_fChancePerMinute, 0, 1);
		if (RZ_GameTime.IsNight())
			chance = chance * Math.Clamp(m_Settings.m_fNightMult, 0, 1);

		// One herd a check at most, for the first player the dice and the ground allow
		foreach (int i, int playerId : ids)
		{
			if (i >= players.Count() || !players[i])
				continue;

			float nextAt;
			if (m_mNextFor.Find(playerId, nextAt) && m_fClock < nextAt)
				continue;

			if (Math.RandomFloat01() >= chance)
				continue;

			if (PutOutFor(playerId, players[i], players))
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Herds nobody is near any more: their deer (and what's left of the shot ones) are deleted.
	protected void PutAwayFar(array<IEntity> players)
	{
		for (int i = m_aHerds.Count() - 1; i >= 0; i--)
		{
			RZ_Herd h = m_aHerds[i];
			if (!players.IsEmpty() && RZ_Util.NearestDistance(h.m_vPos, players) <= m_Settings.m_fAwayRadius)
				continue;

			PutAway(h);
			m_aHerds.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void PutAwayAll()
	{
		foreach (RZ_Herd h : m_aHerds)
		{
			PutAway(h);
		}

		m_aHerds.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void PutAway(RZ_Herd h)
	{
		foreach (IEntity deer : h.m_aDeer)
		{
			RZ_WildlifeEngine.Remove(deer);
		}

		h.m_aDeer.Clear();
	}

	// ================================================================ Putting a herd out
	//------------------------------------------------------------------------------------------------
	//! A herd near this player, if the country round them has a spot. False if it has none.
	protected bool PutOutFor(int playerId, IEntity player, array<IEntity> players)
	{
		vector from = player.GetOrigin();
		if (!IsCountry(from, 0))
			return false;

		vector spot;
		if (!FindSpot(from, players, spot))
			return false;

		RZ_Herd h = new RZ_Herd();
		h.m_vPos = spot;
		h.m_iForPlayer = playerId;
		int fewest = 2;
		if (MaxDoes() < fewest)
			fewest = MaxDoes();

		int does = Math.RandomIntInclusive(fewest, MaxDoes());
		array<vector> placed = {};
		for (int d = 0; d < does; d++)
		{
			vector at = spot;
			if (!placed.IsEmpty())
				at = RZ_Util.RandomPointInRing(placed.GetRandomElement(), 2.5, 6);

			AddDeer(h, m_Settings.m_sDoePrefab, at);
			placed.Insert(at);
		}

		if (h.m_aDeer.IsEmpty())
		{
			// The prefab isn't there (the mod isn't loaded, or the slot names something else)
			m_bFailed = true;
			RZ_Log.Warn("Wildlife: the doe prefab didn't spawn (is WildlifeExpansion loaded, and World > Wildlife > Doe its RedStagDoe.et?). Wildlife is off until the next start.");
			return false;
		}

		if (!m_Settings.m_sBuckPrefab.IsEmpty() && Math.RandomFloat01() < m_Settings.m_fBuckChance)
			AddDeer(h, m_Settings.m_sBuckPrefab, RZ_Util.RandomPointInRing(spot, 20, 30));

		m_aHerds.Insert(h);
		m_mNextFor.Set(playerId, m_fClock + Math.Max(1, m_Settings.m_fPlayerMinutes) * 60);
		RZ_Log.Info(string.Format("Wildlife: a herd of %1 put out at %2, %3 m from player %4 (%5 out).",
			h.m_aDeer.Count(), RZ_SettlementScanner.GridReference(spot), RZ_Util.RoundToInt(vector.DistanceXZ(spot, from)), playerId, m_aHerds.Count()));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddDeer(RZ_Herd h, ResourceName prefab, vector at)
	{
		vector ground = RZ_Util.GroundPos(at);
		if (RZ_Util.IsWater(ground))
			return;

		IEntity deer = RZ_Util.SpawnPrefab(prefab, ground, Math.RandomFloat(0, 360));
		if (deer)
			h.m_aDeer.Insert(deer);
	}

	//------------------------------------------------------------------------------------------------
	//! A spot for a herd round `from`: in the country, far enough from every player and every other
	//! herd, and out of this player's sight where one is found (else a far one in the open).
	protected bool FindSpot(vector from, array<IEntity> players, out vector spot)
	{
		float nearMin = Math.Max(60, m_Settings.m_fNearMin);
		float nearMax = Math.Max(nearMin + 20, m_Settings.m_fNearMax);
		bool haveOpen = false;
		vector openSpot;
		for (int t = 0; t < TRIES; t++)
		{
			vector p = RZ_Mission.LandPointNear(from, nearMin, nearMax);
			if (RZ_Util.IsWater(p) || !IsCountry(p, m_Settings.m_fTownMargin))
				continue;

			if (RZ_Util.NearestDistance(p, players) < nearMin || NearHerd(p))
				continue;

			if (!RZ_WildlifeEngine.Sees(from + Vector(0, EYE, 0), p + Vector(0, 1.2, 0)))
			{
				spot = p;
				return true;
			}

			if (!haveOpen && vector.DistanceXZ(p, from) >= FAR_ENOUGH)
			{
				haveOpen = true;
				openSpot = p;
			}
		}

		if (!haveOpen)
			return false;

		spot = openSpot;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool NearHerd(vector p)
	{
		foreach (RZ_Herd h : m_aHerds)
		{
			if (vector.DistanceXZ(h.m_vPos, p) < HERD_APART)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Out in the country: on the mainland, off the line, and `margin` clear of every town's edge,
	//! every faction's place, safehouse and road post, and the lab.
	protected bool IsCountry(vector p, float margin)
	{
		if (!RZ_Mainland.OnMainland(p) || RZ_Perimeter.IsNearLineAt(p))
			return false;

		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			if (s && vector.DistanceXZ(s.m_vCenter, p) < s.m_fRadius + margin)
				return false;
		}

		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts && facts.IsNearLivedIn(p, Math.Max(50, margin)))
			return false;

		return true;
	}

	// ================================================================ Meat
	//------------------------------------------------------------------------------------------------
	//! A player fired (the game mode's shot hook): a deer they report in the next moments is theirs.
	void OnShot(int playerId)
	{
		if (IsOn())
			m_mShotAt.Set(playerId, m_fClock);
	}

	//------------------------------------------------------------------------------------------------
	//! A client says its player shot a deer that stood at `at` (RZ_WildlifeClient). Believed only
	//! from a living player who fired a moment ago, close enough to it, with a herd out that still
	//! has a deer to give; then the meat lies where it fell.
	void OnDeerShot(int playerId, vector at)
	{
		if (!IsOn() || m_Settings.m_sMeatPrefab.IsEmpty())
			return;

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!player || !RZ_Util.IsCharacterAlive(player))
			return;

		float shotAt;
		if (!m_mShotAt.Find(playerId, shotAt) || m_fClock - shotAt > SHOT_SECONDS + m_GM.GetTickSeconds())
			return;

		vector ground = RZ_Util.GroundPos(at);
		if (vector.DistanceXZ(ground, player.GetOrigin()) > m_Settings.m_fShotRange || RZ_Util.IsWater(ground))
			return;

		RZ_Herd herd = HerdFor(ground);
		if (!herd)
			return;

		if (m_fClock - m_fMeatMinuteAt >= 60)
		{
			m_fMeatMinuteAt = m_fClock;
			m_iMeatThisMinute = 0;
		}

		if (m_iMeatThisMinute >= Math.Max(0, m_Settings.m_iMeatPerMinute))
			return;

		m_iMeatThisMinute++;
		herd.m_iTaken++;
		int legs = Math.RandomIntInclusive(MeatMin(), MeatMax());
		int laid = 0;
		for (int i = 0; i < legs; i++)
		{
			// A deer has no inventory to open (it's a plain entity), so the meat lies beside it
			IEntity leg = RZ_Util.SpawnPrefab(m_Settings.m_sMeatPrefab, RZ_Util.RandomPointInRing(ground, 0.5, 1.2) + Vector(0, 0.3, 0), Math.RandomFloat(0, 360));
			if (!leg)
				continue;

			RZ_GameModeComponent.TrackLooseItem(leg);
			laid++;
		}

		RZ_Log.Info(string.Format("Wildlife: player %1 shot a deer at %2; %3 meat (%4 of %5 from that herd).",
			playerId, RZ_SettlementScanner.GridReference(ground), laid, herd.m_iTaken, herd.m_aDeer.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest herd out that a deer at `at` can belong to and that still has one to give.
	protected RZ_Herd HerdFor(vector at)
	{
		RZ_Herd best;
		float bestDist = HERD_REACH;
		foreach (RZ_Herd h : m_aHerds)
		{
			if (h.m_iTaken >= h.m_aDeer.Count())
				continue;

			float d = vector.DistanceXZ(h.m_vPos, at);
			if (d <= bestDist)
			{
				bestDist = d;
				best = h;
			}
		}

		return best;
	}
}

//------------------------------------------------------------------------------------------------
//! The engine calls, in one place.
class RZ_WildlifeEngine
{
	//------------------------------------------------------------------------------------------------
	//! Nothing of the world or its entities stands between the two points.
	static bool Sees(vector from, vector to)
	{
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = to;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		return ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param) >= 0.999;
	}

	//------------------------------------------------------------------------------------------------
	//! A deer goes. VERIFY: deleting WildlifeExpansion's deer on the server (a GenericEntity with an
	//! RplComponent and the mod's per-frame script) removes it on every client, shot or not.
	static void Remove(IEntity deer)
	{
		if (deer)
			SCR_EntityHelper.DeleteEntityAndChildren(deer);
	}
}

//------------------------------------------------------------------------------------------------
//! The shooter's side of a deer's death (called each frame from the HUD's update). WildlifeExpansion
//! kills a deer on the machine of whoever pulled the trigger while their camera's line met it, within
//! its kill range, and replicates nothing. The same test here, on the same frame, tells the server
//! which deer that was, once per deer.
class RZ_WildlifeClient
{
	protected static const float REACH = 500;			// the mod's trace length (m)
	protected static const int REMEMBERED = 24;

	protected static bool s_bWasDown;
	protected static ref array<IEntity> s_aReported = {};

	//------------------------------------------------------------------------------------------------
	static void Tick()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return;

		RZ_WildlifeSettings settings = gm.GetWildlifeSettings();
		if (!settings || !settings.m_bEnabled || settings.m_sDoePrefab.IsEmpty() || settings.m_sMeatPrefab.IsEmpty())
			return;

		InputManager input = GetGame().GetInputManager();
		if (!input)
			return;

		// VERIFY: "CharacterFire" is triggered on the frame the local player fires (the action
		// WildlifeExpansion's own kill check reads; it also asks "WeaponFire" first)
		bool down = input.GetActionTriggered("CharacterFire");
		if (!down)
		{
			s_bWasDown = false;
			return;
		}

		if (s_bWasDown)
			return;

		s_bWasDown = true;
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		IEntity me = pc.GetControlledEntity();
		if (!me)
			return;

		IEntity hit = Aimed(me);
		if (!hit || s_aReported.Contains(hit) || !IsDeer(hit, settings))
			return;

		if (vector.Distance(hit.GetOrigin(), me.GetOrigin()) > settings.m_fShotRange)
			return;

		if (s_aReported.Count() >= REMEMBERED)
			s_aReported.Remove(0);

		s_aReported.Insert(hit);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendDeerShot(hit.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	//! What the camera's line meets within the mod's reach, the player aside.
	//! VERIFY: the camera trace (world and entities, default layers) stops on a deer's box collider,
	//! as WildlifeExpansion's own does, so both agree on which deer was hit.
	protected static IEntity Aimed(IEntity me)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return null;

		vector mat[4];
		world.GetCurrentCamera(mat);
		TraceParam trace = new TraceParam();
		trace.Start = mat[3];
		trace.End = mat[3] + mat[2] * REACH;
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		trace.Exclude = me;
		if (world.TraceMove(trace, null) >= 1)
			return null;

		return trace.TraceEnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Its prefab is the doe's or the buck's.
	protected static bool IsDeer(IEntity e, RZ_WildlifeSettings settings)
	{
		string name = FileOf(RZ_Util.GetPrefabName(e));
		if (name.IsEmpty())
			return false;

		if (name == FileOf(settings.m_sDoePrefab))
			return true;

		return !settings.m_sBuckPrefab.IsEmpty() && name == FileOf(settings.m_sBuckPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! "redstagdoe.et" from "{GUID}Prefabs/RedStagDoe.et".
	protected static string FileOf(string prefab)
	{
		string name = prefab;
		name.ToLower();
		int slash = name.LastIndexOf("/");
		if (slash >= 0)
			name = name.Substring(slash + 1, name.Length() - slash - 1);

		return name;
	}
}
